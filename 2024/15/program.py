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
        description='This program is used for one of the AoC 2024 puzzles; Day 15'
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


DIR_MAP = {
    '^': [ -1,  0 ],
    'v': [  1,  0 ],
    '<': [  0, -1 ],
    '>': [  0,  1 ]
}


# Calculate the sum of the GPS values, for both part 1 and 2, looking
# for either the original 'O' or the left side of the box '[':
def calculate_sum_gps(opts, grid, max_row, max_col):
    total = 0
    for row in range(max_row):
        for col in range(max_col):
            if grid[row][col] in ['O', '[']:
                if opts.debug:
                    print(f"Box at {row},{col}")
                total += row * 100 + col
    return total


# For verbose/debug (progress) output
def dump_grid(grid, max_row, max_col):
    for row in range(max_row):
        print("".join(grid[row]))
    print()


def run_part1(opts, grid, max_row, max_col, start_row, start_col, moves):
    if opts.debug:
        print(opts)
        print(grid)
        print(max_row, max_col, start_row, start_col, moves)

    row = start_row
    col = start_col
    for ix, move in enumerate(list(moves)):
        rel_row, rel_col = DIR_MAP[move]
        if opts.verbose or opts.debug:
            dump_grid(grid, max_row, max_col)
            print(f"Move {ix + 1}: '{move}' -> ({rel_row},{rel_col})")

        new_row = row + rel_row
        new_col = col + rel_col

        if grid[new_row][new_col] == '#':
            # Hitting a wall, nothing happens
            continue

        if grid[new_row][new_col] == '.':
            # Empty space, move to the new spot
            grid[row][col] = '.'
            row = new_row
            col = new_col
            grid[row][col] = '@'
            continue

        if grid[new_row][new_col] != 'O':
            print(f"#1 Found something unexpected at ({new_row},{new_col}): '{grid[new_row][new_col]}'")
            sys.exit(1)

        # Is there an empty space in that direction?
        space_found = False
        space_row = new_row
        space_col = new_col
        while True:
            space_row += rel_row
            space_col += rel_col
            if grid[space_row][space_col] == '.':
                space_found = True
                break
            if grid[space_row][space_col] == '#':
                # Nope, hitting a wall
                break
            if grid[space_row][space_col] != 'O':
                print(f"#1 Found something unexpected at ({new_row},{new_col}): '{grid[new_row][new_col]}'")
                sys.exit(1)
            # Another box, keep looking
        if not space_found:
            # Nothing happens
            continue

        # Adjust the markings on the grid
        grid[space_row][space_col] = 'O'
        grid[new_row][new_col] = '@'
        grid[row][col] = '.'
        row = new_row
        col = new_col

    if opts.verbose:
        print()
        print("Final layout:")
        dump_grid(grid, max_row, max_col)

    return calculate_sum_gps(opts, grid, max_row, max_col)


# Double the grid, doubling walls and empty spaces, turning boxes from
# 'O' to '[]', and the '@' to '@.'.
def double_grid(grid, max_row, max_col):
    new_grid = []
    for row in range(max_row):
        new_line = []
        for entry in grid[row]:
            if entry == '@':
                new_line.append('@')
                new_line.append('.')
            elif entry == 'O':
                new_line.append('[')
                new_line.append(']')
            elif entry == '.':
                new_line.append('.')
                new_line.append('.')
            else:
                # Must be a #
                new_line.append('#')
                new_line.append('#')
        new_grid.append(new_line)
    return new_grid


# Debug function that I used to find a nasty bug where, in the
# following layout, I would "lose" boxes:
#
# ......                   ......
# ..@...                   ......
# ..[]..                   ..@...
# .[][]. -> Moving down -> ..[]..
# ..[]..                   .[][].
# ......                   ......
#
# This turned out to be because I double-tracked the bottom box after
# encountering it twice on the layer above, and then moving it twice,
# effectively putting empty spaces in its place during the second
# move.

def grid_summary(grid, max_row, max_col):
    summary = {
        '#': 0,
        'O': 0,
        '@': 0,
        '[': 0,
        ']': 0
    }
    for row in range(max_row):
        for col in range(max_col):
            if grid[row][col] != '.':
                summary[grid[row][col]] += 1
    return summary


