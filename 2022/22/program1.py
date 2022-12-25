#!/usr/bin/env python3

import re
import sys

map = []
directions = ""
width = 0
height = 0

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

# Given the coordinates, and the direction,
# return the wrapped coordinates, or -1 if
# you would hit a wall?
def wrap_coord(row, col, fac):
    global width, height

    print(f"wrap_coord({row}, {col}, {fac})")
    if fac == 0:
        # Facing right
        for col in range(width):
            if map[row-1][col] == "#":
                return -1, -1
            if map[row-1][col] != ' ':
                return row, col+1
        # We wrapped all the way around without finding anything?
        sys.exit(1) 
    if fac == 2:
        # Facing left
        for col in range(width):
            if map[row-1][width-1-col] == "#":
                return -1, -1
            if map[row-1][width-1-col] != ' ':
                return row, width-1-col+1
        # We wrapped all the way around without finding anything?
        sys.exit(1) 
    if fac == 1:
        print("ERWIN 1")
        # Facing down
        for row in range(height):
            if map[row][col-1] == "#":
                print("ERWIN 2")
                return -1, -1
            if map[row][col-1] != ' ':
                print("ERWIN 3")
                return row+1, col
        # We wrapped all the way around without finding anything?
        print("ERWIN 4")
        sys.exit(1) 
    if fac == 3:
        # Facing down
        for row in range(height):
            if map[height-1-row][col-1] == "#":
                return -1, -1
            if map[height-1-row][col-1] != ' ':
                return height-1-row+1, col
        # We wrapped all the way around without finding anything?
        sys.exit(1) 

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
        """
        print(f"MAP:")
        for line in map:
            print(f"| {line} |")
        """
        print(f"Position ({row},{col}) - Facing {fac}")
        m = re.match("^(\d+)(.*)$", directions)
        if m:
            directions = m.group(2)

            print(f"Take {m.group(1)} steps")
            steps = int(m.group(1))

            for step in range(steps):
                next_row = row + rel_coord[fac][0]
                next_col = col + rel_coord[fac][1]
                if 1 <= next_col <= width and 1 <= next_row <= height:
                    print(f"Next row,col: {next_row},{next_col}")
                    map_bit = map[next_row-1][next_col-1]
                    if map_bit == "#":
                        break
                    if map_bit != " ":
                        (row, col) = (next_row, next_col)
                        map[row-1] = map[row-1][:col-1] + rel_coord[fac][2] + map[row-1][col:]
                        continue
                    (next_row, next_col) = wrap_coord(next_row, next_col, fac)
                    if next_row == -1:
                        print("We'd hit a wall, stay put.")
                        break
                    (row, col) = (next_row, next_col)
                    map[row-1] = map[row-1][:col-1] + rel_coord[fac][2] + map[row-1][col:]
                    continue
                else:
                    (next_row, next_col) = wrap_coord(next_row, next_col, fac)
                    if next_row == -1:
                        print("We'd hit a wall, stay put.")
                        break
                    (row, col) = (next_row, next_col)
                    map[row-1] = map[row-1][:col-1] + rel_coord[fac][2] + map[row-1][col:]
                    continue
            continue
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
            continue
        print("What? {directions} did not parse into something useful.")
        sys.exit(1)

    print(f"Final position: row {row}, col {col}, facing {fac}, making the final password {row * 1000 + col * 4 + fac}")

main();
