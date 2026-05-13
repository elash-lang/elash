from pathlib import Path
from dataclasses import dataclass

import subprocess
import sys
import os

@dataclass
class TestOutput:
    exitcode: int
    stdout:   str
    stderr:   str

script_dir = Path(__file__).resolve().parent

# TODO: fancy ansi output (because its cool)
def error(*args):
    print('error: ', end='')
    print(*args)
    sys.exit(0)

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

    failed = False
    for path in sorted(script_dir.iterdir()):
        if not path.is_dir() or path.name.startswith('.'): continue
        if not path.joinpath('input.ela').is_file(): continue
        
        name: str = path.name
        expected: TestOutput = get_expected_output(path, name)
        actual:   TestOutput = run_test_case(elc_bin, work_dir, path, name)
        
        if actual == expected:
            print(f'{name}: PASSED')
        else:
            print(f'{name}: FAILED')
            print(f'  expected: {expected}')
            print(f'  actual:   {actual}')
            failed = True

    if failed:
        sys.exit(1)

if __name__ == '__main__':
    main()