def run_part2(opts, grid, max_row, max_col, start_row, start_col, moves):
    if opts.debug:
        print(opts)
        print(grid)
        print(max_row, max_col, start_row, start_col, moves)

    summary1 = grid_summary(grid, max_row, max_col)
    grid = double_grid(grid, max_row, max_col)

    # Change the width of the grid and the starting column accordingly
    max_col *= 2
    start_col *= 2

    summary2 = grid_summary(grid, max_row, max_col)

    if opts.debug:
        print(summary1)
        print(summary2)

    row = start_row
    col = start_col
    for ix, move in enumerate(list(moves)):
        summary3 = grid_summary(grid, max_row, max_col)
        if summary2 != summary3:
            dump_grid(grid, max_row, max_col)
            print(f"Something went very wrong: {summary2} -> {summary3}")
            sys.exit(1)

        rel_row, rel_col = DIR_MAP[move]
        if opts.verbose or opts.debug:
            dump_grid(grid, max_row, max_col)
            print(f"Move {ix + 1}: '{move}' -> ({rel_row},{rel_col})")
            if opts.debug:
                # Hit enter for every move
                line = sys.stdin.readline()

        new_row = row + rel_row
        new_col = col + rel_col

        if grid[new_row][new_col] == '#':
            # Hitting a wall, nothing happens
            continue

        if grid[new_row][new_col] == '.':
            # Empty space, move to the new spot
            grid[row][col] = '.'
            row = new_row
            col = new_col
            grid[row][col] = '@'
            continue

        if grid[new_row][new_col] not in ['[', ']']:
            print(f"#1 Found something unexpected at ({new_row},{new_col}): '{grid[new_row][new_col]}'")
            sys.exit(1)

        if rel_row == 0:
            # Left/right move is still mostly the same as part 1

            # Is there an empty space in that direction?
            space_found = False
            space_row = new_row
            space_col = new_col
            while True:
                space_row += rel_row
                space_col += rel_col
                if grid[space_row][space_col] == '.':
                    space_found = True
                    break
                if grid[space_row][space_col] == '#':
                    # Nope, hitting a wall
                    break
                if grid[space_row][space_col] not in ['[', ']']:
                    print(f"#1 Found something unexpected at ({new_row},{new_col}): '{grid[new_row][new_col]}'")
                    sys.exit(1)
                # Another box, keep looking

            if not space_found:
                # Nothing happens
                continue

            # Shift all markings to the left/right (this is actually
            # different from part 1)
            while space_col != col:
                grid[space_row][space_col] = grid[space_row][space_col - rel_col]
                space_col -= rel_col
            grid[row][col] = '.'
            row = new_row
            col = new_col
            continue

        # Up/down is a little different.

        # Looking how many boxes might be involved in this.
        boxes_seen = {}
        if grid[new_row][new_col] == '[':
            boxes = [ [ new_row, new_col ] ]
            boxes_seen[f"{new_row},{new_col}"] = True
        else:
            boxes = [ [ new_row, new_col -1 ] ]
            boxes_seen[f"{new_row},{new_col - 1}"] = True

        checking = boxes
        wall_found = False
        while True:
            if opts.debug:
                print("Boxes:", boxes)
                print("Checking:", checking)
            new_boxes = []
            for box in checking:
                if grid[box[0] + rel_row][box[1]] == '.' and grid[box[0] + rel_row][box[1] + 1] == '.':
                    # No more boxes here
                    if opts.debug:
                        print(f"Space above/below {box}")
                    continue

                # Another box directly above/below
                if grid[box[0] + rel_row][box[1]] == '[' and grid[box[0] + rel_row][box[1] + 1] == ']':
                    new_boxes.append([ box[0] + rel_row, box[1] ])
                    if opts.debug:
                        print(f"Box directly above/below {box}")
                    continue

                if grid[box[0] + rel_row][box[1]] == '#' or grid[box[0] + rel_row][box[1] + 1] == '#':
                    wall_found = True
                    if opts.debug:
                        print(f"Box runs into a wall {box}")
                    break

                if grid[box[0] + rel_row][box[1]] == ']':
                    if opts.debug:
                        print(f"Box shifted to the left of {box}")
                    new_boxes.append([ box[0] + rel_row, box[1] - 1 ])

                if grid[box[0] + rel_row][box[1] + 1] == '[':
                    if opts.debug:
                        print(f"Box shifted to the right of {box}")
                    new_boxes.append([ box[0] + rel_row, box[1] + 1 ])

            # Did we find a wall during our search for more boxes?
            if wall_found:
                break

            if not len(new_boxes):
                break
            for box in new_boxes:
                box_key = f"{box[0]},{box[1]}"
                if box_key not in boxes_seen:
                    boxes.append(box)
                    boxes_seen[box_key] = True
            checking = new_boxes

        # Did we encounter a wall while we searched for boxes?
        if wall_found:
            if opts.debug:
                print("Alas, a wall, can't move.")
            continue

        # Move all the boxes in reverse order:
        for box in reversed(boxes):
            if opts.debug:
                print(f" -> move {box}")
            for offset in [0, 1]:
                grid[box[0] + rel_row][box[1] + offset] = grid[box[0]][box[1] + offset]
                grid[box[0]][box[1] + offset] = '.'

        grid[new_row][new_col] = '@'
        grid[row][new_col] = '.'
        row = new_row
        col = new_col

    if opts.verbose:
        print()
        print("Final layout:")
        dump_grid(grid, max_row, max_col)

    return calculate_sum_gps(opts, grid, max_row, max_col)


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()
    grid = []
    moves = ""
    max_row = 0
    max_col = 0
    start_row = -1
    start_col = -1
    try:
        with open(opts.filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                if not len(line):
                    break
                if not max_col:
                    max_col = len(line)
                grid.append(list(line))
                if start_row < 0:
                    for col in range(max_col):
                        if line[col] == '@':
                            start_row = max_row
                            start_col = col
                            break
                max_row += 1
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                moves += line

    except FileNotFoundError:
        print(f"ERROR: File not found {opts.filename}")
        sys.exit(1)
    # Done reading

    # Function to call, expected values, for sanity-checking (sample)
    # and later factoring (both):
    run = {
        1: { "call": run_part1, "sample1": 10092, "input":  1517819 },
        2: { "call": run_part2, "sample1":  9021, "input":  1538862 }
    }
    answer = run[opts.part]["call"](opts, grid, max_row, max_col, start_row, start_col, moves)
    if opts.filename in run[opts.part]:
        if answer == run[opts.part][opts.filename]:
            if opts.verbose:
                print(f"Confirmed expected value from part {opts.part}, filename '{opts.filename}'")
        else:
            print(f"Warning: Unexpected value from part {opts.part}, filename '{opts.filename}'")
    else:
        print(f"Warning: No known answer for part {opts.part}, filename '{opts.filename}'")
    print(f"Answer: {answer}")


if __name__ == "__main__":
    sys.exit(main())
