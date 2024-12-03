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
        description='This program is used for one of the AoC 2024 puzzles; Day 3, part 2'
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
    total = 0
    instr_on = True
    for line in sys.stdin:
        line = line.rstrip()
        if opts.debug:
            print("DEBUG: Line received: '{}'".format(line))
        while True:
            instr_match = re.match(r"(.*?)(do\(\)|don't\(\)|mul\(\d+,\d+\))(.*)", line)
            if not instr_match:
                break
            if instr_match.group(2) == "do()":
                instr_on = True
            elif instr_match.group(2) == "don't()":
                instr_on = False
            else:
                mul_match = re.match(r"mul\((\d+),(\d+)\)", instr_match.group(2))
                if instr_on:
                    total += (int(mul_match.group(1)) * int(mul_match.group(2)))
            line = instr_match.group(3)
    # Done
    print(f"Sum of the (active) multiplications: {total}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
