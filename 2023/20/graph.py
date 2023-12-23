#!/usr/bin/env python3

import json
import re
import sys
import pydot

graph = pydot.Dot("AoC - Year 2023, Day 20", graph_type="digraph", bgcolor="white", rankdir = 'UD')

nodes = {}
targets = {}

for line in sys.stdin:
    line = line.rstrip()
    m = re.match(r"([a-z]+) -> (.*)$", line)
    if m:
        mylist = {}
        targ = m.group(2)
        for t in targ.split(", "):
            targets[t] = True
            mylist[t] = True
        
        nodes[m.group(1)] = {
            "t": 0, # The start (broadcaster)
            "l": mylist
        }
        continue
    m = re.match(r"%([a-z]+) -> (.*)$", line)
    if m:
        mylist = {}
        targ = m.group(2)
        for t in targ.split(", "):
            targets[t] = True
            mylist[t] = True
        nodes[m.group(1)] = {
            "t": 1, # Flip-flop modules
            "l": mylist
        }
        continue
    m = re.match(r"&([a-z]+) -> (.*)$", line)
    if m:
        mylist = {}
        for target in m.group(2).split(", "):
            targets[target] = True
            mylist[target] = True
        nodes[m.group(1)] = {
            "t": 2, # Conjunction modules
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
    elif nodes[node]["t"] == 1:
        my_node = pydot.Node(node, label=f"%{node}",
                             style="filled", fillcolor="yellow")
    elif nodes[node]["t"] == 2:
        my_node = pydot.Node(node, label=f"&{node}",
                             style="filled", fillcolor="red")
    elif nodes[node]["t"] == 3:
        my_node = pydot.Node(node, label=f"{node}",
                             style="filled", fillcolor="cyan")
    graph.add_node(my_node)

for node in nodes.keys():
    for t in nodes[node]["l"]:
        my_edge = pydot.Edge(node, t, color="black")
        graph.add_edge(my_edge)

print(graph.create_svg().decode('utf8'))
