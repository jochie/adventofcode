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


#[derive(Debug)]
enum DIR {
    NORTH = 1,
    EAST  = 4,
    SOUTH = 2,
    WEST  = 3,
}


fn dump_map(map: &HashMap<(i64,i64),char>,
            row_cur: i64, col_cur: i64,
            row_min: &i64, row_max: &i64,
            col_min: &i64, col_max: &i64) {
    clearscreen::clear().expect("Failed to clear the screen");
    println!("MAP:");
    for row in *row_min..=*row_max {
        for col in *col_min..=*col_max {
            if row == 0 && col == 0 {
                print!("O");
            } else if row == row_cur && col == col_cur {
                print!("X");
            } else if map.contains_key(&(row, col)) {
                print!("{}", map[&(row, col)]);
            } else {
                print!(" ");
            }
        }
        println!();
    }
}


// Running the intcode, and using the result to implement a follow-
// the-wall algorithm to figure out what the "maze" looks
// like. Depending on the 'right' flag, it will search by following
// the wall to the right, or to the left.
fn search_maze(matches: &Matches, data: &mut Input, right: bool, map: &mut HashMap<(i64,i64),char>,
               row_min: &mut i64, row_max: &mut i64,
               col_min: &mut i64, col_max: &mut i64,
               row_oxy: &mut i64, col_oxy: &mut i64) {
    // Extend the memory slots a bit
    for _ in 0..1000 {
        data.n.push(0);
    }
    let mut pc: usize = 0;
    let mut output: Vec<i64> = Vec::new();
    let mut input: Vec<i64> = Vec::new();
    let mut relbase: usize = 0;

    // up, right, down, left
    let dir_trans: [i64; 4] = [ 1, 4, 2, 3 ];
    let dir_rel:   [(i64,i64); 4] = [(-1, 0), (0, 1), (1, 0), (0, -1)];

    let mut dir = 0;
    let mut row = 0;
    let mut col = 0;

    input.push(dir_trans[dir]);
    if matches.opt_present("v") {
        dump_map(&map, row, col, row_min, row_max, col_min, col_max);
        println!("Starting at ({row},{col}), trying to move {dir} ({})",
                 dir_trans[dir]);
    }
    loop {

        let exec_cont = execute_intcode(&matches, &mut pc, data, &mut input, &mut output, &mut relbase);
        if output.len() == 1 {
            if matches.opt_present("d") {
                dbg!(&output);
            }
            match output[0] {
                0 => {
                    // Nothing there, turn 90 degrees and try again?
                    map.insert((row + dir_rel[dir].0, col + dir_rel[dir].1), '#');
                    if row + dir_rel[dir].0 > *row_max {
                        *row_max = row + dir_rel[dir].0;
                    }
                    if row + dir_rel[dir].0 < *row_min {
                        *row_min = row + dir_rel[dir].0;
                    }
                    if col + dir_rel[dir].1 > *col_max {
                        *col_max = col + dir_rel[dir].1;
                    }
                    if col + dir_rel[dir].1 < *col_min {
                        *col_min = col + dir_rel[dir].1;
                    }
                    if right {
                        dir = (dir + 3) % 4;
                    } else {
                        dir = (dir + 1) % 4;
                    }
                    input.push(dir_trans[dir]);
                    output.clear();
                    if matches.opt_present("d") {
                        dump_map(&map, row, col, row_min, row_max, col_min, col_max);
                        println!("At ({row},{col}), trying to move {dir} ({})",
                                 dir_trans[dir]);
                    }
                },
                1 => {
                    row += dir_rel[dir].0;
                    col += dir_rel[dir].1;
                    if row < *row_min {
                        *row_min = row;
                    }
                    if row > *row_max {
                        *row_max = row;
                    }
                    if col < *col_min {
                        *col_min = col;
                    }
                    if col > *col_max {
                        *col_max = col;
                    }
                    map.insert((row, col), '.');
                    if right {
                        dir = (dir + 1) % 4;
                    } else {
                        dir = (dir + 3) % 4;
                    }
                    input.push(dir_trans[dir]);
                    output.clear();
                    if matches.opt_present("d") {
                        dump_map(&map, row, col, row_min, row_max, col_min, col_max);
                        println!("At ({row},{col}), trying to move {dir} ({})",
                                 dir_trans[dir]);
                    }
                },
                2 => {
                    // Success, and we found the oxygen generator
                    row += dir_rel[dir].0;
                    col += dir_rel[dir].1;
                    map.insert((row, col), '@');
                    if matches.opt_present("v") {
                        dump_map(&map, row, col, row_min, row_max, col_min, col_max);
                        println!("At ({row},{col}), found the target!");
                    }
                    *row_oxy = row;
                    *col_oxy = col;
                    break;
                },
                _ => {
                    println!("Unexpected output from the Intcode: {}",
                             output[0]);
                    break;
                }
            }
        }
        if !exec_cont {
            break;
        }
    }
}


