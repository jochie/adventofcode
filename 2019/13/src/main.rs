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
use std::time;
use std::collections::HashMap; // HashMap::new()
use std::thread;

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

#[derive(Debug)]
struct Input {
    n: Vec<i64>,
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
    let mut lines: Vec<i64> = Vec::new();
    for line in content.lines() {
        for num in line.split(',') {
            lines.push(num.parse::<i64>().unwrap());
        }
    }
    Input { n: lines }
}


// Get parameter values, using the different modes
fn get_parameter(modifier: i64, pos: usize, relbase: usize, data: &mut Input) -> i64 {
    match modifier {
        0 => {
            // Position mode
            return data.n[data.n[pos] as usize];
        },
        1 => {
            // Immediate mode
            return data.n[pos];
        },
        2 => {
            // Relative mode
            return data.n[((relbase as i64) + data.n[pos]) as usize];
        },
        _ => {
            // Unexpected mode
            return 0;
        },
    }
}


// Return the location at which to store a new value ("addition",
// "multiplication", "input", "less than", "equals")
fn get_location(modifier: i64, param: usize, relbase: usize, data: &mut Input) -> usize {
    match modifier {
        0 => {
            return data.n[param].try_into().unwrap();
        },
        2 => {
            return (relbase as i64 + data.n[param]).try_into().unwrap();
        },
        _ => {
            println!("Unexpected modifier!");
            process::exit(1);
        },
    }
}


