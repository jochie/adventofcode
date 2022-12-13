#!/usr/bin/env python3

import sys
import json
import functools

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

def main():
    lines.append([ [ 2 ] ])
    lines.append([ [ 6 ] ])
    for line in sys.stdin:
        line = line.rstrip()
        if line != "":
            lines.append(json.loads(line))
    newlines = sorted(lines, key=functools.cmp_to_key(compare_lists))
    print(f"SORTED LIST:")
    i = 0
    key = 1
    for line in reversed(newlines):
        i += 1
        enc = json.dumps(line)
        if enc == "[[2]]" or enc == "[[6]]":
            key *= i
        print(f"{i} - {line}")

    print(f"Decoder key = {key}")
main()
