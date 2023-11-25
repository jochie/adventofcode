#!/usr/bin/env python

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
        description='This program is used for one of the AoC YYYY puzzles; Day X, part Z'
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
    for line in sys.stdin:
        line = line.rstrip()
        if opts.debug:
            print("DEBUG: Line received: '{}'".format(line))
    # Done
    return 0

if __name__ == "__main__":
    sys.exit(main())