// Return true unless we encountered the 99 ("halt") opcode
fn execute_intcode(matches: &Matches, pc_ref: &mut usize, data: &mut Input, input: &mut Vec<i64>, output: &mut Vec<i64>, relbase: &mut usize) -> bool {
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
            let param1 = get_parameter(modifier_1, pc + 1, *relbase, data);
            let param2 = get_parameter(modifier_2, pc + 2, *relbase, data);
            let param3 = get_location(modifier_3, pc + 3, *relbase, data);
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
            let param1 = get_parameter(modifier_1, pc + 1, *relbase, data);
            let param2 = get_parameter(modifier_2, pc + 2, *relbase, data);
            let param3 = get_location(modifier_3, pc + 3, *relbase, data);
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
            let input_val: i64;
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
            let target = get_location(modifier_1, pc + 1, *relbase, data);
            data.n[target as usize] = input_val;
            pc += 2;
        },
        4 => {
            // Add value to the output list
            let param1 = get_parameter(modifier_1, pc + 1, *relbase, data);
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
            let param1 = get_parameter(modifier_1, pc + 1, *relbase, data);
            let param2 = get_parameter(modifier_2, pc + 2, *relbase, data);
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
            let param1 = get_parameter(modifier_1, pc + 1, *relbase, data);
            let param2 = get_parameter(modifier_2, pc + 2, *relbase, data);
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
            let param1 = get_parameter(modifier_1, pc + 1, *relbase, data);
            let param2 = get_parameter(modifier_2, pc + 2, *relbase, data);
            let param3 = get_location(modifier_3, pc + 3, *relbase, data);
            if param1 < param2 {
                data.n[param3] = 1;
                if matches.opt_present("d") {
                    println!("[{pc:03}] {:5} {:5} {:5} {:5} -- {} < {} -> 1 at {}",
                             data.n[pc], data.n[pc + 1], data.n[pc + 2], data.n[pc + 3],
                             param1, param2, param3);
                }
            } else {
                data.n[param3] = 0;
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
            let param1 = get_parameter(modifier_1, pc + 1, *relbase, data);
            let param2 = get_parameter(modifier_2, pc + 2, *relbase, data);
            let param3 = get_location(modifier_3, pc + 3, *relbase, data);
            if param1 == param2 {
                data.n[param3] = 1;
                if matches.opt_present("d") {
                    println!("[{pc:03}] {:5} {:5} {:5} {:5} -- {} == {} -> 1 at {}",
                             data.n[pc], data.n[pc + 1], data.n[pc + 2], data.n[pc + 3],
                             param1, param2, param3);
                }
            } else {
                data.n[param3] = 0;
                if matches.opt_present("d") {
                    println!("[{pc:03}] {:5} {:5} {:5} {:5} -- {} == {} -> 0 at {}",
                             data.n[pc], data.n[pc + 1], data.n[pc + 2], data.n[pc + 3],
                             param1, param2, param3);
                }
            }
            pc += 4;
        },
        9 => {
            let param1 = get_parameter(modifier_1, pc + 1, *relbase, data);

            if matches.opt_present("d") {
                println!("[{pc:03}] {:5} {:5}             -- {} -> RELBASE {}",
                         data.n[pc], data.n[pc + 1], param1, *relbase as i64 + param1);
            }
            *relbase = (*relbase as i64 + param1) as usize;
            pc += 2;
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

    // Extend the memory slots a bit
    for _ in 0..1000 {
        data.n.push(0);
    }
    let mut pc: usize = 0;
    let mut output: Vec<i64> = Vec::new();
    let mut input: Vec<i64> = Vec::new();
    let mut relbase: usize = 0;

    let mut x_min = 0;
    let mut x_max = 0;
    let mut y_min = 0;
    let mut y_max = 0;
    let mut tiles: Vec<u32> = vec![0, 0, 0, 0, 0];

    loop {
        let exec_cont = execute_intcode(&matches, &mut pc, data, &mut input, &mut output, &mut relbase);
        if output.len() == 3 {
            // Output comes in blocks of 3:
            // <x position>, <y-position>, <tile id>
            //
            // tile ids:
            // 0 - empty             (' ')
            // 1 - wall              ('|'?)
            // 2 - block             ('#'?)
            // 3 - horizontal paddle ('='?)
            // 4 - ball              ('O'?)
            if output[0] < x_min {
                x_min = output[0];
            }
            if output[0] > x_max {
                x_max = output[0];
            }
            if output[1] < y_min {
                y_min = output[1];
            }
            if output[1] > y_max {
                y_max = output[1];
            }
            match output[2] {
                0 | 1 | 2 | 3 | 4 => tiles[output[2] as usize] += 1,
                _ => (),
            }
            output.clear();
        }
        if !exec_cont {
            break;
        }
    }

    tiles[2].into()
}


fn display_layout(layout: &[[char; 38]; 21], score: i64, ball_x: i64, paddle_x: i64) {
    for y in 0..layout.len() {
        for x in 0..layout[y].len() {
            print!("{}", layout[y][x]);
        }
        println!();
    }
    println!("SCORE: {score}\nPaddle X: {paddle_x:2} <-> Ball X: {ball_x:2}");
}


// The main code for part 2
fn run_part2(matches: &Matches, data: &mut Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    // Change 'add' to 'mult'? Okay.
    data.n[0] = 2;

    // Extend the memory slots a bit
    for _ in 0..1000 {
        data.n.push(0);
    }
    let mut pc: usize = 0;
    let mut output: Vec<i64> = Vec::new();
    let mut input: Vec<i64> = Vec::new();
    let mut relbase: usize = 0;

    let mut layout: [[char; 38]; 21] = [[' '; 38]; 21];
    let mut score = 0;

    let mut ball_x = -1;
    let mut paddle_x = -1;

    loop {
        let mut do_refresh: bool = false;
        let exec_cont = execute_intcode(&matches, &mut pc, data, &mut input, &mut output, &mut relbase);
        if output.len() == 3 {
            // Output comes in blocks of 3:
            // <x position>, <y-position>, <tile id>
            //
            // tile ids:
            // 0 - empty             (' ')
            // 1 - wall              ('|'?)
            // 2 - block             ('#'?)
            // 3 - horizontal paddle ('='?)
            // 4 - ball              ('O'?)
            //
            // One exception is the "score" information which is passed along on x: -1, y: 0
            //
            if output[0] == -1 && output[1] == 0 {
                score = output[2];
                do_refresh = true;
            } else {
                match output[2] {
                    0 => {
                        layout[output[1] as usize][output[0] as usize] = ' ';
                    },
                    1 => {
                        layout[output[1] as usize][output[0] as usize] = '#';
                    },
                    2 => {
                        layout[output[1] as usize][output[0] as usize] = '@';
                    },
                    3 => {
                        layout[output[1] as usize][output[0] as usize] = '=';
                        paddle_x = output[0];
                        do_refresh = true;
                    },
                    4 => {
                        layout[output[1] as usize][output[0] as usize] = 'o';
                        ball_x = output[0];
                        do_refresh = true;
                    },
                    _ => (),
                }
            }
            output.clear();
        }
        if !exec_cont {
            break;
        }
        if do_refresh {
            if matches.opt_present("v") {
                clearscreen::clear().expect("Failed to clear the screen");
                display_layout(&layout, score, ball_x, paddle_x);
                thread::sleep(time::Duration::from_millis(50));
            }
        }
        // Implementing a naive "follow the ball" strategy
        if ball_x >= 0 && paddle_x >= 0 {
            input.clear();
            if ball_x == paddle_x {
                input.push(0);
            } else if ball_x < paddle_x {
                input.push(-1);
            } else {
                input.push(1);
            }
        }
    }

    score
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
        ("input",  228),
    ]));
    answers.insert(&2, HashMap::from([
        ("input",  10776),
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
