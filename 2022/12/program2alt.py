#!/usr/bin/env python

# Solution suggested by someone else, to search the shortest path to
# from any 'a' level spot to 'E' by starting at 'E' and finding the
# first 'a' level spot, instead, reversing the possible_path()
# condition.

import sys

end = [ 0, 0 ]
row = 0
map = []
for line in sys.stdin:
    line = line.rstrip()
    map_row = []
    width = len(line)
    for i in range(width):
        if line[i] == "S":
            map_row.append(0)
        elif line[i] == "E":
            map_row.append(25)
            end = [ row, i ]
        else:
            map_row.append(ord(line[i]) - ord('a'))
    map.append(map_row)
    row += 1

height = row
print(f"Height = {height}; width = {width}")
print(f"End at {end}")

def pos_key(p):
    return f"{p[0]},{p[1]}"

def possible_path(cur, pos):
    if pos_key(pos) in seen:
        # Somebody has already been there, so that shouldn't be a path
        return False
    # Check height difference
    diff = map[cur[0]][cur[1]] - map[pos[0]][pos[1]]
    return diff <= 1

def find_paths(pos):
    paths = []
    if pos[0] > 0:
        # Check to the left
        if possible_path(pos, [ pos[0] - 1, pos[1] ]):
            paths.append([ pos[0] - 1, pos[1] ])
    if pos[0] < height - 1:
        # Check to the right
        if possible_path(pos, [ pos[0] + 1, pos[1] ]):
            paths.append([ pos[0] + 1, pos[1] ])
    if pos[1] > 0:
        # Check up:
        if possible_path(pos, [ pos[0], pos[1] - 1 ]):
            paths.append([ pos[0], pos[1] - 1 ])
    if pos[1] < width - 1:
        # Check down:
        if possible_path(pos, [ pos[0], pos[1] + 1 ]):
            paths.append([ pos[0], pos[1] + 1 ])
    return paths

def shortest_path(start):
    global seen
    seen = { pos_key(start): 1 }
    checking = [ start ]
    steps = 0
    while True:
        steps += 1
        # print(f"STEP {steps}")
        next_paths = []
        for cur in checking:
            paths = find_paths(cur)
            for pos in paths:
                seen[pos_key(pos)] = 1
                if map[pos[0]][pos[1]] == 0:
                    return steps
                next_paths.append(pos)
        # print(f"Checking next at: {next_paths}")
        checking = next_paths
        if len(checking) == 0:
            return -1

shortest = shortest_path(end)
print(f"Shortest possible path: {shortest} steps.")
