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

#[derive(Debug)]
struct Input {
    s: String,
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
    let line = content.lines().next().unwrap();
    Input { s: line.to_string() }
}


fn apply_fft(bytes: &mut Vec<u8>) {
    let mut new_bytes: Vec<i64> = Vec::new();

    // dbg!(&bytes);

    // Pattern
    let pattern = [ 0, 1, 0, -1 ];

    // dbg!(&pattern);
    for i in 0..bytes.len() {
        // Generate the pattern for digit J
        let mut real_pattern: Vec<i64> = Vec::new();
        for j in 0..4 {
            for k in 0..=i {
                real_pattern.push(pattern[j]);
            }
        }
        // Shift the pattern around by removing the first one, and
        // add it to the end
        let first = real_pattern.remove(0);
        real_pattern.push(first);
        // dbg!(&real_pattern);

        let mut sum: i64 = 0;

        let mut ix = 0;
        for j in 0..bytes.len() {
            sum += bytes[j] as i64 * real_pattern[ix];
            ix = (ix + 1) % real_pattern.len();
        }
        new_bytes.push(sum.abs() % 10);
    }
    // dbg!(&new_bytes);
    for i in 0..bytes.len() {
        bytes[i] = new_bytes[i].try_into().unwrap();
    }
}


// The main code for part 1
fn run_part1(matches: &Matches, data: &Input) -> String {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let len = data.s.len();
    let mut bytes: Vec<u8> = Vec::new();
    for i in 0..len {
        bytes.push(data.s.as_bytes()[i] - '0' as u8);
    }

    for i in 0..100 {
        apply_fft(&mut bytes);
    }

    let mut result: String = String::from("00000000");
    for i in 0..8 {
        unsafe {
            result.as_bytes_mut()[i] = bytes[i] + '0' as u8;
        }
    }
    if matches.opt_present("v") {
        println!("Result: {result}");
    }

    result
}


// The main code for part 2
fn run_part2(matches: &Matches, data: &Input) -> String {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let len = data.s.len();
    let mut bytes: Vec<u8> = Vec::new();
    for i in 0..len {
        bytes.push(data.s.as_bytes()[i] - '0' as u8);
    }

    for i in 0..10000 {
        apply_fft(&mut bytes);
    }

    let mut result: String = String::from("00000000");
    for i in 0..8 {
        unsafe {
            result.as_bytes_mut()[i] = bytes[i] + '0' as u8;
        }
    }
    if matches.opt_present("v") {
        println!("Result: {result}");
    }

    result
}


fn run_part(matches: &Matches, part: u8, filename: String, expected: String) -> bool {
    let data = parse_input(&matches, &filename);
    let answer: String;
    let passed: bool;

    let now = Instant::now();
    match part {
        1 => answer = run_part1(&matches, &data),
        2 => answer = run_part2(&matches, &data),
        _ => answer = String::from(""),
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

    let mut answers: HashMap<&u8, HashMap<&str, String>> = HashMap::new();
    answers.insert(&1, HashMap::from([
        ("sample", String::from("23845678")),
        ("input",  String::from("58672132")),
    ]));
    answers.insert(&2, HashMap::from([
        ("sample", String::from("")),
        ("input",  String::from("")),
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
                let expected = answers[&t_part][t_filename].clone();
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

    let expected: String;
    if answers.contains_key(&part) && answers[&part].contains_key(&filename.as_str()) {
        expected = answers[&part][&filename.as_str()].clone();
    } else {
        expected = String::from("");
    }
    run_part(&matches, part, filename, expected);
}
