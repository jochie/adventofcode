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
        description='This program is used for one of the AoC 2024 puzzles; Day 11'
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
    parser.add_argument('-b', '--blinks',
                        help="Number of blinks",
                        type=int,
                        default=1)
    return parser.parse_args()


def run_part1(opts, lines):
    if opts.debug:
        print(opts)
        print(lines)

    stones = [ int(x) for x in lines[0].split(" ") ]
    if opts.debug:
        print(stones)
    for blink in range(opts.blinks):
        new_stones = []
        for stone in stones:
            if stone == 0:
                new_stones.append(1)
            else:
                stone_str = str(stone)
                stone_str_len = len(stone_str)
                if stone_str_len % 2 == 0:
                    new_stones.append(int(stone_str[:int(stone_str_len/2)]))
                    new_stones.append(int(stone_str[int(stone_str_len/2):]))
                else:
                    new_stones.append(stone * 2024)
        if opts.debug:
            print(new_stones)
        stones = new_stones
        if opts.verbose:
            print(f"Blinked {blink}: {len(stones)}")
    return len(stones)


# The key part was realizing that this sentence can be ignored
# entirely and the order doesn't actually matter:
#
# "No matter how the stones change, their order is preserved, and they
#  stay on their perfectly straight line."
#
# So for the part 2 rewrite, I'm tracking however many of rock/number
# there is, and do the same iterations.
#
# For the 75 blinks/iterations and with my specific input there were
# 3782 unique rocks/numbers.
def run_part2(opts, lines):
    if opts.debug:
        print(opts)
        print(lines)

    stones = { int(x): 1 for x in lines[0].split(" ") }

    if opts.debug:
        print(stones)
    for blink in range(opts.blinks):
        new_stones = {}
        for stone, count in stones.items():
            if stone == 0:
                if 1 not in new_stones:
                    new_stones[1] = 0
                new_stones[1] += count
            else:
                stone_str = str(stone)
                stone_str_len = len(stone_str)
                if stone_str_len % 2 == 0:
                    l_stone = int(stone_str[:int(stone_str_len/2)])
                    r_stone = int(stone_str[int(stone_str_len/2):])
                    if l_stone not in new_stones:
                        new_stones[l_stone] = 0
                    new_stones[l_stone] += count
                    if r_stone not in new_stones:
                        new_stones[r_stone] = 0
                    new_stones[r_stone] += count
                else:
                    stone *= 2024
                    if stone not in new_stones:
                        new_stones[stone] = 0
                    new_stones[stone] += count
        if opts.debug:
            print(new_stones)
        stones = new_stones
        if opts.verbose:
            print(f"Blinked {blink}: {len(stones)}")
    total = 0
    for stone, count in stones.items():
        total += count
    return total


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()
    lines = []
    try:
        with open(opts.filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                lines.append(line)
    except FileNotFoundError:
        print(f"ERROR: File not found {opts.filename}")
        sys.exit(1)
    # Done reading

    # Function to call, expected values, for sanity-checking (sample)
    # and later refactoring (both):
    run = {
        1: { "call":       run_part1,
             "sample-1":   7,
             "sample2-25": 55312,
             "input-25":   184927
            },
        2: { "call":       run_part2,
             "sample-1":   7,
             "sample2-25": 55312,
             "input-25":   184927,
             "input-75":   220357186726677
            }
    }

    answer = run[opts.part]["call"](opts, lines)
    answer_key = f"{opts.filename}-{opts.blinks}"
    if answer_key in run[opts.part] and answer != run[opts.part][answer_key]:
        print(f"Warning: Unexpected value from part {opts.part}, filename '{opts.filename}, blinks '{opts.blinks}'")
    print(f"Answer: {answer}")


if __name__ == "__main__":
    sys.exit(main())
