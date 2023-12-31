#!/usr/bin/env python3

import json
import re
import sys
import pydot

graph = pydot.Dot("AoC - Year 2015, Day 7", graph_type="digraph", bgcolor="lightgray", rankdir = 'UD')

nodes = {}
targets = {}

for line in sys.stdin:
    line = line.rstrip()
    m = re.match(r"^([a-z]+) -> ([a-z]+)$", line)
    if m:
        targets[m.group(1)] = True
        targets[m.group(2)] = True
        nodes[m.group(2)] = {
            "type": "ASSIGN",
            "src":  [ m.group(1) ],
            "dst":  m.group(2),
            "info": line,
        }
        continue
    m = re.match(r"^([0-9]+) -> ([a-z]+)$", line)
    if m:
        targets[m.group(2)] = True
        nodes[m.group(2)] = {
            "type": "ASSIGN",
            "src":  [ ],
            "dst":  m.group(2),
            "info": line,
        }
        continue
    m = re.match(r"^([a-z]+) (AND|OR|LSHIFT|RSHIFT) ([a-z]+) -> ([a-z]+)$", line)
    if m:
        targets[m.group(1)] = True
        targets[m.group(3)] = True
        targets[m.group(4)] = True
        nodes[m.group(4)] = {
            "type":  m.group(2),
            "src":  [ m.group(1), m.group(3) ],
            "dst":  m.group(2),
            "info": line,
        }
        continue
    m = re.match(r"^([a-z]+) (AND|OR|LSHIFT|RSHIFT) ([0-9]+) -> ([a-z]+)$", line)
    if m:
        targets[m.group(1)] = True
        targets[m.group(4)] = True
        nodes[m.group(4)] = {
            "type":  m.group(2),
            "src":  [ m.group(1) ],
            "dst":  m.group(2),
            "info": line,
        }
        continue
    m = re.match(r"^([0-9]+) (AND|OR|LSHIFT|RSHIFT) ([a-z]+) -> ([a-z]+)$", line)
    if m:
        targets[m.group(3)] = True
        targets[m.group(4)] = True
        nodes[m.group(4)] = {
            "type":  m.group(2),
            "src":  [ m.group(3) ],
            "dst":  m.group(2),
            "info": line,
        }
        continue
    m = re.match(r"^NOT ([a-z0-9]+) -> ([a-z]+)$", line)
    if m:
        targets[m.group(1)] = True
        targets[m.group(2)] = True
        nodes[m.group(2)] = {
            "type": "NOT",
            "src": [ m.group(1) ],
            "dst": m.group(2),
            "info": line,
        }
        continue

for node in targets:
    if node not in nodes:
        nodes[node] = {
            "type": "VALUE", # Modules that only exist as a source
            "src":  [],
            "info": node
        }


color_map = {
    "AND":    "lightgreen",
    "OR":     "lightgreen",
    "NOT":    "yellow",
    "LSHIFT": "cyan",
    "RSHIFT": "cyan",
    "ASSIGN": "white",
    "VALUE":  "red"
}

for node in nodes.keys():
    my_node = pydot.Node(node, label=f"{nodes[node]['info']}",
                         style="filled", fillcolor=color_map[nodes[node]['type']])
    graph.add_node(my_node)

for node in nodes.keys():
    for t in nodes[node]["src"]:
        my_edge = pydot.Edge(t, node, color="black")
        graph.add_edge(my_edge)

print(graph.create_svg().decode('utf8'))
