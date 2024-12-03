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
        description='This program is used for one of the AoC 2024 puzzles; Day 2, part 2'
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

def safe_levels(opts, levels):
    incr = 0
    decr = 0
    level = int(levels[0])
    for next_level in levels[1:]:
        next_level = int(next_level)
        if opts.debug:
            print(f"Comparing {level} to {next_level}")
        if next_level > level:
            incr += 1
        elif next_level < level:
            decr += 1
        else:
            if opts.debug:
                print("No change")
            return False
        diff = abs(next_level - level)
        if diff > 3:
            return False
        level = next_level
    if incr > 0 and decr > 0:
        if opts.debug:
            print("Both incr and decr")
        return False
    return True


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    safe_total = 0
    opts = parse_options()
    for line in sys.stdin:
        line = line.rstrip()
        levels = line.split(" ")
        if opts.debug:
            print(levels)
        if safe_levels(opts, levels):
            safe_total += 1
        else:
            # Brute force, didn't seem worth the effort to look for a
            # non-O(N^2) solution
            for ix in range(len(levels)):
                if safe_levels(opts, levels[:ix] + levels[ix + 1:]):
                    safe_total += 1
                    break
        if opts.debug:
            print("DEBUG: Line received: '{}'".format(line))
    # Done
    print(f"Reports with safe levels: {safe_total}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
