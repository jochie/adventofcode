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
        description='This program is used for one of the AoC 2024 puzzles; Day 1, part 1'
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


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()
    left = []
    right = []
    ix = 0
    for line in sys.stdin:
        line = line.rstrip()
        line_match = re.match(r"(\d+)\s+(\d+)$", line)
        left.append(int(line_match.group(1)))
        right.append(int(line_match.group(2)))
        ix += 1
        if opts.debug:
            print("DEBUG: Line received: '{}'".format(line))
    # Done

    left.sort(key=lambda x: x)
    right.sort(key=lambda x: x)
    distances = 0
    for ix in range(len(left)):
        distances += abs(left[ix] - right[ix])

    print(distances)
    return 0



if __name__ == "__main__":
    sys.exit(main())
