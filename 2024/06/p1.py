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
        description='This program is used for one of the AoC 2024 puzzles; Day 6, part 1'
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
    paces = 0
    direction = 0

    positions = {}
    positions[f"{start_row},{start_col}"] = True
    while True:
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

    print(f"Positions seen: {len(positions.keys())}; Paces walked: {paces}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
