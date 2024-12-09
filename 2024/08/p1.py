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
        description='This program is used for one of the AoC 2024 puzzles; Day 8, part 1'
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


# Very basic vector / grid-position class, just enough to solve this
# puzzle.
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


def on_grid(pos, max_row, max_col):
    return pos.row >= 0 and pos.row < max_row and pos.col >= 0 and pos.col < max_col


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()
    antennas = {}
    grid = []
    max_row = 0
    max_col = 0
    for line in sys.stdin:
        line = line.rstrip()

        if not max_col:
            max_col = len(line)

        grid.append(list(line))
        for col, char in enumerate(line):
            if char == '.':
                continue
            if char not in antennas:
                antennas[char] = []
            antennas[char].append(Vector(max_row, col))
        max_row += 1

        if opts.debug:
            print(f"DEBUG: Line received: '{line}'")
    # Done reading

    antinodes = {}
    seen = {}
    for freq, positions in antennas.items():
        for pos1 in positions:
            for pos2 in positions:
                if pos1 == pos2:
                    # We're looking in the mirror
                    continue
                if f"{pos2},{pos1}" in seen:
                    # We already processed this pair from the opposite direction
                    continue
                seen[f"{pos1},{pos2}"] = True

                pos_diff = pos2 - pos1
                anti1 = pos1 - pos_diff
                if on_grid(anti1, max_row, max_col):
                    antinodes[str(anti1)] = True

                anti2 = pos2 + pos_diff
                if on_grid(anti2, max_row, max_col):
                    antinodes[str(anti2)] = True

    # Result:
    print(f"Unique antinodes found: {len(antinodes)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
