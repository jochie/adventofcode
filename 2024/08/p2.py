#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
import re
import sys

# For reusable AoC modules
sys.path.append("../../modules")

from vector import Vector

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
                anti1 = pos1
                while True:
                    antinodes[str(anti1)] = True
                    anti1 -= pos_diff
                    if not on_grid(anti1, max_row, max_col):
                        break

                anti2 = pos2
                while True:
                    antinodes[str(anti2)] = True
                    anti2 += pos_diff
                    if not on_grid(anti2, max_row, max_col):
                        break

    # Result:
    print(f"Unique antinodes found: {len(antinodes)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
