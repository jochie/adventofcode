#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
import functools
import sys

def parse_options():
    """
    Parser command line options
    """
    parser = argparse.ArgumentParser(
        description='This program is used for one of the AoC 2024 puzzles; Day 5, part 2'
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
    ordering = {}

    for line in sys.stdin:
        line = line.rstrip()
        if len(line) == 0:
            break
        if opts.debug:
            print("DEBUG: Line received: '{}'".format(line))
        ordering[line] = True

    middle_total = 0
    for line in sys.stdin:
        line = line.rstrip()
        if opts.debug:
            print("DEBUG: Line received: '{}'".format(line))
        pagelist = line.split(",")

        def compare_numbers(a, b):
            if f"{a}|{b}" in ordering:
                return -1
            return 1

        sorted_list = sorted(pagelist, key=functools.cmp_to_key(compare_numbers))
        if line != ",".join(sorted_list):
            middle_total += int(sorted_list[int((len(pagelist)-1)/2)])

    print(f"Combined middle values (after sorting) of the unsorted lists: {middle_total}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
