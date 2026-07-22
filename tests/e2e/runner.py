from pathlib import Path
from dataclasses import dataclass
from typing import Literal, Optional

import subprocess
import sys
import os
import difflib

type TestStage = Literal['compilation'] | Literal['linking'] | Literal['runtime'];

@dataclass
class TestExpectation:
    exitcode:    int
    stdout:      str
    stderr:      str
    diags: Optional[list[str]] = None

@dataclass
class TestResult:
    exitcode:    int
    stdout:      str
    stderr:      str
    stage:       TestStage

script_dir = Path(__file__).resolve().parent

CLR_BLUE   = '\033[0;34m'
CLR_GREEN  = '\033[0;32m'
CLR_RED    = '\033[0;31m'
CLR_BOLD   = '\033[0;1m'
CLR_RESET  = '\033[0m'

def print_info(msg: str):
    print(f'[{CLR_BLUE}----{CLR_RESET}] {msg}')

def print_pass(name: str):
    print(f'[{CLR_GREEN}PASS{CLR_RESET}] Test passed: {name}')
def print_skip(name: str):
    print(f'[{CLR_BLUE}SKIP{CLR_RESET}] Test skipped: {name}')
def print_fail(name: str):
    print(f'[{CLR_RED}FAIL{CLR_RESET}] Test failed: {name}')

def error(*args):
    print(f'{CLR_RED}error: {CLR_RESET}', end='')
    print(*args)
    sys.exit(1)

def get_expectation(path: Path, name: str) -> TestExpectation:
    exitcode: int = 0
    if (f := path.joinpath('exitcode.txt')).is_file():
        try:
            exitcode = int(f.read_text())
        except ValueError:
            error(f"ill-formed test case '{name}': {f.joinpath('exitcode.txt')} should contain a valid integer")

    stdout: str = ''
    if (f := path.joinpath('stdout.txt')).is_file():
        stdout = f.read_text().strip()

    stderr: str = ''
    if (f := path.joinpath('stderr.txt')).is_file():
        stderr = f.read_text().strip()

    diags = None
    if (f := path.joinpath('diags.txt')).is_file():
        diags = [line.strip() for line in f.read_text().splitlines() if line.strip()]

    return TestExpectation(exitcode=exitcode, stdout=stdout, stderr=stderr, diags=diags)

def print_diff(expected: str, actual: str, stream_name: str):
    diff = difflib.unified_diff(
        expected.splitlines(keepends=True),
        actual.splitlines(keepends=True),
        fromfile=f'expected {stream_name}',
        tofile=f'actual {stream_name}',
    )
    for line in diff:
        print_info(f'  {line.rstrip()}')

def run_test_case(elc_bin: Path, work_dir: Path, path: Path, name: str, is_negative: bool) -> TestResult | None:
    input_file = path.joinpath('input.ei')
    if not input_file.is_file():
        error(f"ill-formed test case '{name}': no input.ei")

    skip_file = path.joinpath('skip')
    if skip_file.is_file():
        return None

    safe_name = name.replace(os.sep, '_')
    obj = work_dir.joinpath(f'{safe_name}.o')
    exe = work_dir.joinpath(safe_name)

    res = subprocess.run([str(elc_bin), 'compile', str(input_file), '-o', str(obj)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if is_negative or res.returncode != 0:
        return TestResult(exitcode=res.returncode, stdout=res.stdout.strip(), stderr=res.stderr.strip(), stage='compilation')

    res = subprocess.run(['cc', str(obj), '-o', str(exe)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        return TestResult(exitcode=res.returncode, stdout=res.stdout.strip(), stderr=res.stderr.strip(), stage='linking')

    res = subprocess.run([str(exe)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return TestResult(exitcode=res.returncode, stdout=res.stdout.strip(), stderr=res.stderr.strip(), stage='runtime')

def report_failure(name: str, expected: TestExpectation, actual: TestResult):
    print_fail(name)

    is_unexpected_stage = False
    if expected.diags is not None:
        if actual.stage != 'compilation':
            is_unexpected_stage = True
    else:
        if actual.stage != 'runtime':
            is_unexpected_stage = True

    if is_unexpected_stage:
        print_info(f'  Error during {CLR_BOLD}{actual.stage}{CLR_RESET} stage (exitcode {actual.exitcode})')
        for stream in ['stdout', 'stderr']:
            content = getattr(actual, stream)
            if content:
                print_info(f'  {actual.stage} {stream}:')
                for line in content.splitlines():
                    print_info(f'    {line}')
    else:
        if actual.exitcode != expected.exitcode:
            print_info(f'  exitcode: expected {expected.exitcode}, actual {actual.exitcode}')

        if expected.diags:
            combined_output = actual.stdout + actual.stderr
            for code in expected.diags:
                if code not in combined_output:
                    print_info(f'  missing diag code: {CLR_BOLD}{code}{CLR_RESET}')

        if expected.stdout or actual.stdout:
            print_diff(expected.stdout, actual.stdout, 'stdout')
        if expected.stderr or actual.stderr:
            print_diff(expected.stderr, actual.stderr, 'stderr')

def _collect_test_dirs():
    return sorted({p.parent for p in script_dir.rglob('input.ei')})

def _is_success(expected: TestExpectation, actual: TestResult) -> bool:
    if expected.diags is not None:
        if actual.stage != 'compilation':
            return False
        if actual.exitcode == 0:
            return False
        if expected.exitcode != 0 and actual.exitcode != expected.exitcode:
            return False
        combined_output = actual.stdout + actual.stderr
        for code in expected.diags:
            if code not in combined_output:
                return False
        return True
    else:
        if actual.stage != 'runtime':
            return False
        if actual.exitcode != expected.exitcode:
            return False
        if actual.stdout != expected.stdout:
            return False
        if actual.stderr != expected.stderr:
            return False
        return True

def run_suite(elc_bin: Path, work_dir: Path) -> bool:
    passed_count  = 0
    failed_count  = 0
    skipped_count = 0

    test_dirs = _collect_test_dirs()

    for path in test_dirs:
        name = str(path.relative_to(script_dir))
        expected = get_expectation(path, name)
        actual = run_test_case(elc_bin, work_dir, path, name, expected.diags is not None)

        if actual is None:
            print_skip(name)
            skipped_count += 1
        elif _is_success(expected, actual):
            print_pass(name)
            passed_count += 1
        else:
            report_failure(name, expected, actual)
            failed_count += 1

    tested_count = passed_count + failed_count + skipped_count
    print(f'[{CLR_BLUE}===={CLR_RESET}] {CLR_BOLD}Synthesis: ', end='')
    print(f'Tested: {CLR_BLUE}{tested_count}{CLR_RESET}{CLR_BOLD} ', end='')
    print(f'| Passing: {CLR_GREEN}{passed_count}{CLR_RESET}{CLR_BOLD} ', end='')
    print(f'| Failing: {CLR_RED}{failed_count}{CLR_RESET}{CLR_BOLD} ', end='')
    print(f'| Skipped: {CLR_BLUE}{skipped_count}{CLR_RESET}{CLR_BOLD}', end='')
    print(CLR_RESET)

    return failed_count == 0

def main():
    if len(sys.argv) <= 2:
        error('expected argument\nusage: python runner.py <path-to-elc-binary> <work-dir>')

    elc_bin  = Path(sys.argv[1]).resolve()
    work_dir = Path(sys.argv[2]).resolve()

    if not work_dir.exists():
        os.makedirs(str(work_dir), exist_ok=True)

    if not run_suite(elc_bin, work_dir):
        sys.exit(1)

if __name__ == '__main__':
    main()