// The main code for part 1
fn run_part1(matches: &Matches, data: &mut Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let mut copy: Input = data.clone();

    // visited spots
    let mut map: HashMap<(i64,i64),char> = HashMap::new();

    let mut row_min = 0;
    let mut row_max = 0;
    let mut col_min = 0;
    let mut col_max = 0;
    let mut row_oxy = 0;
    let mut col_oxy = 0;
    map.insert((0, 0), '.');

    if matches.opt_present("v") {
        println!("Follow the walls on the right:");
    }
    search_maze(matches, &mut copy, true, &mut map, &mut row_min, &mut row_max, &mut col_min, &mut col_max, &mut row_oxy, &mut col_oxy);
    if matches.opt_present("v") {
        println!("Found oxygen at {row_oxy},{col_oxy}");
    }

    // reset the Intcode
    copy = data.clone();
    if matches.opt_present("v") {
        println!("Follow the walls on the left:");
    }
    search_maze(matches, &mut copy, false, &mut map, &mut row_min, &mut row_max, &mut col_min, &mut col_max, &mut row_oxy, &mut col_oxy);
    if matches.opt_present("v") {
        println!("Found oxygen at {row_oxy},{col_oxy}");

        println!("End result:");
        dump_map(&map, 0, 0, &mut row_min, &mut row_max, &mut col_min, &mut col_max);
    }

    let mut seen: HashMap<(i64,i64),bool> = HashMap::new();
    seen.insert((0, 0), true);
    let mut checking: Vec<(i64,i64)> = vec![(0,0)];
    map.insert((0, 0), 'o');

    let dir_rel:   [(i64,i64); 4] = [(-1, 0), (0, 1), (1, 0), (0, -1)];

    let mut steps = 0;
    loop {
        let mut new_checking: Vec<(i64,i64)> = Vec::new();
        steps += 1;
        for (row, col) in &checking {
            for dir in 0..4 {
                let rel_row = dir_rel[dir].0;
                let rel_col = dir_rel[dir].1;
                let new_row = *row + rel_row;
                let new_col = *col + rel_col;
                if seen.contains_key(&(new_row,new_col)) {
                    continue;
                }
                match map[&(new_row,new_col)] {
                    '#' => {
                        continue;
                    },
                    '.' => {
                        new_checking.push((new_row,new_col));
                        seen.insert((new_row,new_col),true);
                        map.insert((new_row,new_col),'o');
                    },
                    '@' => {
                        // Ding, ding, ding, ding
                        if matches.opt_present("v") {
                            dump_map(&map, 0, 0, &mut row_min, &mut row_max, &mut col_min, &mut col_max);
                        }
                        return steps;
                    },
                    _ => todo!(),
                }
            }
        }
        if new_checking.len() == 0 {
            break;
        }

        if matches.opt_present("v") {
            dump_map(&map, 0, 0, &mut row_min, &mut row_max, &mut col_min, &mut col_max);
        }
        checking = new_checking;
    }

    0
}


// The main code for part 2
fn run_part2(matches: &Matches, data: &mut Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let mut copy: Input = data.clone();

    // visited spots
    let mut map: HashMap<(i64,i64),char> = HashMap::new();

    let mut row_min = 0;
    let mut row_max = 0;
    let mut col_min = 0;
    let mut col_max = 0;
    let mut row_oxy = 0;
    let mut col_oxy = 0;
    map.insert((0, 0), '.');

    if matches.opt_present("v") {
        println!("Follow the walls on the right:");
    }
    search_maze(matches, &mut copy, true, &mut map, &mut row_min, &mut row_max, &mut col_min, &mut col_max, &mut row_oxy, &mut col_oxy);
    if matches.opt_present("v") {
        println!("Found oxygen at {row_oxy},{col_oxy}");
    }

    // reset the Intcode
    copy = data.clone();
    if matches.opt_present("v") {
        println!("Follow the walls on the left:");
    }
    search_maze(matches, &mut copy, false, &mut map, &mut row_min, &mut row_max, &mut col_min, &mut col_max, &mut row_oxy, &mut col_oxy);
    if matches.opt_present("v") {
        println!("Found oxygen at {row_oxy},{col_oxy}");

        println!("End result:");
        dump_map(&map, 0, 0, &mut row_min, &mut row_max, &mut col_min, &mut col_max);
    }

    let mut seen: HashMap<(i64,i64),bool> = HashMap::new();
    seen.insert((row_oxy,col_oxy), true);
    let mut checking: Vec<(i64,i64)> = vec![(row_oxy,col_oxy)];
    map.insert((row_oxy,col_oxy), 'o');

    let dir_rel:   [(i64,i64); 4] = [(-1, 0), (0, 1), (1, 0), (0, -1)];

    let mut steps = 0;
    loop {
        let mut new_checking: Vec<(i64,i64)> = Vec::new();

        for (row, col) in &checking {
            for dir in 0..4 {
                let rel_row = dir_rel[dir].0;
                let rel_col = dir_rel[dir].1;
                let new_row = *row + rel_row;
                let new_col = *col + rel_col;
                if seen.contains_key(&(new_row,new_col)) {
                    continue;
                }
                match map[&(new_row,new_col)] {
                    '#' => {
                        continue;
                    },
                    '.' => {
                        new_checking.push((new_row,new_col));
                        seen.insert((new_row,new_col),true);
                        map.insert((new_row,new_col),'o');
                    },
                    '@' => {
                        // Ding, ding, ding, ding
                        if matches.opt_present("v") {
                            dump_map(&map, 0, 0, &mut row_min, &mut row_max, &mut col_min, &mut col_max);
                        }
                        return steps;
                    },
                    _ => todo!(),
                }
            }
        }
        if new_checking.len() == 0 {
            break;
        }

        if matches.opt_present("d") {
            dump_map(&map, 0, 0, &mut row_min, &mut row_max, &mut col_min, &mut col_max);
        }
        steps += 1;
        checking = new_checking;
    }

    steps
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
        ("input",  240),
    ]));
    answers.insert(&2, HashMap::from([
        ("input",  322),
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
