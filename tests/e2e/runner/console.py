import threading
import builtins
import difflib
import sys

from typing import Optional

CLR_BLUE   = '\033[0;34m'
CLR_GREEN  = '\033[0;32m'
CLR_RED    = '\033[0;31m'
CLR_ORANGE = '\033[38;5;166m'
CLR_BOLD   = '\033[0;1m'
CLR_RESET  = '\033[0m'

PRINT_LOCK = threading.Lock()
def print(*args, **kwargs):
    with PRINT_LOCK:
        builtins.print(*args, **kwargs)

def eprint(*a, **k):
    return print(*a, **k, file=sys.stderr)

def print_info(msg: str, err: bool = True):
    (eprint if err else print)(f'[{CLR_BLUE}----{CLR_RESET}] {msg}')

def print_pass(name: str):
    print(f'[{CLR_GREEN}PASS{CLR_RESET}] Test passed: {name}')
def print_skip(name: str):
    print(f'[{CLR_BLUE}SKIP{CLR_RESET}] Test skipped: {name}')
def print_fail(name: str):
    eprint(f'[{CLR_RED}FAIL{CLR_RESET}] Test failed: {name}')
def print_timeout(name: str):
    eprint(f'[{CLR_ORANGE}TIME{CLR_RESET}] Test timed out: {name}')

def error(*args):
    eprint(f'{CLR_RED}error: {CLR_RESET}', end='')
    eprint(*args)
    sys.exit(1)

def print_diff(expected: str, actual: str, stream_name: Optional[str] = None):
    diff = difflib.unified_diff(
        expected.splitlines(keepends=True),
        actual.splitlines(keepends=True),
        fromfile=f'expected {stream_name}' if stream_name is not None else '',
        tofile=f'actual {stream_name}' if stream_name is not None else '',
    )

    for line in diff:
        if stream_name is None:
            if line.startswith("---") or line.startswith("+++"):
                continue
        print_info(f'  {line.rstrip()}')
