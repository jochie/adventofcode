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
        description='This program is used for one of the AoC 2024 puzzles; Day 17'
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

def combo_operand(opts, registers, operand):
    if operand < 4:
        return operand
    if operand == 7:
        print("Invalid combo operand?")
        sys.exit(1)
    return registers[chr(ord('A') + operand - 4)]

def desc_combo_operand(operand):
    if operand < 4:
        return operand
    return chr(ord('A') + operand - 4)

def disassemble(opts, program):
    ix = 0
    print("IP   OCTETS    ASM     DESC")
    print("---+---------+-------+-----------------")
    while ix < len(program):
        opcode = program[ix]
        operand = program[ix + 1]

        if opcode == 0:
            asm = f"adv {desc_combo_operand(operand)}"
            asm_desc = f"A = A / (2 ** {desc_combo_operand(operand)})"
        elif opcode == 1:
            asm = f"bxl {operand}"
            asm_desc = f"B = B ^ {operand}"
        elif opcode == 2:
            asm = f"bst {desc_combo_operand(operand)}"
            asm_desc = f"B = {desc_combo_operand(operand)} % 8"
        elif opcode == 3:
            asm = f"jnz {operand}"
            asm_desc = ""
        elif opcode == 4:
            asm = f"bxc"
            asm_desc = "B = B ^ C"
        elif opcode == 5:
            asm = f"out {desc_combo_operand(operand)}"
            asm_desc = f"Output {desc_combo_operand(operand)} % 8"
        elif opcode == 6:
            asm = f"bdv {desc_combo_operand(operand)}"
            asm_desc = f"B = A / (2 ** {desc_combo_operand(operand)})"
        elif opcode == 7:
            asm = f"cdv {desc_combo_operand(operand)}"
            asm_desc = f"C = A / (2 ** {desc_combo_operand(operand)})"
        else:
            # Should never happen
            asm = "?"
            asm_desc = "?"
        print(f"{ix:2}   <{opcode}> <{operand}>   {asm:5}   {asm_desc}")
        ix += 2
    
def run_opcode(opts, registers, program):
    ip = registers["IP"]
    opcode = program[ip]
    operand = program[ip + 1]
    if opts.debug:
        print(f"{ip} : {opcode} {operand}")
        print(f"    REGISTERS: A: {registers['A']}; B: {registers['B']}; C: {registers['C']}")

    if opcode == 0:
        # adv
        num = registers["A"]
        den = 2 ** combo_operand(opts, registers, operand)
        if opts.debug:
            print(f"    adv {num} / {den}")
        registers["A"] = num // den
        if opts.debug:
            print(f"    {registers['A']} -> 'A'")
        registers["IP"] += 2
        return

    if opcode == 1:
        # bxl
        if opts.debug:
            print(f"    bxl {registers['B']}, {operand}")
        registers["B"] = registers["B"] ^ operand
        registers["IP"] += 2
        return

    if opcode == 2:
        # bst
        val = combo_operand(opts, registers, operand) % 8
        if opts.debug:
            print(f"    bst {val}")
        registers["B"] = val
        registers["IP"] += 2
        return

    elif opcode == 3:
        # jnz
        if opts.debug:
            print(f"    jnz")
        if registers["A"] == 0:
            registers["IP"] += 2
            return
        registers["IP"] = operand
        return

    elif opcode == 4:
        # bxc
        if opts.debug:
            print(f"    bxc {registers['B']}, {registers['C']}")
        registers['B'] = registers['B'] ^ registers['C']
        registers['IP'] += 2
        return

    elif opcode == 5:
        # out
        val = combo_operand(opts, registers, operand) % 8
        if opts.debug:
            print(f"    out {val}")
        registers["OUT"].append(str(val))
        registers["IP"] += 2
        return

    elif opcode == 6:
        # bdv
        num = registers["A"]
        den = 2 ** combo_operand(opts, registers, operand)
        if opts.debug:
            print(f"    bdv {num} / {den}")
        registers["B"] = num // den
        if opts.debug:
            print(f"    {registers['B']} -> 'B'")
        registers["IP"] += 2
        return

    elif opcode == 7:
        # cdv
        num = registers["A"]
        den = 2 ** combo_operand(opts, registers, operand)
        if opts.debug:
            print(f"    cdv {num} / {den}")
        registers["C"] = num // den
        if opts.debug:
            print(f"    {registers['C']} -> 'C'")
        registers["IP"] += 2
        return

def run_part1(opts, registers, program):
    # Set the instructions pointer
    registers["IP"] = 0

    # Collection of output
    registers["OUT"] = []

    if opts.debug or opts.verbose:
        print()
        disassemble(opts, program)
        print()
    
    if opts.debug:
        print(opts)
        print(registers, program)

    while True:
        run_opcode(opts, registers, program)
        if registers["IP"] >= len(program):
            break

    if opts.debug:
        print(f"    REGISTERS: A: {registers['A']}; B: {registers['B']}; C: {registers['C']}")
    return ",".join(registers["OUT"])


def run_part2(opts, registers, program):
    # Placeholder return value
    if opts.debug:
        print(opts)
        print(registers, program)

    if opts.debug or opts.verbose:
        print()
        disassemble(opts, program)
        print()

    compare = "".join([str(x) for x in program])

    offset = 0
    digits = []
    for x in range(len(program)):
        digits.append('0')
    checked = 0
    checking = [ digits ]
    solutions = []
    while offset < len(program):
        new_checking = []
        # print(f"Checking {checking}")
        found = False
        for digits in checking:
            checked += 1
            for octet in range(8):
                digits[offset] = str(octet)
                initial = int("".join(digits), 8) # Convert from base 8 to an decimal
                registers["IP"] = 0
                registers["OUT"] = []
                registers["B"] = 0
                registers["C"] = 0
                registers["A"] = initial
                while True:
                    run_opcode(opts, registers, program)
                    if registers['IP'] >= len(program):
                        break
                output = ''.join(registers['OUT'])
                if output == compare:
                    if opts.verbose:
                        print(f"Potential solution found: {initial}")
                    found = True
                    solutions.append(initial)
                    continue
                if len(registers['OUT']) == len(program):
                    if (registers['OUT'][len(program) - 1 - offset] == str(program[len(program) - 1 - offset])):
                        if opts.debug:
                            print(f"Using {initial:o} => {output} for the next round")
                        found = True
                        new_checking.append(copy.deepcopy(digits))
        checking = new_checking
        if not len(checking):
            break
        offset += 1
    if opts.verbose:
        print(f"Solutions found (total: {len(solutions)}): {solutions}")
        print(f"Input variations checked: {checked}")
    return sorted(solutions)[0]


def parse_file(opts, filename):
    registers = {}
    program = []
    try:
        with open(filename, "r", encoding="utf-8") as input_fd:
            for line in input_fd:
                line = line.rstrip()
                if opts.debug:
                    print(f"DEBUG: Line received: '{line}'")
                line_re = re.match(r"Register ([A-Z]): (\d+)", line)
                if line_re:
                    registers[line_re.group(1)] = int(line_re.group(2))
                line_re = re.match(r"Program: (.*)", line)
                if line_re:
                    program = [int(x) for x in line_re.group(1).split(",")]
    except FileNotFoundError:
        print(f"ERROR: File not found {filename}")
        sys.exit(1)
    return registers, program


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
                "sample": "4,6,3,5,6,3,5,2,1,0",
                "input":  "2,7,6,5,6,0,2,3,1"
            }
        },
        2: {
            "call": run_part2,
            "files": {
                "input":  107416870455451
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
