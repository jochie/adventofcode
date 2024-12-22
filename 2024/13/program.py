#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
import re
import sys
import json

def parse_options():
    """
    Parser command line options
    """
    parser = argparse.ArgumentParser(
        description='This program is used for one of the AoC 2024 puzzles; Day 13'
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
                        type=int,
                        required=True)
    parser.add_argument('-f', '--filename',
                        help="The filename to read (sample, input)",
                        required=True)
    return parser.parse_args()


# For part 1 I, foolishly, dove right in and implemented an actual
# grid search. As part 2 will show, that wasn't necessary, but I'm
# leaving it here as an example of a grid search, just the same.

def run_part1(opts, machines):
    if opts.debug:
        print(opts)
        print(machines)

    cost_A = 3
    cost_B = 1
    total_tokens = 0
    for machine in machines:
        if opts.debug or opts.verbose:
            print(f"Machine: {machine}")
        attempts = [ [ 0, 0 ] ]
        cheapest = -1
        seen = {"0,0": True}
        while True:
            if opts.debug:
                print(f"Attempts: {attempts}")
            new_attempts = []
            for attempt in attempts:
                # Check the X axis
                x = attempt[0] * machine['A'][0] + attempt[1] * machine['B'][0]
                y = attempt[0] * machine['A'][1] + attempt[1] * machine['B'][1]
                if x == machine['P'][0] and y == machine['P'][1]:
                    cost = attempt[0] * cost_A + attempt[1] * cost_B
                    if opts.verbose:
                        print(f"Solution: {attempt} - Cost: {cost}")
                    if cheapest == -1 or cost < cheapest:
                        cheapest = cost
                    continue
                if x > machine['P'][0] or y > machine['P'][1]:
                    # We've gone too far
                    continue
                cost = attempt[0] * cost_A + attempt[1] * cost_B
                if cheapest != -1 and cost > cheapest:
                    continue
                if f"{attempt[0]+1},{attempt[1]}" not in seen:
                    new_attempts.append([ attempt[0] + 1, attempt[1] ])
                    seen[f"{attempt[0]+1},{attempt[1]}"] = True
                if f"{attempt[0]},{attempt[1]+1}" not in seen:
                    new_attempts.append([ attempt[0], attempt[1] + 1 ])
                    seen[f"{attempt[0]},{attempt[1]+1}"] = True
            if not len(new_attempts):
                break
            attempts = new_attempts
        if cheapest != -1:
            total_tokens += cheapest
    return total_tokens


# For part 2 I ran into a wall and then on the third attempt (I'm
# rusty, okay) I succesfully solved the two equations for 2 variables
# without mixing up some of the constants somewhere along the way.

def run_part2(opts, machines):
    if opts.debug:
        print(opts)
        print(machines)

    cost_A = 3
    cost_B = 1
    total_tokens = 0
    for machine in machines:
        machine['P'][0] += 10000000000000
        machine['P'][1] += 10000000000000

        # Giving the different parts of these equations names that
        # won't make my eyes glaze over

        # Ax = L, Ay = M
        L = machine['A'][0]
        M = machine['A'][1]

        # Bx = N, By = O
        N = machine['B'][0]
        O = machine['B'][1]

        # Px = P, Py = Q
        P = machine['P'][0]
        Q = machine['P'][1]

        # Solving the two equations:

        #    A L + B N = P
        #    A M + B O = Q
        # =>
        #    A L = P - B N
        #    A M = Q - B O
        # =>
        #    A = (P - B N) / L
        #    A = (Q - B O) / M
        # =>
        #    (P - B N) / L = (Q - B O) / M
        # =>
        #    M (P - B N) = L (Q - B O)
        # =>
        #    M P - B M N = L Q - B L O
        # =>
        #    B L O - B M N = L Q - M P
        # =>
        #    B (L O - M N) = (L Q - M P)
        # =>
        #    B = (L Q - M P) / (L O - M N)

        # Numerator
        B_num = (L * Q - M * P)

        # Denominator
        B_den = (L * O - M * N)

        # Does B land on an integer?
        if B_num % B_den > 0:
            continue

        B = B_num // B_den

        # Numerator
        A_num = (P - B * N)

        # Denominator
        A_den = L

        # Does A land on an integer?
        if A_num % A_den > 0:
            continue

        A = A_num // A_den

        # Alternatively:
        # A = (Q - B * O) / M

        cost = A * cost_A + B * cost_B
        if opts.verbose:
            print(f"Cost for {machine}: {cost}")
        total_tokens += cost
    return total_tokens


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()
    machines = []
    try:
        with open(opts.filename, "r", encoding="utf-8") as input_fd:
            machine = {}
            for line in input_fd:
                line = line.rstrip()
                if not len(line):
                    machines.append(machine)
                    machine = {}
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                line_re = re.match("Button ([AB]): X\+(\d+), Y\+(\d+)", line)
                if line_re:
                    machine[line_re.group(1)] = [ int(line_re.group(2)), int(line_re.group(3)) ]
                else:
                    line_re = re.match("Prize: X=(\d+), Y=(\d+)", line)
                    if line_re:
                        machine['P'] = [ int(line_re.group(1)), int(line_re.group(2)) ]

            if len(machine):
                machines.append(machine)
    except FileNotFoundError:
        print(f"ERROR: File not found {opts.filename}")
        sys.exit(1)
    # Done reading

    if opts.debug:
        print(json.dumps(machines))
    # Function to call, expected values, for sanity-checking (sample)
    # and later factoring (both):
    run = {
        1: { "call":    run_part1,
             "sample1":   480,
             "input":   36758 },
        2: { "call":    run_part2,
             "sample1":   875318608908,
             "input":   76358113886726 }
    }

    answer = run[opts.part]["call"](opts, machines)
    if opts.filename in run[opts.part] and answer != run[opts.part][opts.filename]:
        print(f"Warning: Unexpected value from part {opts.part}, filename '{opts.filename}'")
    print(f"Answer: {answer}")


if __name__ == "__main__":
    sys.exit(main())
