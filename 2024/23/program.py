#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
import copy
import re
import sys
import time


def parse_options():
    """
    Parser command line options
    """
    parser = argparse.ArgumentParser(
        description='This program is used for one of the AoC 2024 puzzles; Day 23'
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
                        type=int)
    parser.add_argument('-f', '--filename',
                        help="The filename to read (sample, input)")
    parser.add_argument('-t', '--test',
                        help="Run the program with all known files, and all parts unless one is specified.",
                        action='store_true')

    opts = parser.parse_args()

    if not opts.test:
        if not opts.part or not opts.filename:
            print("The --part and --filename options are required unless you use --test.")
            sys.exit(1)

    if opts.test and opts.filename:
        print("The --test and --filename options are mutually exclusive.")
        sys.exit(1)

    return opts


def run_part1(opts, nodes):
    if opts.debug:
        print(opts)
        print(nodes)

    trios = {}
    for node_a in nodes.keys():
        for node_b in nodes[node_a].keys():
            for node_c in nodes.keys():
                if node_a == node_c or node_b == node_c:
                    continue
                c_map = nodes[node_c]
                if node_a in c_map and node_b in c_map:
                    node_list = [node_a, node_b, node_c]
                    node_list.sort()
                    node_list_key = ",".join(node_list)
                    if node_a[0] != 't' and node_b[0] != 't' and node_c[0] != 't':
                        continue
                    if node_list_key not in trios:
                        trios[node_list_key] = True

    return len(trios.keys())


def find_clusters(opts, ix, node_a, nodes):
    in_cluster = True

    biggest = []

    checking = [ [ node_a, { node_a: True } ] ]
    while True:
        new_checking = []
        for cur_node, cluster in checking:
            for next_node in nodes[cur_node].keys():
                if next_node in cluster:
                    # Don't loop back
                    continue

                # This is a node that's new to the current cluster

                # Does it connect to all the ones already seen before?
                in_cluster = True
                for node in cluster.keys():
                    if node not in nodes[next_node].keys():
                        # Nope
                        in_cluster = False
                        break
                if not in_cluster:
                    # Move along
                    continue
                new_cluster = copy.deepcopy(cluster)
                new_cluster[next_node] = True
                new_checking.append([ next_node, new_cluster ])
                if len(new_cluster.keys()) > len(biggest):
                    biggest = new_cluster.keys()
                    break
        if not len(new_checking):
            break
        checking = new_checking
    return list(biggest)


def run_part2(opts, nodes):
    # Placeholder return value
    if opts.debug:
        print(opts)
        print(nodes)

    biggest_cluster = []
    for ix, node_a in enumerate(nodes.keys()):
        node_list = find_clusters(opts, ix, node_a, nodes)
        if not len(biggest_cluster) or len(node_list) > len(biggest_cluster):
            biggest_cluster = node_list
    biggest_cluster.sort()
    return ",".join(biggest_cluster)


def parse_file(opts, filename):
    nodes = {}
    try:
        with open(filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                conn_a, conn_b = line.split("-")
                if conn_a not in nodes:
                    nodes[conn_a] = {}
                if conn_b not in nodes:
                    nodes[conn_b] = {}
                nodes[conn_a][conn_b] = True
                nodes[conn_b][conn_a] = True
    except FileNotFoundError:
        print(f"ERROR: File not found {filename}")
        sys.exit(1)
    return nodes,


def run_part(opts, run, part, filename, params):
    success = False
    time_before = time.process_time()
    answer = run[part]["call"](opts, *params)
    time_diff = time.process_time() - time_before
    if filename in run[part]["files"]:
        if answer == run[part]["files"][filename]:
            if opts.verbose:
                print(f"Confirmed expected value from part {part}, filename '{filename}'")
            success = True
        else:
            print(f"Warning: Unexpected value from part {part}, filename '{filename}'")
    else:
        print(f"Warning: No known answer for part {part}, filename '{filename}'")
    print(f"[Duration {time_diff:.2f}s] Part {part}, filename '{filename}', answer: {answer}")
    return success


def main():
    """
    Main section, where we parse the command line options, read the
    stdin content, and act on it
    """
    opts = parse_options()

    # Function to call, expected values, for sanity-checking (sample)
    # and later factoring (both):
    run = {
        1: {
            "call": run_part1,
            "files": {
                "sample": 7,
                "input":  1476
            }
        },
        2: {
            "call": run_part2,
            "files": {
                "sample": "co,de,ka,ta",
                "input":  "ca,dw,fo,if,ji,kg,ks,oe,ov,sb,ud,vr,xr"
            }
        }
    }

    if not opts.test:
        params = parse_file(opts, opts.filename)
        run_part(opts, run, opts.part, opts.filename, params)
        return 0

    passed = 0
    failed = 0
    for part in run.keys():
        if opts.part and opts.part != part:
            continue
        for filename in run[part]["files"].keys():
            params = parse_file(opts, filename)
            if run_part(opts, run, part, filename, params):
                passed += 1
            else:
                failed += 1
    print(f"Test results: {passed} passed, {failed} failed.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
