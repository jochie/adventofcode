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
        description='This program is used for one of the AoC 2024 puzzles; Day 12'
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


# Basic vector/position class
class Vector:
    def __init__(self, r, c):
        self.row = r
        self.col = c

    def __add__(self, o):
        return Vector(self.row + o.row, self.col + o.col)

    def __sub__(self, o):
        return Vector(self.row - o.row, self.col - o.col)

    def __repr__(self):
        return f"({self.row},{self.col})"

    def __eq__(self, o):
        return self.row == o.row and self.col == o.col


DIRS = [
    Vector(-1,  0),
    Vector( 0,  1),
    Vector( 1,  0),
    Vector( 0, -1)
]


def on_grid(grid, max_row, max_col, pos):
    return pos.row >= 0 and pos.row < max_row and pos.col >= 0 and pos.col < max_col


def map_region(opts, grid, max_row, max_col, pos, letter):
    search = [ pos ]
    found = { str(pos): [pos.row, pos.col] }
    area = 1
    perimeter = 4
    while True:
        new_search = []
        for pos in search:
            for dir1 in DIRS:
                new_pos = pos + dir1

                if str(new_pos) in found:
                    continue

                if (on_grid(grid, max_row, max_col, new_pos) and
                    grid[new_pos.row][new_pos.col] == letter):
                    found[str(new_pos)] = [new_pos.row, new_pos.col]
                    area += 1
                    perimeter += 4
                    for dir2 in DIRS:
                        new2_pos = new_pos + dir2
                        if (on_grid(grid, max_row, max_col, new2_pos) and
                            grid[new2_pos.row][new2_pos.col] == letter and
                            str(new2_pos) in found):
                            if opts.debug:
                                print(letter, f"{new2_pos} touches")
                            perimeter -= 2
                    if opts.debug:
                        print(letter, f"{new_pos} added", area, perimeter)
                    new_search.append(new_pos)
        if len(new_search) == 0:
            break
        search = new_search
    return found, area, perimeter


def find_regions(opts, grid, max_row, max_col):
    regions = []
    seen = {}
    for row in range(max_row):
        for col in range(max_col):
            pos = Vector(row, col)
            if str(pos) in seen:
                continue
            letter = grid[row][col]
            region, area, perimeter = map_region(opts, grid, max_row, max_col, pos, letter)
            for key in region.keys():
                seen[key] = True
            regions.append([ letter, list(region.values()), area, perimeter ])
    return regions


def run_part1(opts, grid, max_row, max_col):
    if opts.debug:
        print(opts)
        print(lines)
    regions = find_regions(opts, grid, max_row, max_col)

    cost = 0
    for letter, positions, area, perimeter in regions:
        cost += area * perimeter
    return cost


def run_part2(opts, grid, max_row, max_col):
    if opts.debug:
        print(opts)
        print(lines)
    regions = find_regions(opts, grid, max_row, max_col)

    cost = 0
    for letter, positions, area, perimeter in regions:

        # Post-process the positions that we found, to figure out the
        # sides
        sides = 0

        # Sort by row, then column
        positions.sort(key=lambda x: x[0] * max_row + x[1])

        # Check for sides up & down
        for row_dir in [-1, 1]:
            last_row = None
            last_col = None
            for row, col in positions:
                new_row = row + row_dir
                new_col = col
                if (on_grid(opts, max_row, max_col, Vector(new_row, new_col)) and
                    grid[new_row][new_col] == letter):
                    # Bordering another of the same letter, ignore
                    continue
                # We're on _a_ side
                if last_row is None and last_col is None:
                    # Fresh start
                    last_row = row
                    last_col = col
                    sides += 1
                elif last_row == row:
                    if last_col < col - 1:
                        # Not continguous, new side
                        last_row = row
                        last_col = col
                        sides += 1
                    else:
                        last_row = row
                        last_col = col
                else:
                    # New row, new side
                    last_row = row
                    last_col = col
                    sides += 1

        # Sort by row, then column
        positions.sort(key=lambda x: x[1] * max_col + x[0])

        # Check for sides left & right
        for col_dir in [-1, 1]:
            last_row = None
            last_col = None
            for row, col in positions:
                new_row = row
                new_col = col + col_dir
                if (on_grid(opts, max_row, max_col, Vector(new_row, new_col)) and
                    grid[new_row][new_col] == letter):
                    # Bordering another of the same letter, ignore
                    continue
                # We're on _a_ side
                if last_row is None and last_col is None:
                    # Fresh start
                    last_row = row
                    last_col = col
                    sides += 1
                elif last_col == col:
                    if last_row < row - 1:
                        # Not continguous, new side
                        last_row = row
                        last_col = col
                        sides += 1
                    else:
                        last_row = row
                        last_col = col
                else:
                    # New col, new side
                    last_row = row
                    last_col = col
                    sides += 1

        cost += area * sides
    return cost


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()
    grid = []
    max_row = 0
    max_col = 0
    try:
        with open(opts.filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                max_row += 1
                if not max_col:
                    max_col = len(line)
                grid.append(list(line))
    except FileNotFoundError:
        print(f"ERROR: File not found {opts.filename}")
        sys.exit(1)
    # Done reading

    # Function to call, expected values, for sanity-checking (sample)
    # and later factoring (both):
    run = {
        1: { "call": run_part1,
             "sample1":    140,
             "sample2":    772,
             "sample3":   1930,
             "input":  1477924 },
        2: { "call": run_part2,
             "sample1":     80,
             "sample2":    436,
             "sample3":   1206,
             "sample4":    236,
             "sample5":    368,
             "input":   841934 }
    }

    answer = run[opts.part]["call"](opts, grid, max_row, max_col)
    if opts.filename in run[opts.part] and answer != run[opts.part][opts.filename]:
        print(f"Warning: Unexpected value from part {opts.part}, filename '{opts.filename}'")
    print(f"Answer: {answer}")


if __name__ == "__main__":
    sys.exit(main())
