#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
import copy
import re
import sys
import time


def parse_options():
    """
    Parser command line options
    """
    parser = argparse.ArgumentParser(
        description='This program is used for one of the AoC 2024 puzzles; Day 20'
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


# These solutions are suboptimal for the input we were given, because
# there is only one path from S to E, which I didn't realize (because
# it was pointed out to me) until after I was done.


DIRS = [
    [ -1,  0 ],
    [  0,  1 ],
    [  1,  0 ],
    [  0, -1 ]
]


def on_grid(max_row, max_col, row, col):
    return row >= 0 and col >= 0 and row < max_row and col < max_col


def find_non_cheating(opts, grid, max_row, max_col, s_row, s_col, e_row, e_col):
    # row, col, steps
    checking = [ [ s_row, s_col, 0 ] ]

    seen = {}
    while checking:
        new_checking = []
        for row, col, steps in checking:
            for rel_row, rel_col in DIRS:
                new_row = row + rel_row
                new_col = col + rel_col
                if not on_grid(max_row, max_col, new_row, new_col):
                    continue

                if new_row == e_row and new_col == e_col:
                    return steps + 1

                if grid[new_row][new_col] == '#':
                    continue

                new_key = f"{new_row},{new_col}"
                if new_key in seen:
                    continue
                seen[new_key] = True
                new_checking.append([ new_row, new_col, steps + 1 ])
        if not len(new_checking):
            break
        checking = new_checking
    return 0


def find_cheats(opts, grid, max_row, max_col):
    cheats = []
    for row in range(1, max_row - 1):
        for col in range(1, max_col - 1):
            if grid[row][col] != '#':
                continue
            if grid[row - 1][col] != '#' and grid[row + 1][col] != '#':
                cheats.append([ row, col ])
            if grid[row][col - 1] != '#' and grid[row][col + 1] != '#':
                cheats.append([ row, col ])
    return cheats


def run_part1(opts, grid, max_row, max_col, s_row, s_col, e_row, e_col):
    if opts.debug:
        print(opts)

    beat_this = find_non_cheating(opts, grid, max_row, max_col, s_row, s_col, e_row, e_col)
    if opts.verbose:
        print(f"Steps to beat: {beat_this}")
    beat_100 = 0
    results = {}
    cheats = find_cheats(opts, grid, max_row, max_col)
    if opts.debug:
        print(len(cheats))

    for cheat_row, cheat_col in cheats:
        # Pretend we can walk through this wall:
        grid[cheat_row][cheat_col] = '.'

        cheat_steps = find_non_cheating(opts, grid, max_row, max_col, s_row, s_col, e_row, e_col)

        # Restore the wall:
        grid[cheat_row][cheat_col] = '#'

        if cheat_steps >= beat_this:
            # Not better?
            continue

        cheat_beat = 1
        if max_row > 100:
            cheat_beat = 100
        cheat_diff = beat_this - cheat_steps
        if cheat_diff not in results:
            results[cheat_diff] = 0
        results[cheat_diff] += 1
        if cheat_diff >= cheat_beat:
            beat_100 += 1

    if opts.verbose:
        r_keys = list(results.keys())
        r_keys.sort()
        for r_key in r_keys:
            print(f"There {'are' if results[r_key] > 1 else 'is'} {results[r_key]} cheats that save {r_key} picoseconds.")
    return beat_100


# Find cheats up to 20 steps/picoseconds
def find_mega_cheats(opts, grid, max_row, max_col):
    cheats = []
    empties = []

    for row in range(1, max_row - 1):
        for col in range(1, max_col - 1):
            if grid[row][col] != '#':
                empties.append([ row, col ])

    # Create permutations of empties
    for from_row, from_col in empties:
        for to_row, to_col in empties:
            if from_row == to_row and from_col == to_col:
                continue
            steps = abs(from_row - to_row) + abs(from_col - to_col)
            all_empty = True
            if steps > 20:
                continue
            cheats.append([ from_row, from_col, to_row, to_col, steps ])
    return cheats


def find_cheating_endpoints(opts, grid, max_row, max_col, s_row, s_col, beat_this, endpoints):
    # row, col, steps
    checking = [ [ s_row, s_col, 0 ] ]

    len_endpoints = len(endpoints)
    endpoint_steps = {}
    key_s = f"{s_row},{s_col}"
    if key_s in endpoints:
        endpoint_steps[key_s] = 0
    results = {}
    seen = {}
    while checking:
        new_checking = []
        for row, col, steps in checking:
            if steps > beat_this:
                continue
            for rel_row, rel_col in DIRS:
                new_row = row + rel_row
                new_col = col + rel_col
                if not on_grid(max_row, max_col, new_row, new_col):
                    continue

                if grid[new_row][new_col] == '#':
                    continue

                new_key = f"{new_row},{new_col}"
                if new_key in seen:
                    continue

                if new_key in endpoints and new_key not in endpoint_steps:
                    # Had not found this cheat-endpoint yet
                    endpoint_steps[new_key] = steps + 1
                    if len(endpoint_steps) == len_endpoints:
                        # Done!
                        return endpoint_steps
                seen[new_key] = True
                new_checking.append([ new_row, new_col, steps + 1 ])
        if not len(new_checking):
            break
        checking = new_checking
    return endpoint_steps


def run_part2(opts, grid, max_row, max_col, s_row, s_col, e_row, e_col):
    if opts.debug:
        print(opts)

    beat_this = find_non_cheating(opts, grid, max_row, max_col, s_row, s_col, e_row, e_col)
    if opts.verbose:
        print(f"Steps to beat: {beat_this}")
    cheats_list = find_mega_cheats(opts, grid, max_row, max_col)

    endpoints = {}
    for cheats in cheats_list:
        endpoints[f"{cheats[0]},{cheats[1]}"] = True
        endpoints[f"{cheats[2]},{cheats[3]}"] = True

    start_steps = find_cheating_endpoints(opts, grid, max_row, max_col, s_row, s_col, beat_this, endpoints)
    if opts.debug:
        print(start_steps)
    end_steps = find_cheating_endpoints(opts, grid, max_row, max_col, e_row, e_col, beat_this, endpoints)
    if opts.debug:
        print(end_steps)

    beat_100 = 0
    results = {}

    for cheat in cheats_list:
        s_steps = start_steps[f"{cheat[0]},{cheat[1]}"]
        e_steps = end_steps[f"{cheat[2]},{cheat[3]}"]
        if opts.debug:
            print(f"{cheat} - {s_steps} - {e_steps}")
        cheat_steps = s_steps + e_steps + cheat[4]
        if cheat_steps >= beat_this:
            # Not better?
            continue

        cheat_beat = 50
        if max_row > 100:
            cheat_beat = 100
        cheat_diff = beat_this - cheat_steps
        if cheat_diff >= cheat_beat:
            if cheat_diff not in results:
                results[cheat_diff] = 0
            results[cheat_diff] += 1
            beat_100 += 1

    if opts.verbose:
        r_keys = list(results.keys())
        r_keys.sort()
        for r_key in r_keys:
            print(f"There {'are' if results[r_key] > 1 else 'is'} {results[r_key]} cheats that save {r_key} picoseconds.")

    return beat_100


def parse_file(opts, filename):
    grid = []
    max_row = 0
    max_col = 0
    s_row = 0
    s_col = 0
    e_row = 0
    e_col = 0
    try:
        with open(filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                grid.append(list(line))
                if not max_col:
                    max_col = len(line)
                for col in range(max_col):
                    if line[col] == 'E':
                        e_row = max_row
                        e_col = col
                    elif line[col] == 'S':
                        s_row = max_row
                        s_col = col
                max_row += 1
    except FileNotFoundError:
        print(f"ERROR: File not found {filename}")
        sys.exit(1)
    return grid, max_row, max_col, s_row, s_col, e_row, e_col


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
                "sample": 44,
                "input":  1454
            }
        },
        2: {
            "call": run_part2,
            "files": {
                "sample": 285,
                "input":  997879
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
