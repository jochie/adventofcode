#!/usr/bin/env python3

"""
Python code template for AoC programs
"""

import argparse
import copy
from functools import cache, cmp_to_key
import re
import sys
import time


def parse_options():
    """
    Parser command line options
    """
    parser = argparse.ArgumentParser(
        description='This program is used for one of the AoC 2024 puzzles; Day 21'
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


DIRS = [
    [ -1,  0, '^' ],
    [  0,  1, '>' ],
    [  1,  0, 'v' ],
    [  0, -1, '<' ]
]

def on_grid(max_row, max_col, row, col):
    return row >= 0 and col >= 0 and row < max_row and col < max_col


# What are the shortest sequences to go from each button to one of the
# other buttons
def map_keypad(opts, buttons, numeric):
    if opts.debug:
        print(f"Mapping the keyboard {buttons}")
    directions = {}
    max_row = len(buttons)
    max_col = len(buttons[0])

    locations = {}
    for row in range(len(numeric)):
        for col in range(len(numeric[0])):
            locations[numeric[row][col]] = [ row, col ]

    for row in range(max_row):
        for col in range(max_col):
            if opts.debug:
                print(f"Checking for directions from {row},{col} -> {buttons[row][col]}")
            if buttons[row][col] == ' ':
                # Don't go here
                continue
            from_button = buttons[row][col]
            checking = [ [ row, col, '', {f"{row},{col}": True} ] ]
            while True:
                if opts.debug:
                    print(checking)
                new_checking = []
                for at_row, at_col, seq, seen in checking:
                    for rel_row, rel_col, rel_dir in DIRS:
                        new_row = rel_row + at_row
                        new_col = rel_col + at_col
                        if not on_grid(max_row, max_col, new_row, new_col):
                            continue
                        if buttons[new_row][new_col] == ' ':
                            # Don't go there
                            continue
                        new_key = f"{new_row},{new_col}"
                        if new_key in seen:
                            continue
                        combo = f"{from_button}{buttons[new_row][new_col]}"

                        if combo not in directions:
                            directions[combo] = [ len(seq + rel_dir), [ seq + rel_dir ] ]
                        else:
                            if len(seq + rel_dir) < directions[combo][0]:
                                directions[combo] = [ len(seq + rel_dir), [ seq + rel_dir ] ]
                            elif len(seq + rel_dir) == directions[combo][0]:
                                directions[combo][1].append(seq + rel_dir)

                        new_seen = copy.deepcopy(seen)
                        new_seen[new_key] = True

                        new_checking.append([ new_row, new_col, seq + rel_dir, new_seen ])
                if not len(new_checking):
                    break
                checking = new_checking

    dir_mapping = {}

    # Only return the sequences, not the minimum length
    for combo, combo_data in directions.items():
        dir_mapping[combo] = combo_data[1]
    return dir_mapping


def find_sequence(opts, code, botlevel):

    @cache
    def generate_expansions(last_char, combo):
        if len(combo) == 1:
            if last_char == combo[0]:
                return [['A']]
            results = []
            for dirs in directions['C'][f"{last_char}{combo[0]}"]:
                results.append([ dirs + 'A'])
            return results

        results = []
        if last_char == combo[0]:
            for expansion in generate_expansions(combo[0], combo[1:]):
                results.append([ 'A' ] + expansion)
            return results

        for expansion in generate_expansions(combo[0], combo[1:]):
            for dirs in directions['C'][f"{last_char}{combo[0]}"]:
                results.append([ dirs + 'A' ] + expansion)
        return results

    @cache
    def find_min_expansion(combo, level, prefix):
        if opts.debug:
            print(f"{prefix}find_min_expansion('{combo}', {level})")
        min_expansion = 0
        for expansion in generate_expansions('A', combo):
            min_combined = 0
            for exp_bit in expansion:
                if level > 1:
                    min_combined += find_min_expansion(exp_bit, level - 1, prefix + '    ')
                else:
                    min_combined += len(exp_bit)
            expansion = min_combined
            if not min_expansion or expansion < min_expansion:
                min_expansion = expansion

        return min_expansion

    directions = {
        'N': map_keypad(opts, ["789", "456", "123", " 0A"], [" ^A", "<v>"]),
        'C': map_keypad(opts, [" ^A", "<v>"], [" ^A", "<v>"])
    }

    if opts.debug:
        print(directions)

    # We can treat the moves from one key to the next independently,
    # because at the end of each of them all of the directional
    # keypads are back at 'A'

    result = 0
    last_char = 'A'
    for next_char in list(code):
        min_expansion = None
        combos = directions['N'][f"{last_char}{next_char}"]
        # print(next_char, combos)
        for combo in combos:
            combo += 'A'

            expansion1 = find_min_expansion(combo, botlevel, '')
            if not min_expansion or expansion1 < min_expansion:
                min_expansion = expansion1
        result += min_expansion
        last_char = next_char

    return result


def run_part1(opts, lines):
    if opts.debug:
        print(opts)
        print(lines)

    botlevel = 2
    total_complexity = 0
    for code in lines:
        len_sequence = find_sequence(opts, code, botlevel)
        numeric = int(code[:3])
        complexity = len_sequence * numeric
        if opts.verbose:
            print(f"Complexity for {code} is {len_sequence} * {numeric} = {complexity}")
        total_complexity += complexity
    return total_complexity


def run_part2(opts, lines):
    if opts.debug:
        print(opts)
        print(lines)

    botlevel = 25
    total_complexity = 0
    for code in lines:
        len_sequence = find_sequence(opts, code, botlevel)
        numeric = int(code[:3])
        complexity = len_sequence * numeric
        if opts.verbose:
            print(f"Complexity for {code} is {len_sequence} * {numeric} = {complexity}")
        total_complexity += complexity
    return total_complexity


def parse_file(opts, filename):
    lines = []
    try:
        with open(filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                lines.append(line)
    except FileNotFoundError:
        print(f"ERROR: File not found {filename}")
        sys.exit(1)
    return lines,


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
                "sample": 126384,
                "input":  155252
            }
        },
        2: {
            "call": run_part2,
            "files": {
                "sample": 154115708116294, # Determined retroactively
                "input":  195664513288128
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
