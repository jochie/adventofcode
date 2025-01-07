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

use regex::Regex;

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
    d: HashMap<String,(u32,Vec<(u32,String)>)>,
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
    let mut seen: HashMap<String,u32> = HashMap::new();
    let mut data: HashMap<String,(u32,Vec<(u32,String)>)> = HashMap::new();
    for line in content.lines() {
        let re = Regex::new(r"^(.*) => (\d+) ([A-Z]+)$").unwrap();
        let Some(caps) = re.captures(line.trim()) else {
            println!("MALFORMED LINE: {}", line.trim());
            continue;
        };
        let left: &str = &caps[1];
        let right_n: u32 = caps[2].parse().unwrap();
        let right_a: String = caps[3].to_string();
        let mut lhs: Vec<(u32,String)> = Vec::new();
        for segment in left.split(", ") {
            // println!("  Segment: {segment}");
            let re2 = Regex::new(r"^(\d+) ([A-Z]+)$").unwrap();
            let Some(caps2) = re2.captures(segment) else {
                println!("  MALFORMED SEGMENT: {segment}");
                continue;
            };
            lhs.push((
                caps2[1].parse().unwrap(),
                caps2[2].to_string()
            ));
        }
        if seen.contains_key(&right_a) {
            seen.insert(right_a.clone(), seen[&right_a] + 1);
        } else {
            seen.insert(right_a.clone(), 1);
        }
        // println!("'{left}' => '{right_n}' '{right_a}'");
        data.insert(right_a,(right_n,lhs));
    }
    // dbg!(&seen);
    Input { d: data }
}


fn determine_order(key: String, data: &Input, key_order: &mut HashMap<String,u64>) -> u64 {
    if key == "ORE" {
        return 0;
    }
    let mut max_depth = 0;
    for (repl_val, repl_key) in &data.d[&key].1 {
        let depth = determine_order(repl_key.to_string(), data, key_order);
        if depth > max_depth {
            max_depth = depth;
        }
    }
    key_order.insert(key, max_depth + 1);

    max_depth + 1
}


// The main code for part 1
fn run_part1(matches: &Matches, data: &Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let mut key_order: HashMap<String,u64> = HashMap::new();
    let depth = determine_order("FUEL".to_string(), data, &mut key_order);
    key_order.insert("FUEL".to_string(), depth);

    let mut keys: Vec<String> = key_order.clone().into_keys().collect();
    keys.sort_by(|b, a| { key_order[a].cmp(&key_order[b]) });

    let mut collected: HashMap<String,u64> = HashMap::new();
    collected.insert("FUEL".to_string(), 1);
    for key in &keys {
        let needed = collected[key];
        let amnt: u64 = data.d[key].0.into();
        let factor: u64;
        if needed < amnt {
            factor = 1;
        } else if needed % amnt == 0 {
            factor = needed / amnt;
        } else {
            factor = needed / amnt + 1;
        }
        if matches.opt_present("v") {
            println!("{key}: Needs {needed}, actual working value: {}",
                     factor * amnt);
        }
        for (repl_val, repl_key) in &data.d[key].1 {
            let mut val = (*repl_val as u64) * factor;
            if collected.contains_key(repl_key) {
                val += collected[repl_key];
            }
            collected.insert(repl_key.clone(), val);
        }
    }

    collected["ORE"].try_into().unwrap()
}


// The main code for part 2
fn run_part2(matches: &Matches, data: &Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    -2
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
        ("sample",       31),
        ("sample2",     165),
        ("sample3",   13312),
        ("sample4",  180697),
        ("sample5", 2210736),
        ("input",    857266),
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
