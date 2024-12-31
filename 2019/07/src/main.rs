// -*- mode: rust; indent-tabs-mode: nil -*-
//
// Rust code for AoC program

// Useful while developing, allows focus on the bigger picture
#![allow(dead_code, unused_variables)]

extern crate getopts;

use getopts::{Options, Matches};

use std::fs;                   // fs::read_to_string()
use std::env;                  // env::args()
use std::process;              // process::exit()
use std::time::Instant;        // now()
use std::collections::HashMap; // HashMap::new()

use itertools::Itertools;

fn parse_options(args: &Vec<String>) -> Matches {
    let mut opts = Options::new();

    // Basic options
    opts.optflag("h", "help", "Print this help output");
    opts.optflag("d", "debug", "Enable debug output");
    opts.optflag("v", "verbose", "Increase verbosity of the program");
    opts.optflag("n", "dryrun", "Enable dryrun (noop) output");

    // Options for AoC
    opts.optopt("f", "filename", "The filename to read (sample, input)", "FILENAME");
    opts.optopt("p", "part", "Which part to run (1, 2)", "PART");

    // Not used, yet
    opts.optflag("t", "test", "Run the program with all known files, and all parts, unless one is specified.");

    let matches = match opts.parse(&args[1..]) {
        Ok(m) => {
            m
        },
        Err(f) => {
            panic!("{}", f.to_string())
        }
    };

    if matches.opt_present("h") {
        let brief = format!("Usage: {} [options]", args[0]);
        print!("{}", opts.usage(&brief));
        process::exit(1);
    }

    if !matches.opt_present("t") {
        if !matches.opt_present("p") || !matches.opt_present("f") {
            println!("The --part and --filename options are required unless you use --test.");
            process::exit(1);
        }
    }

    if matches.opt_present("t") && matches.opt_present("f") {
        println!("The --test and --filename options are mutually exclusive.");
        process::exit(1);
    }

    matches
}


// Putting the specifics of the different puzzle inputs in this
// struct, to make the various function signatures more generic.

#[derive(Debug, Clone)]
struct Input {
    n: Vec<i32>,
}


// Read the given file and return a list of integers
fn parse_input(matches: &Matches, filename: &String) -> Input {
    let content = match fs::read_to_string(&filename) {
        Ok(data) => data,
        Err(_) => {
            println!("File not found: '{filename}'. Aborting.");
            process::exit(1);
        }
    };
    let mut lines: Vec<i32> = Vec::new();
    for line in content.lines() {
        for num in line.split(',') {
            lines.push(num.parse::<i32>().unwrap());
        }
    }
    Input { n: lines }
}


fn get_parameter(modifier: i32, pos: usize, data: &mut Input) -> i32 {
    if modifier == 0 {
        data.n[data.n[pos] as usize]
    } else {
        data.n[pos]
    }
}


