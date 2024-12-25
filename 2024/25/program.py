#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
import re
import sys
import time


def parse_options():
    """
    Parser command line options
    """
    parser = argparse.ArgumentParser(
        description='This program is used for one of the AoC 2024 puzzles; Day 25'
    )
    parser.add_argument('-d', '--debug',
                        help="Enable debug output",
                        default=False,
                        action='store_true')
    parser.add_argument('-n', '--dryrun',
                        help="Enable dryrun (noop) output",
                        default=False,
                        action='store_true')
    parser.add_argument('-v', '--verbose',
                        help="Enable verbose output",
                        default=False,
                        action='store_true')
    parser.add_argument('-p', '--part',
                        help="Which part to run",
                        type=int)
    parser.add_argument('-f', '--filename',
                        help="The filename to read (sample, input)")
    parser.add_argument('-t', '--test',
                        help="Run the program with all known files, and all parts unless one is specified.",
                        action='store_true')

    opts = parser.parse_args()

    if not opts.test:
        if not opts.part or not opts.filename:
            print("The --part and --filename options are required unless you use --test.")
            sys.exit(1)

    if opts.test and opts.filename:
        print("The --test and --filename options are mutually exclusive.")
        sys.exit(1)

    return opts


def convert_key(opts, key):
    pins = [ 0, 0, 0, 0, 0 ]
    for ix, row in enumerate(key):
        for col in range(5):
            if not pins[col] and row[col] == '#':
                pins[col] = 6 - ix
    return pins


def convert_lock(opts, lock):
    pins = [ 0, 0, 0, 0, 0 ]
    for ix, row in enumerate(lock):
        for col in range(5):
            if row[col] == '#':
                pins[col] = ix
    return pins


def run_part1(opts, locks, keys):
    if opts.debug:
        print(opts)
        print(len(locks), len(keys))

    lock_pins = []
    for lock in locks:
        lock_pins.append(convert_lock(opts, lock))

    key_pins = []
    for key in keys:
        key_pins.append(convert_key(opts, key))

    total_fits = 0
    for key_pin in key_pins:
        for lock_pin in lock_pins:
            fits = True
            for pin in range(5):
                if lock_pin[pin] + key_pin[pin] > 5:
                    fits = False
                    break
            if fits:
                total_fits += 1
    return total_fits


def parse_file(opts, filename):
    locks = []
    keys  = []
    lines = []
    try:
        with open(filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                if len(line):
                    lines.append(line)
                    continue
                if lines[0] == '.....':
                    keys.append(lines)
                    lines = []
                elif lines[0] == '#####':
                    locks.append(lines)
                    lines = []
                else:
                    print(f"Unexpected lines: {lines}")
                    sys.exit(1)

            if len(lines):
                if lines[0] == '.....':
                    keys.append(lines)
                elif lines[0] == '#####':
                    locks.append(lines)

    except FileNotFoundError:
        print(f"ERROR: File not found {filename}")
        sys.exit(1)
    return locks, keys


def run_part(opts, run, part, filename, params):
    success = False
    time_before = time.process_time()
    answer = run[part]["call"](opts, *params)
    time_diff = time.process_time() - time_before
    if filename in run[part]["files"]:
        if answer == run[part]["files"][filename]:
            if opts.verbose:
                print(f"Confirmed expected value from part {part}, filename '{filename}'")
            success = True
        else:
            print(f"Warning: Unexpected value from part {part}, filename '{filename}'")
    else:
        print(f"Warning: No known answer for part {part}, filename '{filename}'")
    print(f"[Duration {time_diff:.2f}s] Part {part}, filename '{filename}', answer: {answer}")
    return success


def main():
    """
    Main section, where we parse the command line options, read the
    content, and act on it
    """
    opts = parse_options()

    # Function to call, expected values, for sanity-checking (sample)
    # and later factoring (both):
    run = {
        1: {
            "call": run_part1,
            "files": {
                "sample": 3,
                "input":  3307
            }
        }
    }

    if not opts.test:
        params = parse_file(opts, opts.filename)
        run_part(opts, run, opts.part, opts.filename, params)
        return 0

    passed = 0
    failed = 0
    for part in run.keys():
        if opts.part and opts.part != part:
            continue
        for filename in run[part]["files"].keys():
            params = parse_file(opts, filename)
            if run_part(opts, run, part, filename, params):
                passed += 1
            else:
                failed += 1
    print(f"Test results: {passed} passed, {failed} failed.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
