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
    p: String,
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
    Input { p: content.lines().next().unwrap().to_string() }
}


// The main code for part 1
fn run_part1(matches: &Matches, data: &Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let width: i32;
    let height: i32;
    if data.p.len() < 100 {
        width  = 3;
        height = 2;
    } else {
        width  = 25;
        height =  6;
    }

    let mut best_count_0 = -1;
    let mut best_count_1 = -1;
    let mut best_count_2 = -1;

    let mut ix: usize = 0;
    loop {
        if ix >= data.p.len() {
            break;
        }
        let mut count_0 = 0;
        let mut count_1 = 0;
        let mut count_2 = 0;
        for i in 0..width * height {
            let index: usize = ix + i as usize;
            match data.p.as_bytes()[index] - '0' as u8 {
                0 => count_0 += 1,
                1 => count_1 += 1,
                2 => count_2 += 1,
                _ => (),
            }
        }
        if matches.opt_present("d") {
            println!("{ix:3} {count_0}; {count_1}; {count_2}");
        }
        if best_count_0 == -1 || count_0 < best_count_0 {
            best_count_0 = count_0;
            best_count_1 = count_1;
            best_count_2 = count_2;
        }
        ix += (width * height) as usize;
    }

    best_count_1 * best_count_2
}


// The main code for part 2
fn run_part2(matches: &Matches, data: &Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }


    let width: i32;
    let height: i32;
    if data.p.len() < 100 {
        width  = 2;
        height = 2;
    } else {
        width  = 25;
        height =  6;
    }
    let mut frame: Vec<u8> = Vec::new();
    for _ in 0..width * height {
        frame.push(2);
    }

    let mut ix: usize = data.p.len() - (width * height) as usize;
    loop {
        for i in 0..width * height {
            let index: usize = ix + i as usize;
            let pixel = data.p.as_bytes()[index] - '0' as u8;
            if pixel == 2 {
                continue;
            }
            frame[i as usize] = pixel;
        }
        if ix == 0 {
            break;
        }
        ix -= (width * height) as usize;
    }
    let mut output: Vec<String> = Vec::new();
    for r in 0..height {
        let mut s: String = "".to_string();
        for c in 0..width {
            // I assume there's a better way for this
            s += &String::from(('0' as u8 + frame[(r * width + c) as usize]) as char);
        }
        output.push(s);
    }
    // Yeah, not going to do some kind of bitmap parsing here :)
    if matches.opt_present("v") {
        dbg!(&output);
    }

    0
}


fn run_part(matches: &Matches, part: u8, filename: String, expected: i64) -> bool {
    let data = parse_input(&matches, &filename);
    let answer: i64;
    let passed: bool;

    let now = Instant::now();
    match part {
        1 => answer = run_part1(&matches, &data),
        2 => answer = run_part2(&matches, &data),
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
        ("sample", 0),
        ("input",  0),
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
