#!/usr/bin/env python3

import json
import re
import sys
import pydot

# graph = pydot.Dot("AoC - Year 2023, Day 25", graph_type="graph", bgcolor="white", rankdir = 'UD')
graph = pydot.Dot("AoC - Year 2023, Day 25", graph_type="graph", bgcolor="white")

nodes = {}
targets = {}

for line in sys.stdin:
    line = line.rstrip()
    m = re.match(r"([a-z]+): (.*)$", line)
    if m:
        mylist = {}
        targ = m.group(2)
        for t in targ.split(" "):
            targets[t] = True
            mylist[t] = True
        
        nodes[m.group(1)] = {
            "t": 0,
            "l": mylist
        }
        continue

for node in targets:
    if node not in nodes:
        nodes[node] = {
            "t": 3, # Modules that only exist as a target
            "l": {}
        }

for node in nodes.keys():
    t = nodes[node]["t"]
    if nodes[node]["t"] == 0:
        my_node = pydot.Node(node, label=f"{node}",
                             style="filled", fillcolor="green")
    if nodes[node]["t"] == 3:
        my_node = pydot.Node(node, label=f"{node}",
                             style="filled", fillcolor="green")
    graph.add_node(my_node)

seen = {}
for node in nodes.keys():
    for t in nodes[node]["l"]:
        # print(f"Checking {node},{t}...")
        if f"{node},{t}" in seen:
            pass # print("  Already seen before.")
        else:
            seen[f"{node},{t}"] = True
            seen[f"{t},{node}"] = True
            my_edge = pydot.Edge(node, t, color="black")
            graph.add_edge(my_edge)

print(graph.create_svg().decode('utf8'))
