#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
import re
import sys


def parse_options():
    """
    Parser command line options
    """
    parser = argparse.ArgumentParser(
        description='This program is used for one of the AoC 2024 puzzles; Day 16'
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


DIRS = [
    [ -1,  0, 0 ],
    [  0,  1, 1 ],
    [  1,  0, 2 ],
    [  0, -1, 3 ]
]

def run_part1(opts, grid, max_row, max_col, start_row, start_col, end_row, end_col):
    if opts.debug:
        print(opts)
        print(lines)

    minimal_cost = 0
    searching = [ [ start_row, start_col, 0, 1 ] ]
    best_seen = {}
    while True:
        new_searching = []
        for row, col, cost, face in searching:
            if minimal_cost > 0 and cost > minimal_cost:
                # No point in continuing here
                continue

            for rel_row, rel_col, rel_face in DIRS:
                new_row = row + rel_row
                new_col = col + rel_col

                if grid[new_row][new_col] == '#':
                    # Hitting a wall
                    continue

                if face == (rel_face + 2) % 4:
                    # Opposite direction doesn't make sense?
                    continue

                if grid[new_row][new_col] == 'S':
                    # Back at the start? That's no good.
                    continue

                if grid[new_row][new_col] == '.':
                    new_key = f"{new_row},{new_col},{rel_face}"
                    if face == rel_face:
                        new_cost = cost + 1
                    else:
                        new_cost = cost + 1001

                    if new_key in best_seen and best_seen[new_key] < new_cost:
                        # Don't bother searching further if we've
                        # already visited here (facing the same way)
                        # with a better cost
                        continue
                        
                    new_searching.append([
                        new_row, new_col, new_cost, rel_face
                    ])
                    best_seen[new_key] = new_cost
                    continue

                if grid[new_row][new_col] != 'E':
                    print(f"Found an unexpected item on ({new_row},{new_col}): '{grid[new_row][new_col]}'")
                    sys.exit(1)

                # Found the end point. How did we do?
                if face == rel_face:
                    # Going the same direction still, 1 point
                    cost += 1
                else:
                    # That means we're going to turn
                    cost += 1001
                if not minimal_cost or cost < minimal_cost:
                    if opts.verbose:
                        print(f"Found a path for {cost} points.")
                    minimal_cost = cost

        if not len(new_searching):
            break

        searching = new_searching

    return minimal_cost


def run_part2(opts, grid, max_row, max_col, start_row, start_col, end_row, end_col):
    if opts.debug:
        print(opts)
        print(lines)

    minimal_cost = 0
    searching = [ [ start_row, start_col, 0, 1, [] ] ]
    best_seen = {}
    best_positions = {}
    while True:
        new_searching = []
        for row, col, cost, face, positions in searching:
            if minimal_cost > 0 and cost > minimal_cost:
                # No point in continuing here
                continue

            for rel_row, rel_col, rel_face in DIRS:
                new_row = row + rel_row
                new_col = col + rel_col

                if grid[new_row][new_col] == '#':
                    # Hitting a wall
                    continue

                if face == (rel_face + 2) % 4:
                    # Opposite direction doesn't make sense?
                    continue

                if grid[new_row][new_col] == 'S':
                    # Back at the start? That's no good.
                    continue

                if grid[new_row][new_col] == '.':
                    new_key = f"{new_row},{new_col},{rel_face}"
                    if face == rel_face:
                        new_cost = cost + 1
                    else:
                        new_cost = cost + 1001

                    if new_key in best_seen and best_seen[new_key] < new_cost:
                        # Don't bother searching further if we've
                        # already visited here (facing the same way)
                        # with a better cost
                        continue
                        
                    new_searching.append([
                        new_row, new_col, new_cost, rel_face,
                        positions + [[ new_row, new_col ]]
                    ])
                    best_seen[new_key] = new_cost
                    continue

                if grid[new_row][new_col] != 'E':
                    print(f"Found an unexpected item on ({new_row},{new_col}): '{grid[new_row][new_col]}'")
                    sys.exit(1)

                # Found the end point. How did we do?
                if face == rel_face:
                    # Going the same direction still, 1 point
                    cost += 1
                else:
                    # That means we're going to turn
                    cost += 1001
                if minimal_cost and cost == minimal_cost:
                    if opts.verbose:
                        print(f"Found a path that matches {cost} points.")
                    for pos in positions:
                        best_positions[f"{pos[0]},{pos[1]}"] = True
                if not minimal_cost or cost < minimal_cost:
                    if opts.verbose:
                        print(f"Found a path for {cost} points.")
                    minimal_cost = cost
                    best_positions = {}
                    for pos in positions:
                        best_positions[f"{pos[0]},{pos[1]}"] = True

        if not len(new_searching):
            break

        searching = new_searching

    # Add 2 for the 'S' and 'E' positions
    return len(best_positions) + 2


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()
    grid = []
    max_row = 0
    max_col = 0
    start_row = 0
    start_col = 0
    end_row = 0
    end_col = 0
    try:
        with open(opts.filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                grid.append(list(line))
                if not max_col:
                    max_col = len(line)
                for col in range(max_col):
                    if line[col] == 'S':
                        start_row = max_row
                        start_col = col
                    elif line[col] == 'E':
                        end_row = max_row
                        end_col = col
                max_row += 1
    except FileNotFoundError:
        print(f"ERROR: File not found {opts.filename}")
        sys.exit(1)
    # Done reading

    # Function to call, expected values, for sanity-checking (sample)
    # and later factoring (both):
    run = {
        1: { "call": run_part1,
             "sample1":  7036,
             "sample2": 11048,
             "input":   75416
            },
        2: { "call":   run_part2,
             "sample1":  45,
             "sample2":  64,
             "input":   476
            }
    }

    answer = run[opts.part]["call"](opts, grid, max_row, max_col, start_row, start_col, end_row, end_col)
    if opts.filename in run[opts.part]:
        if answer == run[opts.part][opts.filename]:
            if opts.verbose:
                print(f"Confirmed expected value from part {opts.part}, filename '{opts.filename}'")
        else:
            print(f"Warning: Unexpected value from part {opts.part}, filename '{opts.filename}'")
    else:
        print(f"Warning: No known answer for part {opts.part}, filename '{opts.filename}'")
    print(f"Answer: {answer}")


if __name__ == "__main__":
    sys.exit(main())
