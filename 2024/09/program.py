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
        description='This program is used for the AoC 2024 puzzles on Day 9'
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
    return parser.parse_args()


# Generic functions


def dump_blocks(blocks):
    represent = [f"<{x}>" if x != -1 else '.' for x in blocks]
    print("".join(represent))


def calculate_checksum(opts, blocks):
    csum = 0
    for ix, val in enumerate(blocks):
        if val != -1:
            csum += ix * val
    return csum


# Part 1 functions


def expand_line(opts, line):
    blocks = []
    block_id = 0
    for ix in range(len(line)):
        if ix % 2 == 0:
            for jx in range(int(line[ix])):
                blocks.append(block_id)
            block_id += 1
        else:
            for jx in range(int(line[ix])):
                blocks.append(-1)
    return blocks


def defrag_blocks(opts, blocks):
    # find all the empty offsets
    empties = []
    for ix, val in enumerate(blocks):
        if val == -1:
            empties.append(ix)
    iy = len(blocks) - 1
    while True:
        blocks[empties[0]] = blocks[iy]
        blocks[iy] = -1
        while True:
            iy -= 1
            if blocks[iy] != -1:
                break
        empties = empties[1:]
        if not len(empties) or iy <= empties[0]:
            break


def run_part1(opts, lines):
    line = lines[0]
    blocks = expand_line(opts, line)
    defrag_blocks(opts, blocks)
    if opts.debug:
        dump_blocks(blocks)
    return calculate_checksum(opts, blocks)


# Part 2 functions


def expand_line2(opts, line):
    blocks = []
    block_id = 0
    for ix in range(len(line)):
        if ix % 2 == 0:
            blocks.append([ block_id, int(line[ix]) ])
            block_id += 1
        else:
            if int(line[ix]) > 0:
                blocks.append([ -1, int(line[ix])])
    return blocks


def find_free_slot(blocks, width):
    for ix in range(len(blocks)):
        if blocks[ix][0] >= 0:
            continue
        if blocks[ix][1] >= width:
            return ix, blocks[ix][1]
    return -1, 0


def defrag_blocks2(opts, blocks):
    ix = len(blocks) - 1
    while ix >= 0:
        if blocks[ix][0] == -1:
            # Ignore blocks of free space, they don't need to be moved.
            ix -= 1
            continue
        block = blocks[ix]
        slot, slot_width = find_free_slot(blocks, block[1])
        if slot >= 0:
            if slot > ix:
                # To the right of the block we're trying to move
                ix -= 1
                continue
            if slot_width == block[1]:
                blocks[slot][0] = block[0]
                block[0] = -1
                ix -= 1
            else:
                extra = slot_width - block[1]
                blocks[slot][0] = block[0]
                blocks[slot][1] = block[1]
                block[0] = -1
                # Insert a bit of empty space for the rest
                blocks = blocks[:slot + 1] + [ [ -1, extra ] ] + blocks[slot + 1:]
                # Keep ix at the same spot, we inserted a new block to its left
        else:
            # This one won't move, then.
            ix -= 1
    return blocks


def normalize_blocks(opts, blocks):
    expanded = []
    for block in blocks:
        for ix in range(block[1]):
            expanded.append(block[0])
    return expanded


def run_part2(opts, lines):
    line = lines[0]
    blocks = expand_line2(opts, line)
    blocks = defrag_blocks2(opts, blocks)
    expanded = normalize_blocks(opts, blocks)
    if opts.debug:
        dump_blocks(expanded)
    return calculate_checksum(opts, expanded)


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()
    lines = []
    with open(opts.filename, "r", encoding="utf-8") as input_fd:
        for line in input_fd:
            line = line.rstrip()
            if opts.debug:
                print(f"DEBUG: Line received: '{line}'")
            lines.append(line)
    # Done reading

    # Expected values, for sanity-checking (sample) and later
    # factoring (both):
    expect = {
        1: { "sample": 1928, "input":  6607511583593 },
        2: { "sample": 2858, "input":  6636608781232 }
    }

    if opts.part == 1:
        answer = run_part1(opts, lines)
    else:
        answer = run_part2(opts, lines)
    if answer != expect[opts.part][opts.filename]:
        print(f"Warning: Unexpected value from part {opts.part}, filename '{opts.filename}'")
    print(f"Answer: {answer}")


if __name__ == "__main__":
    sys.exit(main())
