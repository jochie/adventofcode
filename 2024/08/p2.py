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
        description='This program is used for one of the AoC 2024 puzzles; Day 8, part 2'
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


def on_grid(row, col, max_row, max_col):
    return row >= 0 and row < max_row and col >= 0 and col < max_col


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    antennas = {}
    grid = []
    opts = parse_options()
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
            antennas[char].append([ max_row, col ])
        max_row += 1

        if opts.debug:
            print(f"DEBUG: Line received: '{line}'")
    # Done reading

    antinodes = {}
    seen = {}
    for freq, positions in antennas.items():
        for row1, col1 in positions:
            for row2, col2 in positions:
                if row1 == row2 and col1 == col2:
                    # We're looking in the mirror
                    continue
                if f"{row2},{col2},{row1},{col1}" in seen:
                    # We already processed this pair from the opposite direction
                    continue
                seen[f"{row1},{col1},{row2},{col2}"] = True

                row_diff = row2 - row1
                col_diff = col2 - col1

                anti1_row = row1
                anti1_col = col1
                while True:
                    antinodes[f"({anti1_row},{anti1_col})"] = True
                    anti1_row -= row_diff
                    anti1_col -= col_diff
                    if not on_grid(anti1_row, anti1_col, max_row, max_col):
                        break

                anti2_row = row2
                anti2_col = col2
                while True:
                    antinodes[f"({anti2_row},{anti2_col})"] = True
                    anti2_row += row_diff
                    anti2_col += col_diff
                    if not on_grid(anti2_row, anti2_col, max_row, max_col):
                        break

    # Result:
    print(f"Unique antinodes found: {len(antinodes)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
