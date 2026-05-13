from pathlib import Path
from dataclasses import dataclass

import subprocess
import sys
import os
import difflib

@dataclass
class TestOutput:
    exitcode: int
    stdout:   str
    stderr:   str

script_dir = Path(__file__).resolve().parent

CLR_BLUE   = '\033[0;34m'
CLR_GREEN  = '\033[0;32m'
CLR_RED    = '\033[0;31m'
CLR_BOLD   = '\033[0;1m'
CLR_RESET  = '\033[0m'

def print_info(msg):
    print(f'[{CLR_BLUE}----{CLR_RESET}] {msg}')

def print_pass(name):
    print(f'[{CLR_GREEN}PASS{CLR_RESET}] Test passed: {name}')

def print_fail(name):
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
    input = path.joinpath('input.ela')
    if not input.is_file():
        error(f"ill-formed test case '{name}': no input.ela")

    obj = work_dir.joinpath(f'{name}.o')
    exe = work_dir.joinpath(name)

    # TODO: better error handling

    res = subprocess.run([str(elc_bin), str(input), str(obj)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        return TestOutput(exitcode=res.returncode, stdout='', stderr='')
    
    res = subprocess.run(['cc', str(obj), '-o', str(exe)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        return TestOutput(exitcode=res.returncode, stdout='', stderr='')
    
    res = subprocess.run([str(exe)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return TestOutput(exitcode=res.returncode, stdout=res.stdout.strip(), stderr=res.stderr.strip())

def main():
    if len(sys.argv) <= 2:
        error('expected argument\nusage: python runner.py <path-to-elc-binary> <work-dir>')
    elc_bin  = Path(sys.argv[1]).resolve()
    work_dir = Path(sys.argv[2]).resolve()

    if not work_dir.exists():
        os.makedirs(str(work_dir), exist_ok=True)

    passed_count = 0
    failed_count = 0
    test_cases = sorted([path for path in script_dir.iterdir() if path.is_dir() and not path.name.startswith('.') and path.joinpath('input.ela').is_file()])

    for path in test_cases:
        name: str = path.name
        expected: TestOutput = get_expected_output(path, name)
        actual:   TestOutput = run_test_case(elc_bin, work_dir, path, name)
        
        if actual == expected:
            print_pass(name)
            passed_count += 1
        else:
            print_fail(name)
            failed_count += 1
            if actual.exitcode != expected.exitcode:
                print_info(f'  exitcode: expected {expected.exitcode}, actual {actual.exitcode}')
            
            print_diff(expected.stdout, actual.stdout, 'stdout')
            print_diff(expected.stderr, actual.stderr, 'stderr')

    tested_count = passed_count + failed_count
    print(f'[{CLR_BLUE}===={CLR_RESET}] {CLR_BOLD}Synthesis: ', end='')
    print(f'Tested: {CLR_BLUE}{tested_count}{CLR_RESET}{CLR_BOLD} ', end='')
    print(f'| Passing: {CLR_GREEN}{passed_count}{CLR_RESET}{CLR_BOLD} ', end='')
    print(f'| Failing: {CLR_RED}{failed_count}{CLR_RESET}{CLR_BOLD}', end='')
    print()

    if failed_count > 0:
        sys.exit(1)

if __name__ == '__main__':
    main()
