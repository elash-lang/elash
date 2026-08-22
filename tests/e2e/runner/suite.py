import threading
import typing
import json
import os

from concurrent.futures import (
    ThreadPoolExecutor, as_completed
)
from pathlib import Path

from .models  import *
from .console import *
from .defs    import *

from .test import run_test_case, get_expectation

def _parse_jsonl_diagnostics(actual: FinishedResult):
    diagnostics = []
    output = actual.stdout + "\n" + actual.stderr

    errors: list[Exception] = []
    for line in output.splitlines():
        try:
            data = json.loads(line)
            if data['type'] == 'diag':
                diagnostics.append(data)
        except (json.JSONDecodeError, KeyError) as err:
            errors.append(err)

    if len(errors) != 0:
        if len(errors) == 1:
            print_info(f'failed to parse elc output: {errors[0]}')
        else:
            print_info(f'failed to parse elc output:')
            for i, err in enumerate(errors):
                print_info(f'  {i}: {err}')

        print_info('compilation stdout + stderr:')
        for line in output.splitlines():
            print_info(f'  {line}')

    return diagnostics

def _match_diagnostic(exp: DiagExpectation, diag: dict) -> bool:
    if not _match_diagnostic_nomsg(exp, diag):
        return False

    if exp.message is not None:
        if 'formatted' not in diag:
            return False
        if exp.message.lower().strip() != diag['formatted'].lower().strip():
            return False

    return True

def _match_diagnostic_nomsg(exp: DiagExpectation, diag: dict) -> bool:
    if exp.severity != diag['severity']:
        return False
    if exp.code is not None and exp.code != diag['category']:
        return False

    if exp.lines is not None:
        # jsonl printer outputs 0-indexed lines and line numbers in diags.txt files are 1-indexed
        actual_lines = [r['start']['line'] + 1 for r in diag['span']['ranges']]
        if not all(l in actual_lines for l in exp.lines):
            return False

    return True

def _expand_expectations(diags: list[DiagExpectation]) -> list[DiagExpectation]:
    expanded = []
    for exp in diags:
        if exp.lines:
            for line in exp.lines:
                expanded.append(DiagExpectation(severity=exp.severity, code=exp.code, lines=[line], message=exp.message))
        else:
            expanded.append(exp)
    return expanded

def _report_diag_mismatches(expanded: list[DiagExpectation], diagnostics: list[dict]):
    for exp in expanded:
        if any(_match_diagnostic(exp, diag) for diag in diagnostics):
            continue

        partial_matches = [d for d in diagnostics if _match_diagnostic_nomsg(exp, d)]
        if partial_matches and exp.message:
            for diag in partial_matches:
                print_info(f'  diagnostic message mismatch:')
                print_diff(exp.message, diag.get('formatted', ''))
            continue

        line_str = ','.join(map(str, exp.lines)) + ':' if exp.lines else ''
        msg_str = f': {exp.message}' if exp.message else ''
        cat_str = f'[{exp.code}]' if exp.code is not None else ''
        print_info(f'  missing diag: {CLR_BOLD}{line_str}{exp.severity}{cat_str}{msg_str}{CLR_RESET}')

    for diag in diagnostics:
        if not any(_match_diagnostic(exp, diag) for exp in expanded):
            actual_lines = [r['start']['line'] + 1 for r in diag['span']['ranges']]
            lines_str = ','.join(map(str, actual_lines))  + ':'
            print_info(f'  unexpected diag: {CLR_BOLD}{lines_str}{diag["severity"]}[{diag["category"]}]: {diag["formatted"]}{CLR_RESET}')

def report_failure(name: str, expected: TestExpectation, actual: TestResult):
    if isinstance(actual, TimedOutResult):
        print_timeout(name)
        print_info(f'  Timed out during {CLR_BOLD}{actual.stage}{CLR_RESET} stage')
        return

    print_fail(name)

    if isinstance(expected, NegativeTestExpectation):
        if actual.stage != 'compilation':
            print_info(f'  Error during {CLR_BOLD}{actual.stage}{CLR_RESET} stage (exitcode {actual.exitcode})')
            return

        diagnostics = _parse_jsonl_diagnostics(actual)
        expanded    = _expand_expectations(expected.diags)
        _report_diag_mismatches(expanded, diagnostics)

    elif isinstance(expected, PositiveTestExpectation):
        if actual.stage != 'runtime':
             print_info(f'  Error during {CLR_BOLD}{actual.stage}{CLR_RESET} stage (exitcode {actual.exitcode})')
             return

        if actual.exitcode != expected.exitcode:
            print_info(f'  exitcode: expected {expected.exitcode}, actual {actual.exitcode}')

        if expected.stdout or actual.stdout:
            print_diff(expected.stdout, actual.stdout, 'stdout')
        if expected.stderr or actual.stderr:
            print_diff(expected.stderr, actual.stderr, 'stderr')

        if expected.diags and actual.compilation_result is not None:
            diagnostics = _parse_jsonl_diagnostics(actual.compilation_result)
            expanded    = _expand_expectations(expected.diags)
            _report_diag_mismatches(expanded, diagnostics)

