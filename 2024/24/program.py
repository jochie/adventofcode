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


def evaluate(opts, wire, wires, gates, checking):
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
    if gate[0] in checking or gate[2] in checking:
        # We have a loop in the gates
        return None

    wire1 = evaluate(opts, gate[0], wires, gates, checking + [ wire ])
    if wire1 is None:
        # We ran into a loop in the gates
        return None

    wire2 = evaluate(opts, gate[2], wires, gates, checking + [ wire ])
    if wire2 is None:
        # We ran into a loop in the gates
        return None

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
        evaluate(opts, wire, wires, gates, [])
    z_wires.sort()
    bitmap = 0
    for wire in z_wires:
        bit = int(wire[1:])
        if wires[wire]:
            bitmap |= 1 << bit
        if opts.debug:
            print(f"{wire}: {wires[wire]}")
    if opts.verbose:
        print(f"Decimal: {bitmap}")
    return bitmap


def bitmap_wires(opts, wire_prefix, wires):
    bitmap = 0
    for wire in wires.keys():
        if wire[0] != wire_prefix:
            continue
        bit = int(wire[1:])
        if wires[wire]:
            bitmap |= 1 << bit
    return bitmap


def set_wires(opts, wire_prefix, wires, value):
    for bit in range(45):
        wires[f"{wire_prefix}{bit:02}"] = value & (1 << bit) != 0


def check_addition(opts, wires, gates):
    for wire in gates.keys():
        if wire[0] != 'z':
            continue
        evaluate(opts, wire, wires, gates, [])

    bitmap_x = bitmap_wires(opts, 'x', wires)
    bitmap_y = bitmap_wires(opts, 'y', wires)
    bitmap_z = bitmap_wires(opts, 'z', wires)
    return bitmap_x + bitmap_y == bitmap_z


def traverse_from(opts, wire, gates):
    collected = {wire: True}
    checking = [wire]
    while True:
        new_checking = []
        for wire in checking:
            if wire not in gates:
                continue
            wire_l = gates[wire][0]
            wire_r = gates[wire][2]
            if wire_l not in collected and wire_l[0] != 'x' and wire_l[0] != 'y':
                new_checking.append(wire_l)
                collected[wire_l] = True
            if wire_r not in collected and wire_r[0] != 'x' and wire_r[0] != 'y':
                new_checking.append(wire_r)
                collected[wire_r] = True
        if not len(new_checking):
            break
        checking = new_checking
    return collected


def check_adders(opts, wires, gates, min_shift, max_shift):
    for shift in range(min_shift, max_shift + 1):
        for x in range(4):
            # 00, 01, 10, 11
            for y in range(4):
                # 00 01, 10, 11
                test_x = x << shift
                test_y = y << shift

                # Reset all wire values
                wires = {}

                set_wires(opts, 'x', wires, test_x)
                set_wires(opts, 'y', wires, test_y)
                if not check_addition(opts, wires, gates):
                    if opts.debug:
                        print(f"Failed for {test_x} + {test_y} (x = {x}, y = {y}, bit shift = {shift})?")
                    return False
    return True


def run_part2(opts, wires, gates):
    if opts.debug:
        print(opts)
        print(wires)
        print(gates)

    # ======================================================================
    # Collecting the wires into groups of the first ZXY wire that
    # required them for functioning

    clusters = []
    cluster = None
    functional = False

    # To track which wires are associated with each different zXY
    # wire, below
    wires_seen = {}

    # Wires associated with the next zXY wire
    z_wires = []

    for bit in range(44):
        impacted = traverse_from(opts, f"z{bit:02}", gates)
        new_wires = []
        for wire in impacted.keys():
            if wire not in wires_seen:
                new_wires.append(wire)
        z_wires.append(new_wires)

    # ======================================================================
    # Send a test pattern through to find the zXY entries that seem
    # flawed

    for bit in range(0, 43):
        if check_adders(opts, wires, gates, bit, bit):
            if not functional:
                if cluster:
                    clusters.append(cluster)
                    cluster = None
                functional = True
        else:
            if functional:
                cluster = [ [], [], {} ]
                functional = False
            cluster[0].append(bit)

    # If there's a cluster at the end (though there wasn't, in my
    # specific input)
    if cluster:
        clusters.append(cluster)

    # Collect the wires that are involved in the calculation of the
    # flawed zXY entry, plus the next one over

    for ix, cluster in enumerate(clusters):
        for z_wire in range(min(cluster[0]), max(cluster[0]) + 2):
            cluster[1].append(z_wire)
            for wire in z_wires[z_wire]:
                cluster[2][wire] = True
        if opts.verbose:
            print(f"Cluster {ix+1}: {cluster[0]} - {len(cluster[2].keys())}")
            print(cluster[1])
            print(cluster[2].keys())

    # Now loop over each cluster of flawed zXY wires and
    # experimentally swap 2 in the group that is associated with them

    all_swapped = []
    for cluster in clusters:
        cluster_wires = list(cluster[2].keys())
        swapped = None
        min_wire = min(cluster[0])
        max_wire = max(cluster[0])
        for x0 in range(len(cluster_wires)):
            wire0 = cluster_wires[x0]
            for x1 in range(x0 + 1, len(cluster_wires)):
                wire1 = cluster_wires[x1]

                # Swap the gates/wires
                (gates[wire0], gates[wire1]) = (gates[wire1], gates[wire0])

                # Recheck just the addition for the zXY wires in this cluster
                if check_adders(opts, wires, gates, min_wire, max_wire):
                    if opts.verbose:
                        print(f"Swapping {wire0} and {wire1} seems to be a fix.")
                    swapped = [ wire0, wire1 ]
                    break

                # Swap the gates/wires back
                (gates[wire0], gates[wire1]) = (gates[wire1], gates[wire0])

            if swapped:
                break
        for wire in swapped:
            all_swapped.append(wire)

    all_swapped.sort()
    return ",".join(all_swapped)


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
                "input":  "fcd,fhp,hmk,rvf,tpc,z16,z20,z33"
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
