#!/usr/bin/env python3

import sys
import json
import re

min_x = 0
min_y = 0
max_x = 0
max_y = 0

grid = []
x = 0 # left to right
y = 0 # up and down

# Round 1, reading the directions and determining the size of the grid
data = []
for line in sys.stdin:
    line = line.rstrip()
    m = re.match(r"^([RLUD]) (\d+)$", line)
    if m:
        print(f"Going {m.group(2)} steps {m.group(1)}")
        d = m.group(1)
        n = int(m.group(2))
        data.append([ d, n ])
        if   d == "L":
            x = x - n
            if x < min_x:
                min_x = x
        elif d == "R":
            x = x + n
            if x > max_x:
                max_x = x
        elif d == "U":
            y = y + n
            if y > max_y:
                max_y = y
        elif d == "D":
            y = y - n
            if y < min_y:
                min_y = y
        else:
            pass
    else:
        print("Dafuq?")
        sys.exit(1)
    print(f"Now at ({x}, {y})")

print(f"x range: {min_x} - {max_x}")
print(f"y range: {min_y} - {max_y}")

x_width = max_x - min_x + 1
y_width = max_y - min_y + 1

# Clumsily creating the grid
for i in range(y_width):
    grid.append([])
    for j in range(x_width):
        grid[i].append([".", 0])

x_offset = -min_x
y_offset = -min_y

print(f"x width {x_width}; offset {x_offset}")
print(f"y width {y_width}; offset {y_offset}")

# Knots coordinates
knots = []
for i in range(10):
    knots.append([ 0, 0 ])

# Mark the starting point
grid_seen = 1
grid[y_offset][x_offset][1] = 1

def knot_adjacent(i):
    if abs(knots[i - 1][0] - knots[i][0]) > 1:
        return False
    if abs(knots[i - 1][1] - knots[i][1]) > 1:
        return False
    return True

def adjust_knots():
    global grid_seen
    for i in range(9):
        if not knot_adjacent(i + 1):
            x_diff = knots[i][0] - knots[i + 1][0]
            y_diff = knots[i][1] - knots[i + 1][1]
            if x_diff == 2 and y_diff == 2:
                print(f"Snap {i+1} diagonal? ({x_diff},{y_diff})")
                knots[i + 1][0] = knots[i][0] - 1
                knots[i + 1][1] = knots[i][1] - 1
            elif x_diff == 2 and y_diff == -2:
                print(f"Snap {i+1} diagonal? ({x_diff},{y_diff})")
                knots[i + 1][0] = knots[i][0] - 1
                knots[i + 1][1] = knots[i][1] + 1
            elif x_diff == -2 and y_diff == 2:
                print(f"Snap {i+1} diagonal? ({x_diff},{y_diff})")
                knots[i + 1][0] = knots[i][0] + 1
                knots[i + 1][1] = knots[i][1] - 1
            elif x_diff == -2 and y_diff == -2:
                print(f"Snap {i+1} diagonal? ({x_diff},{y_diff})")
                knots[i + 1][0] = knots[i][0] + 1
                knots[i + 1][1] = knots[i][1] + 1
            elif x_diff == 2:
                print(f"Snap {i+1} to the right? ({x_diff},{y_diff})")
                knots[i + 1][0] = knots[i][0] - 1
                knots[i + 1][1] = knots[i][1]
            elif x_diff == -2:
                print(f"Snap {i+1} to the right? ({x_diff},{y_diff})")
                knots[i + 1][0] = knots[i][0] + 1
                knots[i + 1][1] = knots[i][1]
            elif y_diff == 2:
                print(f"Snap {i+1} up? ({x_diff},{y_diff})")
                knots[i + 1][0] = knots[i][0]
                knots[i + 1][1] = knots[i][1] - 1
            elif y_diff == -2:
                print(f"Snap {i+1} down? ({x_diff},{y_diff})")
                knots[i + 1][0] = knots[i][0]
                knots[i + 1][1] = knots[i][1] + 1
            if i == 8:
                if grid[knots[i + 1][1] + y_offset][knots[i + 1][0] + x_offset][1] == 0:
                    print("Mark tail positions")
                    grid_seen += 1
                    grid[knots[i + 1][1] + y_offset][knots[i + 1][0] + x_offset][1] = 1

def unmark_knots():
    for i in range(10):
        c = grid[knots[i][1] + y_offset][knots[i][0] + x_offset][0]
        grid[knots[i][1] + y_offset][knots[i][0] + x_offset][0] = "."

def mark_knots():
    grid[knots[9][1] + y_offset][knots[9][0] + x_offset][0] = "T"
    for i in reversed(range(8)):
        grid[knots[i + 1][1] + y_offset][knots[i + 1][0] + x_offset][0] = str(i + 1)
    grid[knots[0][1] + y_offset][knots[0][0] + x_offset][0] = "H"

    if False:
        # For debugging
        result = ""
        for i in reversed(range(y_width)):
            for j in range(x_width):
                result = result + (grid[i][j][0])
            result = result + "\n"
        print(result)
    # sys.exit(0)

# Mark the starting position
mark_knots()

# Round 2, now do the real thing
for info in data:
    (d, n) = info
    if d == "R":
        for i in range(n):
            unmark_knots()
            knots[0][0] += 1
            adjust_knots()
            mark_knots()
    elif d == "L":
        for i in range(n):
            unmark_knots()
            knots[0][0] -= 1
            adjust_knots()
            mark_knots()
    elif d == "U":
        for i in range(n):
            unmark_knots()
            knots[0][1] += 1
            adjust_knots()
            mark_knots()
    else:
        # d == "D"
        for i in range(n):
            unmark_knots()
            knots[0][1] -= 1
            adjust_knots()
            mark_knots()

print(f"Grid seen: {grid_seen}")
