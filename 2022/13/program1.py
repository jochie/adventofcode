#!/usr/bin/env python3

import sys
import json

lines = []

#  1 in order
#  0 undecided
# -1 out of order
def compare_lists(line1, line2):
    while len(line1) > 0 and len(line2) > 0:
        pop1 = line1[0]
        pop2 = line2[0]
        line1 = line1[1:]
        line2 = line2[1:]
        if type(pop1) == int and type(pop2) == int:
            if pop1 < pop2:
                return 1
            elif pop1 > pop2:
                return -1
        elif type(pop1) == list and type(pop2) == list:
            # Check recursively
            res = compare_lists(pop1, pop2)
            if res != 0:
                return res
        else:
            if type(pop1) == int:
                pop1 = [ pop1 ]
            else:
                pop2 = [ pop2 ]
            # Check recursively
            res = compare_lists(pop1, pop2)
            if res != 0:
                return res
    if len(line1) > 0:
        return -1
    if len(line2) > 0:
        return 1
    return 0

pairs = 0
ordered = 0
for line in sys.stdin:
    line = line.rstrip()
    if line == "":
        pairs += 1
        print(f"PAIR {pairs}")
        if compare_lists(lines[0], lines[1]) >= 0:
            print(f"In order")
            ordered += pairs
        else:
            print(f"Out of order")
        lines = []
    else:
        lines.append(json.loads(line))

if len(lines) == 2:
    pairs += 1
    print(f"PAIR {pairs}")
    if compare_lists(lines[0], lines[1]) >= 0:
        print(f"In order")
        ordered += pairs
    else:
        print(f"Out of order")

print(f"Sum of indices that are in order: {ordered}")
