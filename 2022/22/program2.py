#!/usr/bin/env python3

import re
import sys

DEBUG = False

"""
             Z-axis
           (0,0,1)
               *   Y-axis
               |  *  (0,1,0)
               | /
               |/
(-1,0,0) *-----*-----* (1,0,0)  X-axis
              /|
             / |
            *  |
    (0,-1,0)   * (0,0,-1)

"""

map = []
directions = ""
width = 0
height = 0

# In this case the sample and the actual input are differently laid out,
# which is a bit frustrating

# Sample:
"""
cube_size = 4
cube_layout = [
    [ ' ', ' ', '1' ],
    [ '2', '3', '4' ],
    [ ' ', ' ', '5', '6' ]
]
"""

# Actual input:
cube_size = 50
cube_layout = [
    [ ' ', '1', '2' ],
    [ ' ', '3' ],
    [ '4', '5' ],
    [ '6' ]
]
side_offsets = {}
for grid_row in range(len(cube_layout)):
    for grid_col in range(len(cube_layout[grid_row])):
        side = cube_layout[grid_row][grid_col]
        if side != ' ':
            side_offsets[side] = [ grid_row * cube_size, grid_col * cube_size ]

def determine_side(row, col):
    grid_row = (row - 1) // cube_size
    grid_col = (col - 1) // cube_size
    rel_row = row - 1 - grid_row * cube_size
    rel_col = col - 1 - grid_col * cube_size
    if DEBUG:
        print(f"determine_side({row}, {col}) => {grid_row}, {grid_col} '{cube_layout[grid_row][grid_col]}', {rel_row}, {rel_col}")
    return cube_layout[grid_row][grid_col], rel_row, rel_col

def read_input():
    global width, height, directions

    for line in sys.stdin:
        line = line.rstrip()
        if line == "":
            break
        map.append(line)
        if len(line) > width:
            width = len(line)
    height = len(map)
    for row in range(height):
        while len(map[row]) < width:
            map[row] += " "
            
    for line in sys.stdin:
        directions = line.rstrip()
    return

def starting_col():
    global width

    for col in range(width):
        if map[0][col] == ".":
            return col + 1
    return -1

def move_to(side, row, col, fac):
    print(f"wrap_coord moves to '{side}': {side_offsets[side][0] + row}, {side_offsets[side][1] + col}, {fac}")
    return side_offsets[side][0] + row, side_offsets[side][1] + col, fac

# Given the coordinates, and the direction,
# return the wrapped coordinates, or -1 if
# you would hit a wall?
def wrap_coord(row, col, fac):
    global width, height

    print(f"wrap_coord({row}, {col}, {fac})")
    side, rel_row, rel_col = determine_side(row, col)

    # Travel from 1 to 6
    if side == "1" and fac == 3:
        # Hop from the top of "1" to the left of "6"
        return move_to('6', rel_col + 1, 1, 0)

    # Travel from 6 to 1
    if side == "6" and fac == 2:
        # Hop from the left of "6" to the top of "1"
        return move_to('1', 1, rel_row + 1, 1)

    # Travel from 1 to 4
    if side == "1" and fac == 2:
        # Hop from the left of "1" to the left of "4" (upsidedown)
        return move_to('4', cube_size - rel_row, 1, 0)

    # Travel from 5 to 6
    if side == "5" and fac == 1:
        # Hop from the bottom of "5" to the right of "6"
        return move_to('6', rel_col + 1, cube_size, 2)

    # Travel from 4 to 1
    if side == "4" and fac == 2:
        # Hop from the left of "4" to the left of "1" (upsidedown)
        return move_to('1', cube_size - rel_row, 1, 0)

    # Travel from 4 to 3
    if side == "4" and fac == 3:
        # Hop from the top of "4" to the left of "3"
        return move_to('3', rel_col + 1, 1, 0)

    # Travel from 3 to 4
    if side == "3" and fac == 2:
        # Hop from the left of "3" to the top of "4"
        return move_to('4', 1, rel_row + 1, 1)

    # Travel from 3 to 2
    if side == "3" and fac == 0:
        # Hop from the right of "3" to the bottom of "2"
        return move_to('2', cube_size, rel_row + 1, 3)

    # Travel from 2 to 3
    if side == "2" and fac == 1:
        # Hop from the bottom of "2" to the right of "3"
        return move_to('3', rel_col + 1, cube_size, 2)

    # Travel from 6 to 2
    if side == "6" and fac == 1:
        # Hop from the bottom of "6" to the top of "2"
        return move_to('2', 1, rel_col + 1, 1)

    # Travel from 2 to 6
    if side == "2" and fac == 3:
        # Hop from the top of "2" to the bottom of "6"
        return move_to('6', cube_size, rel_col + 1, 3)

    # Travel from 2 to 5
    if side == "2" and fac == 0:
        # Hop from the right of "2" to the right of "5" (upsidedown)
        return move_to('5', cube_size - rel_row, cube_size, 2)

    # Travel from 5 to 2
    if side == "5" and fac == 0:
        # Hop from the right of "5" to the right of "2" (upsidedown)
        return move_to('2', cube_size - rel_row, cube_size, 2)

    # Travel from 6 to 5
    if side == "6" and fac == 0:
        # Hop from the right of "6" to the bottom of "5"
        return move_to('5', cube_size, rel_row + 1, 3)

    print(f"Haven't figured out an answer for this one yet: '{side}', {rel_row}, {rel_col}")
    sys.exit(1)

