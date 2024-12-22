#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
import re
import sys

import PIL
from PIL import Image, ImageDraw, ImageFont

def parse_options():
    """
    Parser command line options
    """
    parser = argparse.ArgumentParser(
        description='This program is used for one of the AoC 2024 puzzles; Day 14'
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
    parser.add_argument('-s', '--seconds',
                        help="How many seconds to run",
                        type=int,
                        default=100)
    parser.add_argument('-f', '--filename',
                        help="The filename to read (sample, input)",
                        required=True)
    return parser.parse_args()


def display_grid(grid_size, pos_list, suppress=False):
    grid = []
    for row in range(grid_size[1]):
        line = []
        for col in range(grid_size[0]):
            line.append(".")
        grid.append(line)
    for pos in pos_list:
        if grid[pos[1]][pos[0]] == '.':
            
            grid[pos[1]][pos[0]] = '1'
        else:
            grid[pos[1]][pos[0]] = str(int(grid[pos[1]][pos[0]]) + 1)
    value = 0
    max_seq = 0
    for line in grid:
        seq = 0
        for entry in line:
            if entry == '.':
                seq = 0
            else:
                seq += 1
                if seq > max_seq:
                    max_seq = seq
        if not suppress:
            print(f"{''.join(line)} - {max_seq}")
    return max_seq

def draw_grid(grid_size, seconds, pos_list):
    grid = []
    for row in range(grid_size[1]):
        line = []
        for col in range(grid_size[0]):
            line.append(".")
        grid.append(line)
    for pos in pos_list:
        if grid[pos[1]][pos[0]] == '.':
            grid[pos[1]][pos[0]] = '1'
        else:
            grid[pos[1]][pos[0]] = str(int(grid[pos[1]][pos[0]]) + 1)
    img = Image.new('RGB', grid_size, "#a0a0a0")
    for ix, line in enumerate(grid):
        for iy, entry in enumerate(line):
            if entry != '.':
                img.putpixel((iy, ix), (0, 0, 0))
    drawing = ImageDraw.Draw(img)
    drawing.text((0, 0), f"Frame: {seconds}")
    return img


def save_frames(grid_size, frames):
    frame0 = frames[0]
    frame0.save("tree-frames.gif",
                save_all=True,
                append_images=frames[1:])
    # Optional parameters:
    # - duration=1 (milliseconds)
    # - loop=1 (how many additional times to loop through?)


def mult_quadrants(grid_size, pos_list):
    quadrants = [
        [ 0,                     0,                     grid_size[0] // 2 - 1, grid_size[1] // 2 - 1 ],
        [ grid_size[0] // 2 + 1, 0,                     grid_size[0] - 1,      grid_size[1] // 2 - 1 ],
        [ 0,                     grid_size[1] // 2 + 1, grid_size[0] // 2 - 1, grid_size[1] - 1      ],
        [ grid_size[0] // 2 + 1, grid_size[1] // 2 + 1, grid_size[0] - 1,      grid_size[1] - 1      ]
    ]
    quad_totals = [ 0, 0, 0, 0 ]
    for pos in pos_list:
        for ix, quad in enumerate(quadrants):
            if quad[0] <= pos[0] <= quad[2] and quad[1] <= pos[1] <= quad[3]:
                quad_totals[ix] += 1

    mult = 1
    for total in quad_totals:
        mult *= total
    return mult


def run_part1(opts, grid_size, robots):
    if opts.debug:
        print(opts)
        print(grid_size)
        print(robots)

    pos_list = []
    for robot in robots:
        pos_list.append([ robot[0], robot[1] ])
    if opts.verbose:
        display_grid(grid_size, pos_list)

    pos_list = []
    for robot in robots:
        x = robot[0]
        y = robot[1]
        v_x = robot[2]
        v_y = robot[3]

        # Jump to where we expect to be in opts.seconds seconds:
        x_new = x + v_x * opts.seconds
        y_new = y + v_y * opts.seconds

        # Wrap around the grid's edges:
        x_new = x_new % grid_size[0]
        y_new = y_new % grid_size[1]

        pos_list.append([ x_new, y_new ])
    if opts.verbose:
        print("")
        display_grid(grid_size, pos_list)

    return mult_quadrants(grid_size, pos_list)


def run_part2(opts, grid_size, robots):
    if opts.debug:
        print(opts)
        print(grid_size)
        print(robots)

    pos_list = []
    for robot in robots:
        pos_list.append([ robot[0], robot[1] ])

    frames = []
    seconds = 1
    while True:
        pos_list = []
        for robot in robots:
            x = robot[0]
            y = robot[1]
            v_x = robot[2]
            v_y = robot[3]

            # Move to the next spot:
            x_new = x + v_x
            y_new = y + v_y

            # Wrap around the grid's edges:
            x_new = x_new % grid_size[0]
            y_new = y_new % grid_size[1]

            # Update the robot with the new position
            robot[0] = x_new
            robot[1] = y_new
            pos_list.append([ x_new, y_new ])

        # Heuristic (realized only in retrospect) to detect the easter
        # egg Christmas tree pattern:
        frame = draw_grid(grid_size, seconds, pos_list)
        frames.append(frame)
        if display_grid(grid_size, pos_list, suppress=True) > 10:
            if opts.verbose:
                print("")
                print(f"Position after {seconds} second(s)")
                display_grid(grid_size, pos_list)
            save_frames(grid_size, frames)
            return seconds
        seconds += 1
    return -2


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()
    robots = []
    try:
        with open(opts.filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                line_re = re.match(r"^p=(\d+),(\d+) v=(-?\d+),(-?\d+)$", line)
                if not line_re:
                    print(f"Failed to parse line: {line}")
                else:
                    robots.append([
                        int(line_re.group(1)),
                        int(line_re.group(2)),
                        int(line_re.group(3)),
                        int(line_re.group(4))
                    ])
    except FileNotFoundError:
        print(f"ERROR: File not found {opts.filename}")
        sys.exit(1)
    # Done reading

    # Function to call, expected values, for sanity-checking (sample)
    # and later factoring (both):
    run = {
        "grid": {"sample":  [11, 7],
                 "sample1": [11, 7],
                 "input":   [101, 103]},
        1: { "call":   run_part1,
             "sample": 12,
             "input":  218619324 },
        2: { "call":   run_part2,
             "grid":   [101, 103],
             "input":  6446 }
    }

    answer = run[opts.part]["call"](opts, run["grid"][opts.filename], robots)
    if opts.filename in run[opts.part]:
        if answer == run[opts.part][opts.filename]:
            if opts.verbose:
                print(f"Confirmed expected value from part {opts.part}, filename '{opts.filename}'")
        else:
            print(f"Warning: Unexpected value from part {opts.part}, filename '{opts.filename}'")
    print(f"Answer: {answer}")


if __name__ == "__main__":
    sys.exit(main())
