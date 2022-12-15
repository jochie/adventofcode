#!/usr/bin/env python3

import re
import sys
import functools

def read_sensors():
    sensors = []
    for line in sys.stdin:
        line = line.rstrip()
        m = re.match(r"^Sensor at x=(-?\d+), y=(-?\d+): closest beacon is at x=(-?\d+), y=(-?\d+)$", line)
        if m:
            sensors.append([ int(m.group(1)), int(m.group(2)), int(m.group(3)), int(m.group(4)) ])
        else:
            print("Wut?")
            sys.exit()
    return sensors

def plot_grid(sensor, checking_x, checking_y):
    distance = abs(sensor[0] - sensor[2]) + abs(sensor[1] - sensor[3])
    y_offset = abs(checking_y - sensor[1])
    # print(f"({sensor[0]},{sensor[1]}) - ({sensor[2]},{sensor[3]}) - distance {distance}; Y offset {y_offset}")
    if y_offset > distance:
        return None
    left = distance - y_offset
    min_x = sensor[0] - left
    max_x = sensor[0] + left
    if min_x < 0 and max_x < 0:
        return None
    if min_x > checking_x and max_x > checking_x:
        return None
    if min_x < 0:
        min_x = 0
    if max_x > checking_x:
        max_x = checking_x
    return [ min_x, max_x, '#', sensor ]

def compare_sections(a, b):
    if a[0] < b[0]:
        return -1
    if a[0] > b[0]:
        return 1
    return 0

def main():
    checking = 4000000
    sensors = read_sensors()
    for y in range(checking + 1):
        # print(f"Scanning row {y}")
        sections = []
        for sensor in sensors:
            section = plot_grid(sensor, checking, y)
            if section:
                sections.append(section)
        sections = sorted(sections, key=functools.cmp_to_key(compare_sections))
        # print(f"SECTIONS:")
        x = -1
        for section in sections:
            # print(f"  [{section[0]}..{section[1]}] - {section[2]}")
            if section[0] > x + 1:
                print(f"Gap found at {x+1},{y}")
                print(f"Tuning frequency for that position is: {(x+1)*4000000+y}")
                sys.exit(0)
            if section[1] > x:
                x = section[1]
        if x < checking:
            print(f"Gap found at {x+1},{y}")
            print(f"Tuning frequency for that position is: {(x+1)*4000000+y}")
            sys.exit(0)

main()
