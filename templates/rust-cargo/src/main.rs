// -*- mode: rust; indent-tabs-mode: nil -*-
//
// Rust code for AoC program

extern crate getopts;

use getopts::Options;
use getopts::Matches;

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


// Read the given file and return a list of integers
fn parse_input(matches: &Matches, filename: &String) -> Vec<i32> {
    let mut lines: Vec<i32> = Vec::new();
    let content = match fs::read_to_string(&filename) {
        Ok(data) => data,
        Err(_) => {
            println!("File not found: '{filename}'. Aborting.");
            process::exit(1);
        }
    };
    for line in content.lines() {
        let num: i32 = match line.trim().parse() {
            Ok(num) => {
                if matches.opt_present("d") {
                    println!("LINE: {num}>");
                }
                num
            },
            Err(_) => {
                println!("LINE MALFORMED <{line}>");
                continue;
            }
        };
        lines.push(num);
    }

    lines
}


// The main code for part 1
fn run_part1(matches: &Matches, numbers: &Vec<i32>) -> i64 {
    if matches.opt_present("d") {
        dbg!(&numbers);
    }

    -1
}


// The main code for part 2
fn run_part2(matches: &Matches, numbers: &Vec<i32>) -> i64 {
    if matches.opt_present("d") {
        dbg!(&numbers);
    }

    -2
}


// Main program, which parses the command line options, parses the
// file, and call the appropriate run_* functions with the parsed data
fn main() {
    let args: Vec<String> = env::args().collect();

    let matches = parse_options(&args);

    let mut answers: HashMap<&u8, HashMap<&str, i64>> = HashMap::new();
    answers.insert(&1, HashMap::from([
        ("sample",  0),
        ("input",   0),
    ]));
    answers.insert(&2, HashMap::from([
        ("sample", 0),
        ("input",  0),
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
                let numbers = parse_input(&matches, &String::from(t_filename));

                let answer: i64;
                let now = Instant::now();

                // Still hardcoding which function to call
                match t_part {
                    1 => answer = run_part1(&matches, &numbers),
                    2 => answer = run_part2(&matches, &numbers),
                    _ => answer = -1,
                }
                if answer == answers[&t_part][t_filename] {
                    if matches.opt_present("v") {
                        println!("Confirmed expected value from part {t_part}, filename '{t_filename}'");
                    }
                    passed += 1;
                } else {
                    println!("Warning: Unexpected value from part {t_part}, filename '{t_filename}'");
                    failed += 1;
                }
                let duration = now.elapsed();
                println!("[Duration {:7.1?}] Part {t_part}, filename '{t_filename}', answer: {answer}",
                         duration);
            }
        }
        println!("Test results: {passed} passed, {failed} failed.");
        return;
    }
    let filename = matches.opt_str("f").unwrap();
    let numbers = parse_input(&matches, &filename);

    let part_str = matches.opt_str("p").unwrap();
    let part = part_str.parse::<u8>().unwrap();
    let answer: i64;
    let now = Instant::now();

    match part {
        1 => answer = run_part1(&matches, &numbers),
        2 => answer = run_part2(&matches, &numbers),
        _ => {
            println!("Error: Unexpected part: {part}.");
            return;
        }
    }
    if answers.contains_key(&part) && answers[&part].contains_key(&filename.as_str()) {
        if answer == answers[&part][&filename.as_str()] {
            if matches.opt_present("v") {
                println!("Confirmed expected value from part {part}, filename '{filename}'");
            }
        } else {
            println!("Warning: Unexpected value from part {part}, filename '{filename}'");
        }
    }
    let duration = now.elapsed();
    println!("[Duration {:.1?}] Part {part}, filename '{filename}', answer: {answer}",
             duration);
}