// Return true if we should continue executing code
fn execute_intcode(matches: &Matches, pc_ref: &mut usize, data: &mut Input, input: &mut Vec<i32>, output: &mut Vec<i32>) -> bool {
    let mut pc: usize = *pc_ref;
    let mut inst = data.n[pc as usize];
    let modifier_1;
    let modifier_2;
    let modifier_3; #[allow(unused_assignments)]
    if inst > 100 {
        let modifier = inst / 100;
        modifier_1 = modifier % 10;
        modifier_2 = modifier % 100 / 10;
        modifier_3 = modifier % 1000 / 100;
        inst = inst % 100;
    } else {
        modifier_1 = 0;
        modifier_2 = 0;
        modifier_3 = 0;
    }
    match inst {
        99 => {
            if matches.opt_present("d") {
                println!("[{pc:03}]    99                   -- EXIT");
            }
            return false;
        },
        1 => {
            // Addition
            let param1 = get_parameter(modifier_1, pc + 1, data);
            let param2 = get_parameter(modifier_2, pc + 2, data);
            let param3 = data.n[pc + 3];
            if matches.opt_present("d") {
                println!("[{pc:03}] {:5} {:5} {:5} {:5} -- {} + {} -> {} at {}",
                         data.n[pc], data.n[pc + 1], data.n[pc + 2], data.n[pc + 3],
                         param1, param2, param1 + param2, param3);
            }
            data.n[param3 as usize] = param1 + param2;
            pc += 4;
        },
        2 => {
            // Multiplication
            let param1 = get_parameter(modifier_1, pc + 1, data);
            let param2 = get_parameter(modifier_2, pc + 2, data);
            let param3 = data.n[pc + 3];
            if matches.opt_present("d") {
                println!("[{pc:03}] {:5} {:5} {:5} {:5} -- {} * {} -> {} at {}",
                         data.n[pc], data.n[pc + 1], data.n[pc + 2], data.n[pc + 3],
                         param1, param2, param1 * param2, param3);
            }
            data.n[param3 as usize] = param1 * param2;
            pc += 4;
        },
        3 => {
            // Store the given 'input' value
            let input_val: i32;
            if input.len() > 0 {
                input_val = input[0];
                input.remove(0);
            } else {
                println!("INPUT DEFAULTING TO 0!");
                input_val = 0;
            }
            if matches.opt_present("d") {
                println!("[{pc:03}] {:5} {:5}             -- {} -> {} at {}",
                         data.n[pc], data.n[pc + 1], input_val, input_val, data.n[pc + 1]);
            }
            let target = data.n[pc + 1];
            data.n[target as usize] = input_val;
            pc += 2;
        },
        4 => {
            // Add value to the output list
            let param1 = get_parameter(modifier_1, pc + 1, data);
            if matches.opt_present("d") {
                println!("[{pc:03}] {:5} {:5}             -- {} -> OUTPUT",
                         data.n[pc], data.n[pc + 1],
                         param1);
            }
            output.push(param1);
            pc += 2;
        },
        5 => {
            // jump-if-true
            let param1 = get_parameter(modifier_1, pc + 1, data);
            let param2 = get_parameter(modifier_2, pc + 2, data);
            if param1 != 0 {
                if matches.opt_present("d") {
                    println!("[{pc:03}] {:5} {:5} {:5}       -- {} != 0 -> PC to {}",
                             data.n[pc], data.n[pc + 1], data.n[pc + 2],
                             param1, param2);
                }
                pc = param2 as usize;
            } else {
                if matches.opt_present("d") {
                    println!("[{pc:03}] {:5} {:5} {:5}       -- {} == 0 -> NOOP",
                             data.n[pc], data.n[pc + 1], data.n[pc + 2],
                             param1);
                }
                pc += 3;
            }
        },
        6 => {
            // jump-if-false
            let param1 = get_parameter(modifier_1, pc + 1, data);
            let param2 = get_parameter(modifier_2, pc + 2, data);
            if param1 == 0 {
                if matches.opt_present("d") {
                    println!("[{pc:03}] {:5} {:5} {:5}       -- {} == 0 -> PC to {}",
                             data.n[pc], data.n[pc + 1], data.n[pc + 2],
                             param1, param2);
                }
                pc = param2 as usize;
            } else {
                if matches.opt_present("d") {
                    println!("[{pc:03}] {:5} {:5} {:5}       -- {} != 0 -> NOOP",
                             data.n[pc], data.n[pc + 1], data.n[pc + 2],
                             param1);
                }
                pc += 3;
            }
        },
        7 => {
            // less than
            let param1 = get_parameter(modifier_1, pc + 1, data);
            let param2 = get_parameter(modifier_2, pc + 2, data);
            let param3 = data.n[pc + 3];
            if param1 < param2 {
                data.n[param3 as usize] = 1;
                if matches.opt_present("d") {
                    println!("[{pc:03}] {:5} {:5} {:5} {:5} -- {} < {} -> 1 at {}",
                             data.n[pc], data.n[pc + 1], data.n[pc + 2], data.n[pc + 3],
                             param1, param2, param3);
                }
            } else {
                data.n[param3 as usize] = 0;
                if matches.opt_present("d") {
                    println!("[{pc:03}] {:5} {:5} {:5} {:5} -- {} >= {} -> 0 at {}",
                             data.n[pc], data.n[pc + 1], data.n[pc + 2], data.n[pc + 3],
                             param1, param2, param3);
                }
            }
            pc += 4;
        },
        8 => {
            // equals
            let param1 = get_parameter(modifier_1, pc + 1, data);
            let param2 = get_parameter(modifier_2, pc + 2, data);
            let param3 = data.n[pc + 3];
            if param1 == param2 {
                data.n[param3 as usize] = 1;
                if matches.opt_present("d") {
                    println!("[{pc:03}] {:5} {:5} {:5} {:5} -- {} == {} -> 1 at {}",
                             data.n[pc], data.n[pc + 1], data.n[pc + 2], data.n[pc + 3],
                             param1, param2, param3);
                }
            } else {
                data.n[param3 as usize] = 0;
                if matches.opt_present("d") {
                    println!("[{pc:03}] {:5} {:5} {:5} {:5} -- {} == {} -> 0 at {}",
                             data.n[pc], data.n[pc + 1], data.n[pc + 2], data.n[pc + 3],
                             param1, param2, param3);
                }
            }
            pc += 4;
        },
        _ => {
            println!("[{pc:03} {} -- Unexpected instruction!", data.n[pc]);
            process::exit(1);
        }
    }
    *pc_ref = pc;

    true
}


