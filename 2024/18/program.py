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


def dump_grid(grid):
    for line in grid:
        print("".join(line))


DIRS = [
    [ -1,  0  ],
    [  0,  1  ],
    [  1,  0  ],
    [  0, -1  ]
]


def on_grid(max_row, max_col, row, col):
    return row >= 0 and col >= 0 and row < max_row and col < max_col


def reachable(opts, grid, max_row, max_col):
    # Find a path to (max_row,max_col)
    checking = [
        [ 0, 0, [] ]
    ]

    steps = 1
    seen = {f"0,0": True}
    while True:
        new_checking = []
        for row, col, history in checking:
            for rel_row, rel_col in DIRS:
                new_row = row + rel_row
                new_col = col + rel_col

                if not on_grid(max_row, max_col, new_row, new_col):
                    continue

                if new_row == max_row - 1 and new_col == max_col - 1:
                    # Arrived
                    return steps, history

                if grid[new_row][new_col] == '#':
                    # Can't go there
                    continue

                new_key = f"{new_row}.{new_col}"
                if new_key in seen:
                    # Already been there
                    continue
                seen[new_key] = True
                new_checking.append([ new_row, new_col, history + [ f"{new_row},{new_col}" ] ])
        checking = new_checking
        steps += 1
        if not len(checking):
            break

    # Return -1 to indicate no path was found
    return 0, [ ]


def initialize_grid(max_row, max_col):
    grid = []
    for y in range(max_row):
        line = []
        for x in range(max_col):
            line.append('.')
        grid.append(line)
    return grid


def run_part1(opts, coordinates, max_row, max_col, initial):
    if opts.debug:
        print(opts)
        print(coordinates, max_row, max_col, initial)

    grid = initialize_grid(max_row, max_col)
    ix = 0
    for x, y in coordinates:
        grid[y][x] = '#'
        if opts.verbose:
            print(f"(x,y) = ({x},{y})")
            dump_grid(grid)
            print()
        ix += 1
        if ix == initial:
            break

    result = reachable(opts, grid, max_row, max_col)
    return result[0]


def run_part2(opts, coordinates, max_row, max_col, initial):
    if opts.debug:
        print(opts)
        print(coordinates, max_row, max_col, initial)

    grid = initialize_grid(max_row, max_col)
    ix = 0
    path = {}
    for x, y in coordinates:
        grid[y][x] = '#'
        if opts.verbose:
            print(f"(x,y) = ({x},{y})")
            dump_grid(grid)
            print()
        ix += 1
        if ix <= initial:
            # We already know (from part 1) we can get through for those first 1024 bytes
            continue

        if len(path) and f"{y},{x}" not in path:
            continue

        result = reachable(opts, grid, max_row, max_col)
        if not result[0]:
            return f"{x},{y}"
        path = { x: True for x in result[1] }
    return "?,?"


def parse_file(opts, filename):
    coordinates = []
    try:
        with open(filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                coordinates.append([ int(x) for x in line.split(",") ])
    except FileNotFoundError:
        print(f"ERROR: File not found {filename}")
        sys.exit(1)
    return [ coordinates ]


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
    grid = {
        "sample": [  7,  7 ],
        "input":  [ 71, 71 ]
    }
    initial = {
        "sample":   12,
        "input":  1024
    }
    run = {
        1: {
            "call": run_part1,
            "files": {
                "sample":  22,
                "input":  438
            }
        },
        2: {
            "call": run_part2,
            "files": {
                "sample": "6,1",
                "input":  "26,22"
            }
        }
    }

    if not opts.test:
        params = parse_file(opts, opts.filename)
        params.append(grid[opts.filename][0])
        params.append(grid[opts.filename][1])
        params.append(initial[opts.filename])
        run_part(opts, run, opts.part, opts.filename, params)
        return 0

    passed = 0
    failed = 0
    for part in run.keys():
        if opts.part and opts.part != part:
            continue
        for filename in run[part]["files"].keys():
            params = parse_file(opts, filename)
            params.append(grid[filename][0])
            params.append(grid[filename][1])
            params.append(initial[filename])
            if run_part(opts, run, part, filename, params):
                passed += 1
            else:
                failed += 1
    print(f"Test results: {passed} passed, {failed} failed.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
