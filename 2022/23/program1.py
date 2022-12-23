#!/usr/bin/env python3

import re
import sys
import time

positions = {}
check_list = []

def to_key(row, col):
    return f"{row},{col}"

def from_key(key):
    m = re.match(r"(-?\d+),(-?\d+)$", key)
    return int(m.group(1)), int(m.group(2))

def read_map():
    row = 0
    for line in sys.stdin:
        line = line.rstrip()
        for col in range(len(line)):
            if line[col] == '#':
                positions[to_key(row, col)] = True
        row += 1
    pass


def dump_map():
    global positions

    (row_min, row_max, col_min, col_max) = (None, None, None, None)

    for pos in positions.keys():
        row, col = from_key(pos)
        if row_min is None or row < row_min:
            row_min = row
        if row_max is None or row > row_max:
            row_max = row
        if col_min is None or col < col_min:
            col_min = col
        if col_max is None or col > col_max:
            col_max = col

    empty = 0
    for row in range(row_min, row_max + 1):
        line = "| "
        for col in range(col_min, col_max + 1):
            if to_key(row, col) in positions:
                line += "#"
            else:
                line += "."
                empty += 1
        line += " |"
        print(line)

    print(f"Empty tiles in this grid: {empty}")
    return

def check_north(row, col):
    adj = 0
    for c in range(col - 1, col + 2):
        if to_key(row - 1, c) in positions:
            adj += 1
    if adj == 0:
        return "N"
    return None

def check_south(row, col):
    adj = 0
    for c in range(col - 1, col + 2):
        if to_key(row + 1, c) in positions:
            adj += 1
    if adj == 0:
        return "S"
    return None

def check_west(row, col):
    adj = 0
    for r in range(row - 1, row + 2):
        if to_key(r, col - 1) in positions:
            adj += 1
    if adj == 0:
        return "W"
    return None

def check_east(row, col):
    adj = 0
    for r in range(row - 1, row + 2):
        if to_key(r, col + 1) in positions:
            adj += 1
    if adj == 0:
        return "E"
    return None

def get_direction(row, col):
    adj = -1
    for r in range(row - 1, row + 2):
        for c in range(col - 1, col + 2):
            if to_key(r, c) in positions:
                adj += 1
    if adj == 0:
        return "X"

    for func in check_list:
        dir = func(row, col)
        if dir is not None:
            return dir
    # Nowhere to move, after all?
    return "X"

def iterate_map():
    global positions

    movements = 0
    old_positions = {}
    new_positions = {}
    for pos in positions.keys():
        row, col = from_key(pos)
        dir = get_direction(row, col)
        if dir == "X":
            old_positions[pos] = True
        elif dir == "N":
            new_key = to_key(row - 1, col)
            if new_key not in new_positions:
                new_positions[new_key] = []
            new_positions[new_key].append(pos)
        elif dir == "E":
            new_key = to_key(row, col + 1)
            if new_key not in new_positions:
                new_positions[new_key] = []
            new_positions[new_key].append(pos)
        elif dir == "S":
            new_key = to_key(row + 1, col)
            if new_key not in new_positions:
                new_positions[new_key] = []
            new_positions[new_key].append(pos)
        elif dir == "W":
            new_key = to_key(row, col - 1)
            if new_key not in new_positions:
                new_positions[new_key] = []
            new_positions[new_key].append(pos)
        else:
            print("Should not happen")
            sys.exit(1)
    positions = old_positions
    for pos in new_positions.keys():
        val = new_positions[pos]
        if len(val) == 1:
            positions[pos] = True
            movements += 1
        else:
            for old_pos in val:
                positions[old_pos] = True

    return movements

def main():
    global check_list

    read_map()
    dump_map()
    check_list = [ check_north, check_south, check_west, check_east ]
    for round in range(10):
        print()
        print(f"ROUND {round+1}")
        iterate_map()
        dump_map()
        check_list = check_list[1:] + check_list[:1]

main()
