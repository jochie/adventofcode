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
        description='This program is used for one of the AoC 2024 puzzles; Day 24'
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


def evaluate(opts, wire, wires, gates):
    if opts.debug:
        print(f"Evaluating {wire}")
    if wire in wires:
        if opts.debug:
            print(f"  Wire value for {wire} -> {wires[wire]}")
        return wires[wire]
    if wire not in gates:
        print(f"Missing entry for {wire}?")
        return None
    gate = gates[wire]
    wire1 = evaluate(opts, gate[0], wires, gates)
    wire2 = evaluate(opts, gate[2], wires, gates)
    if gate[1] == 'AND':
        wires[wire] = wire1 and wire2
    elif gate[1] == 'OR':
        wires[wire] = wire1 or wire2
    elif gate[1] == 'XOR':
        wires[wire] = wire1 ^ wire2
    else:
        print(f"Unexpected gate type for wire {wire}: {gate[1]}")
        sys.exit(1)
    if opts.debug:
        print(f"  {gate} -> {wire1}; {wire2}; {wires[wire]}")
    return wires[wire]


def run_part1(opts, wires, gates):
    if opts.debug:
        print(opts)
        print(wires)
        print(gates)

    z_wires = []
    for wire in gates.keys():
        if wire[0] != 'z':
            continue
        z_wires.append(wire)
        evaluate(opts, wire, wires, gates)
    z_wires.sort()
    bitmap = 0
    for wire in z_wires:
        bit = int(wire[1:])
        if wires[wire]:
            bitmap |= 1 << bit
        if opts.verbose:
            print(f"{wire}: {wires[wire]}")
    if opts.verbose:
        print(f"Decimal: {bitmap}")
    return bitmap

def run_part2(opts, wires, gates):
    if opts.debug:
        print(opts)
        print(wires)
        print(gates)

    return -2


def parse_file(opts, filename):
    wires = {}
    gates = {}
    try:
        with open(filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                if not len(line):
                    break
                line_re = re.match(r"^([^:]+): (\d+)", line)
                wires[line_re.group(1)] = int(line_re.group(2)) == 1

            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                line_re = re.match(r"^([^ ]+) (OR|AND|XOR) ([^ ]+) -> ([^ ]+)", line)
                if line_re.group(4) in gates:
                    print(f"Did we get our wires crossed for {line_re.group(4)}?")
                    sys.exit(1)
                gates[line_re.group(4)] = [ line_re.group(1), line_re.group(2), line_re.group(3) ]

    except FileNotFoundError:
        print(f"ERROR: File not found {filename}")
        sys.exit(1)
    return wires, gates


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
                "sample": 4,
                "sample2": 2024,
                "input":  66055249060558
            }
        },
        2: {
            "call": run_part2,
            "files": {
                "sample": 0,
                "input":  0
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
