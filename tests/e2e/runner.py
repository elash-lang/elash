#!/usr/bin/env python

from pathlib import Path
from typing import Optional

import argparse
import typing
import sys
import os

from runner.models import Timeouts
from runner.suite import run_suite

class CliArgs(argparse.Namespace):
    elc_bin:         Path
    work_dir:        Path
    parallel:        Optional[int]

    compile_timeout: float
    link_timeout:    float
    runtime_timeout: float

def main():
    parser = argparse.ArgumentParser(description="Elash's end-to-end test runner");
    parser.add_argument('elc_bin',  type=Path, help='Path to elc binary');
    parser.add_argument('work_dir', type=Path, help='Path to working directory');
    parser.add_argument(
        '-j', '--parallel', nargs='?',
        const=None, default=1, type=int,
        help='run in parallel with optional N workers limit'
    );

    parser.add_argument(
        '-t', '--timeout', type=float, default=1.0,
        help='base timeout multiplier'
    )
    parser.add_argument('--compile-timeout', type=float, help='compile stage timeout (default: 1.5s * timeout)')
    parser.add_argument('--link-timeout',    type=float, help='link stage timeout (default: 5s * timeout)')
    parser.add_argument('--runtime-timeout', type=float, help='runtime stage timeout (default: 3s * timeout)')

    #very advanced static typing
    args: CliArgs = typing.cast(CliArgs, parser.parse_args())

    timeouts = Timeouts(
        compile = (args.compile_timeout or 1.5) * args.timeout,
        runtime = (args.runtime_timeout or 3.0) * args.timeout,
        link    = (args.link_timeout    or 5.0) * args.timeout,
    )

    elc_bin  = args.elc_bin.resolve()
    work_dir = args.work_dir.resolve()

    if not work_dir.exists():
        os.makedirs(str(work_dir), exist_ok=True)

    if not run_suite(elc_bin, work_dir, args.parallel, timeouts):
        sys.exit(1)

if __name__ == '__main__':
    main()
