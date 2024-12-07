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
        description='This program is used for one of the AoC 2024 puzzles; Day 7, part 2'
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


# Turning this into a generator shaved about 8-10 seconds off the time
# for the actual input.
def generate_expressions(opts, values):
    if len(values) == 1:
        yield values
        return
    for expr in generate_expressions(opts, values[1:]):
        yield [ values[0], '+'  ] + expr
        yield [ values[0], '*'  ] + expr
        yield [ values[0], '||' ] + expr


def evaluate_expression(opts, expr, target):
    result = expr[0]
    expr = expr[1:]
    while len(expr) > 0:
        if result > target:
            # Micro-optimization:
            #
            # We've already overshot our target, throwing more
            # operators and values won't get us any closer.
            #
            # This only shaves a second or two for the actual input.
            return -1
        if expr[0] == '+':
            result += expr[1]
        elif expr[0] == '*':
            result *= expr[1]
        else:
            # Must be the '||' operator
            result = int(str(result) + str(expr[1]))
        expr = expr[2:]
    return result


def find_operators(opts, result, values):
    for expr in generate_expressions(opts, values):
        expr_eval = evaluate_expression(opts, expr, result)
        if expr_eval == result:
            return True
    return False


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()
    total = 0
    for line in sys.stdin:
        line = line.rstrip()
        if opts.debug:
            print("DEBUG: Line received: '{}'".format(line))
        line_match = re.match(r"(\d+): (.*)", line)
        result = int(line_match.group(1))
        values = [int(x) for x in line_match.group(2).split(" ")]
        if find_operators(opts, result, values):
            total += result
    print(f"Total calibration result: {total}")

    # Done
    return 0

if __name__ == "__main__":
    sys.exit(main())