def _collect_test_cases() -> list[TestCase]:
    def get_name(item: Path) -> str:
        return str(item.relative_to(script_dir)).removesuffix(".eu")

    def collect(ttype: TestType) -> list[TestCase]:
        cases: list[TestCase] = []

        dir = script_dir.joinpath(ttype)
        if not dir.is_dir():
            error(f"invalid tests directory structure: expected '{ttype}' to be a directory")

        for p in dir.rglob('input.eu'):
            cases.append(TestCase(path=p.parent, name=get_name(p.parent), type=ttype))
        for p in dir.rglob('*.eu'):
            if p.name != 'input.eu':
                cases.append(TestCase(path=p, name=get_name(p), type=ttype))

        return cases

    cases = (*collect('positive'), *collect('negative'))
    return sorted(cases)

def _is_success(expected: TestExpectation, actual: TestResult) -> bool:
    if isinstance(actual, TimedOutResult):
        return False

    if isinstance(expected, NegativeTestExpectation):
        if actual.stage != 'compilation':
            return False
        if actual.exitcode == 0:
            return False

        diagnostics = _parse_jsonl_diagnostics(actual)
        expanded    = _expand_expectations(expected.diags)

        # missing diagnostics
        for exp in expanded:
            if not any(_match_diagnostic(exp, diag) for diag in diagnostics):
                return False

        if expected.ignore_unexpected: return True

        # unexpected diagnostics
        for diag in diagnostics:
            if not any(_match_diagnostic(exp, diag) for exp in expanded):
                return False

        return True

    elif isinstance(expected, PositiveTestExpectation):
        if actual.stage != 'runtime':
            return False
        if actual.exitcode != expected.exitcode:
            return False
        if actual.stdout != expected.stdout:
            return False
        if actual.stderr != expected.stderr:
            return False

        if expected.diags and actual.compilation_result is not None:
            diagnostics = _parse_jsonl_diagnostics(actual.compilation_result)
            expanded    = _expand_expectations(expected.diags)

            for exp in expanded:
                if not any(_match_diagnostic(exp, diag) for diag in diagnostics):
                    return False

            for diag in diagnostics:
                if not any(_match_diagnostic(exp, diag) for exp in expanded):
                    return False

        return True

    assert False

def run_suite(elc_bin: Path, work_dir: Path, jobs: Optional[int], timeouts: Timeouts) -> bool:
    passed_count  = 0
    failed_count  = 0
    skipped_count = 0

    test_cases = _collect_test_cases()

    tasks = []
    for case in test_cases:
        tasks.append((case, get_expectation(case)))

    def handle_result(name, expected, actual):
        nonlocal passed_count, failed_count, skipped_count
        if actual is None:
            print_skip(name)
            skipped_count += 1
        elif _is_success(expected, actual):
            print_pass(name)
            passed_count += 1
        else:
            report_failure(name, expected, actual)
            failed_count += 1

    if jobs is None or jobs > 1:
        with ThreadPoolExecutor(max_workers=jobs) as executor:
            future_to_test = {}
            for case, expected in tasks:
                future = executor.submit(run_test_case, elc_bin, work_dir, case, timeouts)
                future_to_test[future] = (case.name, expected)

            for future in as_completed(future_to_test):
                name, expected = future_to_test[future]
                handle_result(name, expected, future.result())
    else:
        for case, expected in tasks:
            handle_result(case.name, expected, run_test_case(elc_bin, work_dir, case, timeouts))

    tested_count = passed_count + failed_count + skipped_count
    print(f'[{CLR_BLUE}===={CLR_RESET}] {CLR_BOLD}Synthesis: ', end='')
    print(f'Tested: {CLR_BLUE}{tested_count}{CLR_RESET}{CLR_BOLD} ', end='')
    print(f'| Passing: {CLR_GREEN}{passed_count}{CLR_RESET}{CLR_BOLD} ', end='')
    print(f'| Failing: {CLR_RED}{failed_count}{CLR_RESET}{CLR_BOLD} ', end='')
    print(f'| Skipped: {CLR_BLUE}{skipped_count}{CLR_RESET}{CLR_BOLD}', end='')
    print(CLR_RESET)

    return failed_count == 0