def dump_map():
    print(f"MAP:")
    for line in map:
        print(f"| {line} |")
    return

def dump_partial_map(row, col):
    (side, rel_row, rel_col) = determine_side(row, col)
    row_off = side_offsets[side][0]
    col_off = side_offsets[side][1]
    print(f"MAP for {side}:")
    for rel_row in range(cube_size):
        line = map[rel_row + row_off]
        print(f": {line[col_off:col_off+cube_size]} :")
    return

def main():
    global directions

    rel_coord = [
        [  0,  1, '>' ], # right
        [  1,  0, 'v' ], # down
        [  0, -1, '<' ], # left
        [ -1,  0, '^' ]  # up
    ]
    read_input()
    fac = 0 # facing right
    row = 1
    col = starting_col()
    print(f"Width: {width}; Height: {height}; starting column = {col}; facing {fac}")
    map[row-1] = map[row-1][:col-1] + rel_coord[fac][2] + map[row-1][col:]
    print(f"Directions: {directions}")
    while len(directions) > 0:
        print(f"Position ({row},{col}) - Facing {fac}")
        m = re.match("^(\d+)(.*)$", directions)
        if m:
            directions = m.group(2)

            print(f"Take {m.group(1)} steps")
            steps = int(m.group(1))

            for step in range(steps):
                next_row = row + rel_coord[fac][0]
                next_col = col + rel_coord[fac][1]
                if 1 <= next_col and next_col <= width and 1 <= next_row and next_row <= height:
                    print(f"Next row,col: {next_row},{next_col}")
                    map_bit = map[next_row-1][next_col-1]
                    if map_bit == "#":
                        break
                    if map_bit == " ":
                        if DEBUG:
                            dump_partial_map(row, col)
                        (next_row, next_col, next_fac) = wrap_coord(row, col, fac)
                        if map[next_row-1][next_col-1] == "#":
                            print("We'd hit a wall, stay put.")
                            break
                        (row, col, fac) = (next_row, next_col, next_fac)
                        map[row-1] = map[row-1][:col-1] + rel_coord[fac][2] + map[row-1][col:]
                        if DEBUG:
                            dump_partial_map(row, col)
                    else:
                        (row, col) = (next_row, next_col)
                        map[row-1] = map[row-1][:col-1] + rel_coord[fac][2] + map[row-1][col:]
                else:
                    if DEBUG:
                        dump_partial_map(row, col)
                    (next_row, next_col, next_fac) = wrap_coord(row, col, fac)
                    if map[next_row-1][next_col-1] == "#":
                        print("We'd hit a wall, stay put.")
                        break
                    (row, col, fac) = (next_row, next_col, next_fac)
                    map[row-1] = map[row-1][:col-1] + rel_coord[fac][2] + map[row-1][col:]
                    if DEBUG:
                        dump_partial_map(row, col)
        else:
            m = re.match("^([LR])(.*)$", directions)
            if m:
                directions = m.group(2)

                print(f"Turn {m.group(1)}")
                turn = m.group(1)
                if turn == "L":
                    fac = (fac - 1) % 4
                else:
                    fac = (fac + 1) % 4
                map[row-1] = map[row-1][:col-1] + rel_coord[fac][2] + map[row-1][col:]
            else:
                print("What? {directions} did not parse into something useful.")
                sys.exit(1)

    print(f"Final position: row {row}, col {col}, facing {fac}, making the final password {row * 1000 + col * 4 + fac}")

main();