// The main code for part 1
fn run_part1(matches: &Matches, data: &mut Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let mut best_output = 0;
    let phases: Vec<i32> = vec![0, 1, 2, 3, 4];
    for digits in phases.iter().permutations(phases.len()).unique() {
        let mut last_output = 0;
        let mut output: Vec<i32>;
        for digit in &digits {
            let mut input: Vec<i32> = Vec::new();
            input.push(**digit);
            input.push(last_output);
            output = Vec::new();
            let mut pc: usize = 0;
            loop {
                let exec_cont = execute_intcode(&matches, &mut pc, data, &mut input, &mut output);
                if !exec_cont {
                    break;
                }
            }
            if matches.opt_present("d") {
                dbg!(&output);
            }
            last_output = output[0];
        }
        if last_output > best_output {
            best_output = last_output;
        }
    }
    // return the best output value possible
    best_output.into()
}


fn run_feedback_loop(matches: &Matches, data: &mut Input, digits: &Vec<&i32>) -> i64 {
    // Keep track of each amplifier's state, input, and output, separately
    let mut amp_pc:     Vec<usize>    = Vec::new();
    let mut amp_data:   Vec<Input>    = Vec::new();
    let mut amp_output: Vec<Vec<i32>> = Vec::new();
    let mut amp_input:  Vec<Vec<i32>> = Vec::new();
    let mut amp_state:  Vec<bool>     = Vec::new();

    // Initialize each of the amplifier state machines (A=0, ... E=4)
    for ix in 0..5 {
        amp_pc.push(0);
        amp_data.push(data.clone());
        amp_output.push(Vec::new());
        amp_input.push(Vec::new());
        amp_input[ix].push(*digits[ix]);
        amp_state.push(true);
    }
    // Seed amplifier A with the initial 0
    amp_input[0].push(0);

    let mut amp_ix = 0;
    loop {
        // Run instructions until either the program on the current
        // amplifier generated more output, or it halted
        let exec_cont = execute_intcode(
            &matches,
            &mut amp_pc[amp_ix],
            &mut amp_data[amp_ix],
            &mut amp_input[amp_ix],
            &mut amp_output[amp_ix]
        );
        // If it halted, we're done with all the amplifiers
        if !exec_cont {
            if matches.opt_present("d") {
                println!("Amplifier {amp_ix} halted.");
            }
            break;
        }
        // If there was new output, that is fed into the next
        // amplifier (A.. E, back to A)
        if amp_output[amp_ix].len() > 0 {
            let len = amp_output[amp_ix].len();
            let output = amp_output[amp_ix][len - 1];
            let next_amp = (amp_ix + 1) % 5;
            if matches.opt_present("d") {
                println!("Passing along {output} from amplifier {amp_ix} to {next_amp}");
            }

            // We have output, pass it along as input for the next amplifier
            amp_input[next_amp].push(output);
            amp_ix = next_amp;

            // Reset the output of the next amplifier
            amp_output[amp_ix].clear();
        }
    }

    // See what the most recent output for amplifier E was:
    let len = amp_output[4].len();
    return amp_output[4][len - 1].into()
}


