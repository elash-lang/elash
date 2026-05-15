from pathlib import Path
from dataclasses import dataclass
from typing import Optional

import subprocess
import sys
import os
import difflib

@dataclass
class TestOutput:
    exitcode:    int
    stdout:      str
    stderr:      str
    error_stage: Optional[str] = None

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

def print_fail(name: str):
    print(f'[{CLR_RED}FAIL{CLR_RESET}] Test failed: {name}')

def error(*args):
    print(f'{CLR_RED}error: {CLR_RESET}', end='')
    print(*args)
    sys.exit(1)

def get_expected_output(path: Path, name: str) -> TestOutput:
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

    return TestOutput(exitcode=exitcode, stdout=stdout, stderr=stderr)

def print_diff(expected: str, actual: str, stream_name: str):
    diff = difflib.unified_diff(
        expected.splitlines(keepends=True),
        actual.splitlines(keepends=True),
        fromfile=f'expected {stream_name}',
        tofile=f'actual {stream_name}',
    )
    for line in diff:
        print_info(f'  {line.rstrip()}')

def run_test_case(elc_bin: Path, work_dir: Path, path: Path, name: str) -> TestOutput:
    input_file = path.joinpath('input.ela')
    if not input_file.is_file():
        error(f"ill-formed test case '{name}': no input.ela")

    obj = work_dir.joinpath(f'{name}.o')
    exe = work_dir.joinpath(name)

    res = subprocess.run([str(elc_bin), 'compile', str(input_file), '-o', str(obj)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        return TestOutput(exitcode=res.returncode, stdout=res.stdout.strip(), stderr=res.stderr.strip(), error_stage='compilation')
    
    res = subprocess.run(['cc', str(obj), '-o', str(exe)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        return TestOutput(exitcode=res.returncode, stdout=res.stdout.strip(), stderr=res.stderr.strip(), error_stage='linking')
    
    res = subprocess.run([str(exe)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return TestOutput(exitcode=res.returncode, stdout=res.stdout.strip(), stderr=res.stderr.strip())

def report_failure(name: str, expected: TestOutput, actual: TestOutput):
    print_fail(name)
    if actual.error_stage:
        print_info(f'  Error during {CLR_BOLD}{actual.error_stage}{CLR_RESET} stage (exitcode {actual.exitcode})')
        for stream in ['stdout', 'stderr']:
            content = getattr(actual, stream)
            if content:
                print_info(f'  {actual.error_stage} {stream}:')
                for line in content.splitlines():
                    print_info(f'    {line}')
    else:
        if actual.exitcode != expected.exitcode:
            print_info(f'  exitcode: expected {expected.exitcode}, actual {actual.exitcode}')
        print_diff(expected.stdout, actual.stdout, 'stdout')
        print_diff(expected.stderr, actual.stderr, 'stderr')

def run_suite(elc_bin: Path, work_dir: Path) -> bool:
    passed_count = 0
    failed_count = 0

    for path in sorted(script_dir.iterdir()):
        if not path.is_dir() or path.name.startswith('.'): continue

        name = path.name
        expected = get_expected_output(path, name)
        actual = run_test_case(elc_bin, work_dir, path, name)
        
        if actual.error_stage is None and actual == expected:
            print_pass(name)
            passed_count += 1
        else:
            report_failure(name, expected, actual)
            failed_count += 1

    tested_count = passed_count + failed_count
    print(f'[{CLR_BLUE}===={CLR_RESET}] {CLR_BOLD}Synthesis: ', end='')
    print(f'Tested: {CLR_BLUE}{tested_count}{CLR_RESET}{CLR_BOLD} ', end='')
    print(f'| Passing: {CLR_GREEN}{passed_count}{CLR_RESET}{CLR_BOLD} ', end='')
    print(f'| Failing: {CLR_RED}{failed_count}{CLR_RESET}{CLR_BOLD}', end='')
    print()
    
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
