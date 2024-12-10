#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
import copy
import re
import sys


def parse_options():
    """
    Parser command line options
    """
    parser = argparse.ArgumentParser(
        description='This program is used for one of the AoC 2024 puzzles; Day 10'
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


def on_grid(max_row, max_col, pos):
    return pos.row >= 0 and pos.col >= 0 and pos.row < max_row and pos.col < max_col


def reachable_nines(opts, grid, max_row, max_col, pos):
    seen = {
        str(pos): True
    }
    search = [[ pos, 0 ]]
    found = {}
    while True:
        if opts.debug:
            print(search)
        new_search = []
        for pos, val in search:
            for rel_pos in DIRS:
                new_pos = pos + rel_pos
                if not on_grid(max_row, max_col, new_pos):
                    continue
                if str(new_pos) in seen:
                    continue
                if grid[new_pos.row][new_pos.col] == '.':
                    continue
                new_val = int(grid[new_pos.row][new_pos.col])
                if new_val != val + 1:
                    continue
                if new_val == 9:
                    found[str(new_pos)] = True
                else:
                    new_search.append([ new_pos, new_val ])
                seen[str(new_pos)] = True
        if not len(new_search):
            break
        search = new_search
    return len(found)


def run_part1(opts, grid, max_row, max_col):
    if opts.debug:
        print(opts)
        print(max_row, max_col, grid)
    total = 0
    for row in range(max_row):
        for col in range(max_col):
            if grid[row][col] == '0':
                total += reachable_nines(opts, grid, max_row, max_col, Vector(row, col))
    return total


def distinct_trails(opts, grid, max_row, max_col, pos):
    search = [[ pos, 0, { str(pos): True } ]]
    found = 0
    while True:
        if opts.debug:
            print(search)
        new_search = []
        for pos, val, seen in search:
            for rel_pos in DIRS:
                new_pos = pos + rel_pos
                if not on_grid(max_row, max_col, new_pos):
                    continue
                if str(new_pos) in seen:
                    continue
                if grid[new_pos.row][new_pos.col] == '.':
                    continue
                new_val = int(grid[new_pos.row][new_pos.col])
                if new_val != val + 1:
                    continue
                if new_val == 9:
                    found += 1
                else:
                    new_seen = copy.deepcopy(seen)
                    new_seen[str(new_pos)] = True

                    new_search.append([ new_pos, new_val, new_seen ])
        if not len(new_search):
            break
        search = new_search
    return found


def run_part2(opts, grid, max_row, max_col):
    if opts.debug:
        print(opts)
        print(max_row, max_col, grid)
    total = 0
    for row in range(max_row):
        for col in range(max_col):
            if grid[row][col] == '0':
                total += distinct_trails(opts, grid, max_row, max_col, Vector(row, col))
    return total


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()
    grid = []
    max_row = 0
    max_col = 0
    with open(opts.filename, "r", encoding="utf-8") as input_fd:
        for line in input_fd:
            line = line.rstrip()
            if opts.debug:
                print(f"DEBUG: Line received: '{line}'")
            max_row += 1
            if not max_col:
                max_col = len(line)
            grid.append(list(line))
    # Done reading

    # Function to call, expected values, for sanity-checking (sample)
    # and later factoring (both):
    run = {
        1: { "call": run_part1,
             "sample1":   2,
             "sample2":   4,
             "sample3":   3,
             "sample4":  36,
             "input":   472
            },
        2: { "call": run_part2,
             "sample5":   3,
             "sample6":  13,
             "sample4":  81,
             "input":   969
            }
    }

    answer = run[opts.part]["call"](opts, grid, max_row, max_col)
    if answer != run[opts.part][opts.filename]:
        print(f"Warning: Unexpected value from part {opts.part}, filename '{opts.filename}'")
    print(f"Answer: {answer}")


if __name__ == "__main__":
    sys.exit(main())
