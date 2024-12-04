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
        description='This program is used for one of the AoC 2024 puzzles; Day 4, part 2'
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

MAS = list("MAS")

def search_mas(opts, grid, row, col, collected):
    total = 0
    for dir_row in [-1, 0, 1]:
        for dir_col in [-1, 0, 1]:
            if dir_row == 0 and dir_col == 0:
                # That's not a direction :)
                continue
            if abs(dir_row) != abs(dir_col):
                # Only search diagonal directions
                continue
            new_row = row
            new_col = col
            found = True
            for letter in range(len(MAS)):
                if new_row < 0 or new_row >= len(grid):
                    found = False
                    break
                if new_col < 0 or new_col >= len(grid[new_row]):
                    found = False
                    break
                if grid[new_row][new_col] != MAS[letter]:
                    found = False
                    break
                new_row += dir_row
                new_col += dir_col
            if found:
                # Mark the spot where the A is
                loc = f"{row + dir_row},{col + dir_col}"
                if loc not in collected:
                    collected[loc] = 0
                collected[loc] += 1


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    grid = []
    opts = parse_options()
    for line in sys.stdin:
        line = line.rstrip()
        grid.append(list(line))
        if opts.debug:
            print("DEBUG: Line received: '{}'".format(line))
    # Done
    all_found = 0
    collected = {}
    for row in range(len(grid)):
        for col in range(len(grid[row])):
            search_mas(opts, grid, row, col, collected)

    for loc, val in collected.items():
        if val == 2:
            all_found += 1
    print(f"Total X-MAS found: {all_found}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
