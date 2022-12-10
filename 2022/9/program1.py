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

# Head coordinates
h_x = 0
h_y = 0

# Tail coordinates
t_x = 0
t_y = 0

grid_seen = 0

def tail_adjacent():
    if abs(h_x - t_x) > 1:
        return False
    if abs(h_y - t_y) > 1:
        return False
    return True

def mark_tail():
    global grid_seen

    grid[t_y + y_offset][t_x + x_offset][0] = "T"
    if grid[t_y + y_offset][t_x + x_offset][1] == 0:
        grid[t_y + y_offset][t_x + x_offset][1] = 1
        grid_seen += 1
    grid[h_y + y_offset][h_x + x_offset][0] = "H"

#    result = ""
#    for i in reversed(range(y_width)):
#        for j in range(x_width):
#            result = result + (grid[i][j][0])
#        result = result + "\n"
#    print(result)

# Mark the starting position
mark_tail()

for info in data:
    (d, n) = info
    if d == "R":
        for i in range(n):
            h_x = h_x + 1
            if not tail_adjacent():
                t_x = h_x - 1
                t_y = h_y
            mark_tail()
    elif d == "L":
        for i in range(n):
            h_x = h_x - 1
            if not tail_adjacent():
                t_x = h_x + 1
                t_y = h_y
            mark_tail()
    elif d == "U":
        for i in range(n):
            h_y = h_y + 1
            if not tail_adjacent():
                t_y = h_y - 1
                t_x = h_x
            mark_tail()
    else:
        # d == "D"
        for i in range(n):
            h_y = h_y - 1
            if not tail_adjacent():
                t_y = h_y + 1
                t_x = h_x
            mark_tail()

print(f"Grid seen: {grid_seen}")
