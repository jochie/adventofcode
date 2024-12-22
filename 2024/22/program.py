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
        description='This program is used for one of the AoC 2024 puzzles; Day 22'
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


def iterate_secret_number(opts, secret, iterations):
    for ix in range(iterations):
        secret = ((secret *   64) ^ secret) % 16777216
        secret = ((secret //  32) ^ secret) % 16777216
        secret = ((secret * 2048) ^ secret) % 16777216
    return secret


def run_part1(opts, lines):
    if opts.debug:
        print(opts)
        print(lines)

    total = 0
    for secret in lines:
        secret = int(secret)
        total += iterate_secret_number(opts, secret, 2000)
    return total


def collect_secret_numbers(opts, secret, iterations):
    secrets = [ [ secret, secret % 10, 0 ] ]
    for ix in range(iterations):
        last_secret = secret % 10
        secret = ((secret *   64) ^ secret) % 16777216
        secret = ((secret //  32) ^ secret) % 16777216
        secret = ((secret * 2048) ^ secret) % 16777216
        secrets.append([ secret, secret % 10, secret % 10 - last_secret ])
    return secrets


def analyze_secret_sequence(opts, secrets):
    sequence = []
    bananas = {}
    for secret, digit, diff in secrets[1:]:
        sequence.append(diff)
        if len(sequence) > 4:
            sequence = sequence[1:]
        if len(sequence) == 4:
            sequence_key = ",".join([ str(x) for x in sequence ])
            if sequence_key in bananas:
                continue
            bananas[sequence_key] = digit
    return bananas


def run_part2(opts, lines):
    if opts.debug:
        print(opts)
        print(lines)

    banana_pile = []
    all_bananas = {}
    for ix, secret in enumerate(lines):
        secret = int(secret)
        secrets = collect_secret_numbers(opts, secret, 2000)
        bananas = analyze_secret_sequence(opts, secrets)
        banana_pile.append(bananas)
        for key, val in bananas.items():
            if key not in all_bananas:
                all_bananas[key] = [ [], 0 ]
            all_bananas[key][0].append(ix)
            all_bananas[key][1] += val
    max_key = None
    max_data = None
    for key, data in all_bananas.items():
        if not max_key or data[1] > max_data[1]:
            max_key = key
            max_data = data
    if opts.verbose:
        print(f"With sequence {max_key} you get {max_data[1]} bananas from {len(max_data[0])} buyers")
    return max_data[1]


def parse_file(opts, filename):
    lines = []
    try:
        with open(filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                lines.append(line)
    except FileNotFoundError:
        print(f"ERROR: File not found {filename}")
        sys.exit(1)
    return lines,


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
                "sample": 37327623,
                "input":  17724064040
            }
        },
        2: {
            "call": run_part2,
            "files": {
                "sample1": 9,
                "sample2": 23,
                "input":  1998
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
