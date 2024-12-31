// -*- mode: rust; indent-tabs-mode: nil -*-
//
// Rust code for AoC program

// Useful while developing, allows focus on the bigger picture
#![allow(dead_code, unused_variables)]

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


// Putting the specifics of the different puzzle inputs in this
// struct, to make the various function signatures more generic.

#[derive(Debug)]
struct Input {
    p: HashMap<String, String>,
    m: HashMap<String, Vec<String>>,
}


// Read the given file and return two HashMaps:
// - one to map each object to whatever other object it is orbiting,
// - another to map each object to a list of 1 or more objects that
//   orbit it.
fn parse_input(matches: &Matches, filename: &String) -> Input {
    let content = match fs::read_to_string(&filename) {
        Ok(data) => data,
        Err(_) => {
            println!("File not found: '{filename}'. Aborting.");
            process::exit(1);
        }
    };

    let mut moons: HashMap<String, Vec<String>> = HashMap::new();
    let mut planet: HashMap<String, String> = HashMap::new();

    for line in content.lines() {
        let mut bits: Vec<&str> = Vec::new();
        for segment in line.trim().split(")") {
            bits.push(segment);
        }

        // The *.to_string() were vital to make this work
        let key = bits[0].to_string();
        let val = bits[1].to_string();

        // Find the list that this key points to, or insert a new one,
        // all in one go
        let list = moons.entry(key.clone()).or_insert(Vec::new());
        list.push(val.clone());

        planet.insert(val, key);
    }

    Input {
        m: moons,
        p: planet,
    }
}


// The main code for part 1
fn run_part1(matches: &Matches, data: &mut Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let mut queue: Vec<String> = Vec::new();
    let mut total = 0;
    let mut level = 0;

    queue.push(String::from("COM"));
    loop {
        let mut new_queue: Vec<String> = Vec::new();

        for planet in queue {
            if data.m.contains_key(&planet) {
                let orbiting: &Vec<String> = &data.m[&planet];
                for moon in orbiting {
                    if matches.opt_present("d") {
                        println!("{planet} -> {moon}");
                    }
                    total += level + 1;
                    new_queue.push(String::from(moon));
                }
            }
        }
        if new_queue.len() == 0 {
            break;
        }
        level += 1;
        queue = new_queue;
    }
    if matches.opt_present("d") {
        println!("Final level: {level}");
    }
    total
}


// The main code for part 2
fn run_part2(matches: &Matches, data: &mut Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let you_planet = &data.p[&String::from("YOU")];
    let san_planet = &data.p[&String::from("SAN")];

    // Found the planets we need
    if matches.opt_present("v") {
        println!("YOU are orbiting {you_planet}");
        println!("SAN is orbiting {san_planet}");
    }

    let mut orbits: HashMap<String,u32> = HashMap::new();

    let mut travel = you_planet.clone();
    let mut steps = 1;
    loop {
        let next_orbit = &data.p[&travel];
        orbits.insert(next_orbit.clone(), steps);
        if next_orbit == "COM" {
            break;
        }
        travel = next_orbit.to_string();
        steps += 1;
    }

    travel = san_planet.clone();
    steps = 1;
    loop {
        let next_orbit = &data.p[&travel];
        if orbits.contains_key(&next_orbit.clone()) {
            return (steps + orbits[next_orbit]).into();
        }
        travel = next_orbit.to_string();
        steps += 1;
    }
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
        ("sample", 42),
        ("input",  301100),
    ]));
    answers.insert(&2, HashMap::from([
        ("sample2", 4),
        ("input",   547),
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
