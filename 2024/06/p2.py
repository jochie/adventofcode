#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
import sys

def parse_options():
    """
    Parser command line options
    """
    parser = argparse.ArgumentParser(
        description='This program is used for one of the AoC 2024 puzzles; Day 6, part 2'
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
    return parser.parse_args()

DIR = [
    [ -1,  0 ],
    [  0,  1 ],
    [  1,  0 ],
    [  0, -1 ]
]

def walk_grid(opts, grid, max_row, max_col, start_row, start_col):
    paces = 0
    direction = 0
    positions = {}
    positions[f"{start_row},{start_col}"] = True
    looped = False

    seen = {}
    seen[f"{start_row},{start_col},{direction}"] = True

    while True:
        # print(start_row, start_col, direction)
        next_row = start_row + DIR[direction][0]
        next_col = start_col + DIR[direction][1]
        if next_row < 0 or next_col < 0 or next_row >= max_row or next_col >= max_col:
            # Walking off the grid? Then we're done.
            break
        if grid[next_row][next_col] == '#':
            # Turn and try again
            direction = (direction + 1) % 4
        else:
            paces += 1
            start_row = next_row
            start_col = next_col
            positions[f"{start_row},{start_col}"] = True
            pos_key = f"{start_row},{start_col},{direction}"
            if pos_key in seen:
                looped = True
                break
            seen[pos_key] = True

    return looped, paces, positions.keys()

def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    start_row = 0
    start_col = 0
    max_col = 0
    grid = []
    opts = parse_options()
    for line in sys.stdin:
        line = line.rstrip()
        if opts.debug:
            print("DEBUG: Line received: '{}'".format(line))
        grid.append(list(line))
        if not max_col:
            max_col = len(line)
        for ix in range(len(line)):
            if line[ix] == '^':
                start_row = len(grid) - 1
                start_col = ix
    # Done
    max_row = len(grid)

    looped, paces, positions = walk_grid(opts, grid, max_row, max_col, start_row, start_col)
    if opts.verbose:
        print(f"Initial walk through - Positions seen: {len(positions)}; Paces walked: {paces}")

    loops_found = 0
    for pos_key in positions:
        pos_row, pos_col = pos_key.split(",")
        pos_row = int(pos_row)
        pos_col = int(pos_col)
        if pos_row == start_row and pos_col == start_col:
            continue

        # Let's see if dropping an obstacle here causes a loop
        grid[pos_row][pos_col] = '#'
        looped, paces, positions = walk_grid(opts, grid, max_row, max_col, start_row, start_col)
        if looped:
            loops_found += 1
        if opts.debug:
            print(f"Looped: {looped}; Positions seen: {len(positions)}; Paces walked: {paces}")
        grid[pos_row][pos_col] = '.'

    print(f"Loops found: {loops_found}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
