#!/usr/bin/env python3

import re
import sys

M_WALL  = -1
M_RIGHT = 1
M_DOWN  = 2
M_LEFT  = 4
M_UP    = 8
M_EMPTY = 0

DEBUG   = False
VERBOSE = False

map = []
width = 0
height = 0

def to_key(row, col):
    return f"{row},{col}"

def from_key(key):
    m = re.match(r"(\d+),(\d+)$", key)
    return int(m.group(1)), int(m.group(2))

def parse_map():
    global map, width, height

    map = {}
    row = 0
    for line in sys.stdin:
        line = line.rstrip()
        width = len(line)
        for col in range(len(line)):
            if line[col] == "#":
                map[to_key(row, col)] = M_WALL
            elif line[col] == ">":
                map[to_key(row, col)] = M_RIGHT
            elif line[col] == "v":
                map[to_key(row, col)] = M_DOWN
            elif line[col] == "<":
                map[to_key(row, col)] = M_LEFT
            elif line[col] == "^":
                map[to_key(row, col)] = M_UP
        row += 1
    height = row
    return

def dump_map(positions):
    global map, width, height
    print(f"MAP ({height} x {width}):")
    for row in range(height):
        output = ""
        for col in range(width):
            if to_key(row, col) in map:
                val = map[to_key(row, col)]
                if val == M_WALL:
                    output += "#"
                elif val > 0:
                    if DEBUG:
                        output += str(val)
                    else:
                        b = []
                        if val & M_RIGHT:
                            b.append(">")
                        if val & M_DOWN:
                            b.append("v")
                        if val & M_LEFT:
                            b.append("<")
                        if val & M_UP:
                            b.append("^")
                        if len(b) == 1:
                            output += b[0]
                        else:
                            output += str(len(b))
            elif to_key(row, col) in positions:
                output += "E"
            else:
                output += "."
        print(output)
    print()
    return

def iterate_map():
    global map, width, height

    new_map = {}
    for row in range(height):
        for col in range(width):
            if to_key(row, col) in map:
                val = map[to_key(row, col)]
                if val == M_WALL:
                    new_map[to_key(row, col)] = M_WALL
                else:
                    if val & M_RIGHT:
                        if col == width - 2:
                            new_col = 1
                        else:
                            new_col = col + 1
                        if to_key(row, new_col) in new_map:
                            new_map[to_key(row, new_col)] |= M_RIGHT
                        else:
                            new_map[to_key(row, new_col)] = M_RIGHT
                    if val & M_DOWN:
                        if row == height - 2:
                            new_row = 1
                        else:
                            new_row = row + 1
                        if to_key(new_row, col) in new_map:
                            new_map[to_key(new_row, col)] |= M_DOWN
                        else:
                            new_map[to_key(new_row, col)] = M_DOWN
                    if val & M_LEFT:
                        if col == 1:
                            new_col = width -2
                        else:
                            new_col = col - 1
                        if to_key(row, new_col) in new_map:
                            new_map[to_key(row, new_col)] |= M_LEFT
                        else:
                            new_map[to_key(row, new_col)] = M_LEFT
                    if val & M_UP:
                        if row == 1:
                            new_row = height - 2
                        else:
                            new_row = row - 1
                        if to_key(new_row, col) in new_map:
                            new_map[to_key(new_row, col)] |= M_UP
                        else:
                            new_map[to_key(new_row, col)] = M_UP
    map = new_map
    return

def possible_steps(row, col):
    global map

    options = [ ]
    if row == 0:
        # We're at the starting position (still, again)
        if to_key(row + 1, col) in map:
            # Something is blocking us, so only option is to stay put
            return [ [ row, col ] ]
        return [ [ row + 1, col ] ]
    if row == height - 1:
        # We're at the end position (still, again)
        if to_key(row - 1, col) in map:
            # Something is blocking us, so only option is to stay put
            return [ [ row, col ] ]
        return [ [ row - 1, col ] ]
    if row == height - 2 and col == width -2:
        options.append([ row + 1, col ])
    if row == 1 and col == 1:
        options.append([ 0, 1 ])
    if to_key(row, col) not in map:
        # Stay put, if that's possible
        options.append([ row, col ])
    if row > 1 and to_key(row - 1, col) not in map:
        options.append([ row - 1, col ])
    if row < height - 2 and to_key(row + 1, col) not in map:
        options.append([ row + 1, col ])
    if col > 1 and to_key(row, col - 1) not in map:
        options.append([ row, col - 1 ])
    if col < width - 2 and to_key(row, col + 1) not in map:
        options.append([ row, col + 1 ])
    return options

def main():
    parse_map()
    positions = {
        to_key(0, 1): True
    }
    minute = 0
    dump_map(positions)

    targets = [ [ height - 1, width - 2 ], [ 0, 1 ], [ height - 1, width - 2 ] ]
    while True:
        new_positions = {}
        minute += 1
        print(f"Minute {minute}:")
        iterate_map()
        for pos in positions.keys():
            row, col = from_key(pos)
            if DEBUG:
                print(f"Checking position {row},{col}")
            steps = possible_steps(row, col)
            if DEBUG:
                print(f"Steps: {steps}")
            if len(steps) == 0:
                if DEBUG:
                    print("  Eliminated, because one of the blizzards hit us.")
            else:
                for step in steps:
                    new_positions[to_key(step[0], step[1])] = True
        positions = new_positions
        target = targets[0]
        if to_key(target[0], target[1]) in positions:
            print(f"We've reached {target} in {minute} minutes.")
            targets = targets[1:]
            if len(targets) == 0:
                print(f"That was the end!")
                sys.exit(1)
            positions = {
                to_key(target[0], target[1]): True
            }
        if len(positions) == 0:
            print(f"We've run out of options?")
            sys.exit(1)
        if VERBOSE:
            dump_map(positions)
    return

main()
