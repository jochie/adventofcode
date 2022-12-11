#!/usr/bin/env python3

import re
import json
import sys

monkeys = {}
monkey = -1
data = {}

factor = 1
for line in sys.stdin:
    line = line.rstrip()
    m = re.match(r"^Monkey (\d+):", line)
    if m:
        if monkey >= 0:
            data['inspections'] = 0
            monkeys[monkey] = data
            data = {}
        monkey = int(m.group(1))
    else:
        m = re.match(r"^\s*Starting items: (.*)", line)
        if m:
            data['items'] = re.split(r", ", m.group(1))
        m = re.match(r"^\s*Operation: new = old (.) (\d+|old)", line)
        if m:
            data['operation'] = [ m.group(1), m.group(2) ]
        m = re.match(r"^\s*Test: divisible by (\d+)", line)
        if m:
            data['test'] = int(m.group(1))
            factor *= data['test']
        m = re.match(r"^\s*If true: throw to monkey (\d+)", line)
        if m:
            data['T'] = int(m.group(1))
        m = re.match(r"^\s*If false: throw to monkey (\d+)", line)
        if m:
            data['F'] = int(m.group(1))

print(f"Factor = {factor}")

if monkey >= 0:
    data['inspections'] = 0
    monkeys[monkey] = data

mtotal = len(monkeys)
print(json.dumps(monkeys))

for round in range(10000):
    print(f"ROUND {round + 1}")
    for monkey in range(mtotal):
        # print(f"  MONKEY {monkey}")
        for item in monkeys[monkey]['items']:
            monkeys[monkey]['inspections'] += 1
            # print(f"    ITEM {item}")
            old = int(item)
            (op, rhs) = monkeys[monkey]['operation']
            if rhs == "old":
                rhs = old
            else:
                rhs = int(rhs)
            if op == "+":
                new = old + rhs
            else:
                new = old * rhs
            if new >= 2 * factor:
                new = new % factor
            if new % monkeys[monkey]['test']:
                target = monkeys[monkey]['F']
            else:
                target = monkeys[monkey]['T']
            # print(f"Moving {new} to {target}")
            monkeys[target]['items'].append(new)
        # Empty this list
        monkeys[monkey]['items'] = []

    print("INSPECTIONS:")
    l = []
    for monkey in range(mtotal):
        val = monkeys[monkey]['inspections']
        print(f"Monkey {monkey} inspected items {val}")
        l.append(val)
    l.sort()
    print(f"Monkey business level: {l[mtotal-2]} * {l[mtotal-1]} = {l[mtotal-2] * l[mtotal-1]}")