// The main code for part 2
fn run_part2(matches: &Matches, data: &mut Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let phases: Vec<i32> = vec![5, 6, 7, 8, 9];
    let mut best_output = 0;
    for digits in phases.iter().permutations(phases.len()).unique() {
        if matches.opt_present("d") {
            dbg!(&digits);
        }
        let output = run_feedback_loop(matches, data, &digits);
        if matches.opt_present("d") {
            println!("--> {output}");
        }
        if output > best_output {
            best_output = output;
        }
    }
    best_output
}


fn run_part(matches: &Matches, part: u8, filename: String, expected: i64) -> bool {
    let mut data = parse_input(&matches, &filename);
    let answer: i64;
    let passed: bool;

    let now = Instant::now();
    match part {
        1 => answer = run_part1(&matches, &mut data),
        2 => answer = run_part2(&matches, &mut data),
        _ => answer = -1,
    }
    if answer == expected {
        passed = true;
        if matches.opt_present("v") {
            println!("Confirmed expected value from part {part}, filename '{filename}'");
        }
    } else {
        println!("Warning: Unexpected value from part {part}, filename '{filename}'");
        passed = false;
    }
    let duration = now.elapsed();
    println!("[Duration {:>7.1?}] Part {part}, filename '{filename}', answer: {answer}",
             duration);

    passed
}


// Main program, which parses the command line options, parses the
// file, and call the appropriate run_* functions with the parsed data
fn main() {
    let args: Vec<String> = env::args().collect();

    let matches = parse_options(&args);

    let mut answers: HashMap<&u8, HashMap<&str, i64>> = HashMap::new();
    answers.insert(&1, HashMap::from([
        ("sample1", 43210),
        ("sample2", 54321),
        ("sample3", 65210),
        ("input",   17790),
    ]));
    answers.insert(&2, HashMap::from([
        ("sample4", 139629729),
        ("sample5",     18216),
        ("input",    19384820),
    ]));

    if matches.opt_present("t") {
        let mut passed = 0;
        let mut failed = 0;
        let mut part: u8 = 0;

        if matches.opt_present("p") {
            // Run the tests for a specific part
            let part_str = matches.opt_str("p").unwrap();
            part = part_str.parse::<u8>().unwrap();
        }

        // Get the known parts, and sort them
        let mut t_parts: Vec<&u8> = answers.clone().into_keys().collect();
        t_parts.sort();

        for t_part in t_parts {
            if part != 0 && *t_part != part {
                continue;
            }

            // Get the known filenames, and sort them
            let mut t_filenames: Vec<&str> = answers[&t_part].clone().into_keys().collect();
            t_filenames.sort();

            for t_filename in t_filenames {
                let expected = answers[&t_part][t_filename];
                if run_part(&matches, *t_part, String::from(t_filename), expected) {
                    passed += 1;
                } else {
                    failed += 1;
                }
            }
        }
        println!("Test results: {passed} passed, {failed} failed.");
        return;
    }

    let filename = matches.opt_str("f").unwrap();
    let part_str = matches.opt_str("p").unwrap();
    let part = part_str.parse::<u8>().unwrap();

    let expected: i64;
    if answers.contains_key(&part) && answers[&part].contains_key(&filename.as_str()) {
        expected = answers[&part][&filename.as_str()];
    } else {
        expected = 0;
    }
    run_part(&matches, part, filename, expected);
}
