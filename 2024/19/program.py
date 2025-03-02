#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
from functools import cache
import re
import sys
import time


def parse_options():
    """
    Parser command line options
    """
    parser = argparse.ArgumentParser(
        description='This program is used for one of the AoC 2024 puzzles; Day 18'
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


def match_patterns(opts, design, patterns):
    for pattern in patterns:
        if len(pattern) > len(design):
            continue
        if design[:len(pattern)] == pattern:
            if len(pattern) == len(design):
                return True
            if match_patterns(opts, design[len(pattern):], patterns):
                return True
    return False


def run_part1(opts, patterns, designs):
    if opts.debug:
        print(opts)
        print(patterns, designs)

    possible = 0
    for design in designs:
        if match_patterns(opts, design, patterns):
            if opts.verbose:
                print(f"Design '{design}' is possible")
            possible += 1
        else:
            if opts.verbose:
                print(f"Design '{design}' is impossible")
    return possible


def run_part2(opts, patterns, designs):

    @cache
    def count_patterns_matches(design):
        count = 0
        for pattern in patterns:
            if len(pattern) > len(design):
                continue
            if design[:len(pattern)] == pattern:
                if len(pattern) == len(design):
                    count += 1
                else:
                    sub_count = count_patterns_matches(design[len(pattern):])
                    count += sub_count
        return count

    if opts.debug:
        print(opts)
        print(patterns, designs)

    total = 0
    for design in designs:
        count = count_patterns_matches(design)
        if count > 0:
            if opts.verbose:
                print(f"Design '{design}' is possible in {count} ways")
            total += count
        else:
            if opts.verbose:
                print(f"Design '{design}' is impossible")
    return total


def parse_file(opts, filename):
    patterns = None
    designs = []
    try:
        with open(filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                if not patterns:
                    patterns = line.split(", ")
                    continue
                if not len(line):
                    continue
                designs.append(line)
    except FileNotFoundError:
        print(f"ERROR: File not found {filename}")
        sys.exit(1)
    return patterns, designs


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
    stdin content, and act on it
    """
    opts = parse_options()

    # Function to call, expected values, for sanity-checking (sample)
    # and later factoring (both):
    run = {
        1: {
            "call": run_part1,
            "files": {
                "sample": 6,
                "input":  319
            }
        },
        2: {
            "call": run_part2,
            "files": {
                "sample": 16,
                "input":  692575723305545
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
