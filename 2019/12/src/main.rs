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
    b: Vec<(i32,i32,i32,i32,i32,i32)>,
}


// For this day I'm returning different result types for Part 1 and 2,
// so I jumped through a few hoops to accommodate this

#[derive(Debug, Clone, PartialEq)]
enum Answer {
    P1(Vec<i32>),
    P2(i64),
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
    let mut bodies: Vec<(i32,i32,i32,i32,i32,i32)> = Vec::new();
    for line in content.lines() {
        let re = Regex::new(r"^<x=(-?\d+), y=(-?\d+), z=(-?\d+)>$").unwrap();
        let Some(caps) = re.captures(line.trim()) else {
            println!("MALFORMED LINE: {}", line.trim());
            continue;
        };
        // The matches look like signed integers, let's extract those
        // values now
        let x: i32 = caps[1].parse().unwrap();
        let y: i32 = caps[2].parse().unwrap();
        let z: i32 = caps[3].parse().unwrap();
        bodies.push(( x, y, z, 0, 0, 0 ));
        // dbg!(caps);
    }
    Input { b: bodies }
}


// The main code for part 1
fn run_part1(matches: &Matches, data: &mut Input) -> Answer {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let mut energy_list: Vec<i32> = Vec::new();

    let mut steps = 0;
    loop {
        for ix in 0..data.b.len() {
            let mut x_diff: i32 = 0;
            let mut y_diff: i32 = 0;
            let mut z_diff: i32 = 0;

            for iy in 0..data.b.len() {
                if ix == iy {
                    continue;
                }
                if data.b[ix].0 < data.b[iy].0 {
                    x_diff += 1;
                } else if data.b[ix].0 > data.b[iy].0 {
                    x_diff -= 1;
                }
                if data.b[ix].1 < data.b[iy].1 {
                    y_diff += 1;
                } else if data.b[ix].1 > data.b[iy].1 {
                    y_diff -= 1;
                }
                if data.b[ix].2 < data.b[iy].2 {
                    z_diff += 1;
                } else if data.b[ix].2 > data.b[iy].2 {
                    z_diff -= 1;
                }
            }
            data.b[ix].3 += x_diff;
            data.b[ix].4 += y_diff;
            data.b[ix].5 += z_diff;
        }
        for ix in 0..data.b.len() {
            data.b[ix].0 += data.b[ix].3;
            data.b[ix].1 += data.b[ix].4;
            data.b[ix].2 += data.b[ix].5;
        }
        steps += 1;

        if matches.opt_present("d") {
            println!("Situation after {steps} step(s):");
            for ix in 0..data.b.len() {
                println!("pos=<x={:2}, y={:2}, z={:2}>, vel=<x={:2}, y={:2}, z={:2}>",
                         data.b[ix].0,
                         data.b[ix].1,
                         data.b[ix].2,
                         data.b[ix].3,
                         data.b[ix].4,
                         data.b[ix].5);
            }
            println!();
        }

        if steps == 10 || steps == 100 || steps == 1000 {
            let mut energy = 0;

            for ix in 0..data.b.len() {
                let pot = data.b[ix].0.abs() + data.b[ix].1.abs() + data.b[ix].2.abs();
                let kin = data.b[ix].3.abs() + data.b[ix].4.abs() + data.b[ix].5.abs();

                energy += pot * kin;
            }
            if matches.opt_present("v") {
                println!("Total energy in the system after {steps} steps: {energy}");
            }
            energy_list.push(energy);
            if energy_list.len() == 3 {
                break;
            }
        }
    }


    Answer::P1(energy_list)
}


fn gcd(m_orig: i64, n_orig: i64) -> i64 {
    let mut m = m_orig;
    let mut n = n_orig;

    while m != 0 {
        let tmp = m;
        m = n % m;
        n = tmp;
    }
    n
}


fn lcm(m: i64, n: i64) -> i64 {
    return m / gcd(m, n) * n;
}


// The main code for part 2
fn run_part2(matches: &Matches, data: &mut Input) -> Answer {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let mut x_dupes: HashMap<(i32,i32, i32,i32, i32,i32, i32,i32),u32> = HashMap::new();
    let mut y_dupes: HashMap<(i32,i32, i32,i32, i32,i32, i32,i32),u32> = HashMap::new();
    let mut z_dupes: HashMap<(i32,i32, i32,i32, i32,i32, i32,i32),u32> = HashMap::new();

    x_dupes.insert((data.b[0].0, data.b[0].3, data.b[1].0, data.b[1].3,
                    data.b[2].0, data.b[2].3, data.b[3].0, data.b[3].3), 0);
    y_dupes.insert((data.b[0].1, data.b[0].4, data.b[1].1, data.b[1].4,
                    data.b[2].1, data.b[2].4, data.b[3].1, data.b[3].4), 0);
    z_dupes.insert((data.b[0].2, data.b[0].5, data.b[1].2, data.b[1].5,
                    data.b[2].2, data.b[2].5, data.b[3].2, data.b[3].5), 0);

    let mut dupes_found: Vec<(bool, u32)> = vec![(false, 0), (false, 0), (false, 0)];
    let mut dupes_found_total = 0;
    let mut steps = 0;

    loop {
        for ix in 0..data.b.len() {
            let mut x_diff: i32 = 0;
            let mut y_diff: i32 = 0;
            let mut z_diff: i32 = 0;

            for iy in 0..data.b.len() {
                if ix == iy {
                    continue;
                }
                if data.b[ix].0 < data.b[iy].0 {
                    x_diff += 1;
                } else if data.b[ix].0 > data.b[iy].0 {
                    x_diff -= 1;
                }
                if data.b[ix].1 < data.b[iy].1 {
                    y_diff += 1;
                 } else if data.b[ix].1 > data.b[iy].1 {
                    y_diff -= 1;
                }
                if data.b[ix].2 < data.b[iy].2 {
                    z_diff += 1;
                } else if data.b[ix].2 > data.b[iy].2 {
                    z_diff -= 1;
                }
            }
            data.b[ix].3 += x_diff;
            data.b[ix].4 += y_diff;
            data.b[ix].5 += z_diff;
        }
        for ix in 0..data.b.len() {
            data.b[ix].0 += data.b[ix].3;
            data.b[ix].1 += data.b[ix].4;
            data.b[ix].2 += data.b[ix].5;
        }
        steps += 1;

        if matches.opt_present("d") {
            println!("Situation after {steps} step(s):");
            for ix in 0..data.b.len() {
                println!("pos=<x={:2}, y={:2}, z={:2}>, vel=<x={:2}, y={:2}, z={:2}>",
                         data.b[ix].0,
                         data.b[ix].1,
                         data.b[ix].2,
                         data.b[ix].3,
                         data.b[ix].4,
                         data.b[ix].5);
            }
            println!();
        }

        let x_key = (data.b[0].0, data.b[0].3,
                     data.b[1].0, data.b[1].3,
                     data.b[2].0, data.b[2].3,
                     data.b[3].0, data.b[3].3);
        let y_key = (data.b[0].1, data.b[0].4,
                     data.b[1].1, data.b[1].4,
                     data.b[2].1, data.b[2].4,
                     data.b[3].1, data.b[3].4);
        let z_key = (data.b[0].2, data.b[0].5,
                     data.b[1].2, data.b[1].5,
                     data.b[2].2, data.b[2].5,
                     data.b[3].2, data.b[3].5);

        if x_dupes.contains_key(&x_key) {
            if !dupes_found[0].0 {
                let prev_steps = x_dupes[&x_key];

                if matches.opt_present("v") {
                    println!("Revisiting X position/velocity at step {steps}, previously {} -- {}",
                             prev_steps, steps - prev_steps);
                }
                dupes_found[0] = (true, steps - prev_steps);
                dupes_found_total += 1;
            }
            x_dupes.insert(x_key, steps);
        } else {
            x_dupes.insert(x_key, steps);
        }
        if y_dupes.contains_key(&y_key) {
            if !dupes_found[1].0 {
                let prev_steps = y_dupes[&y_key];

                if matches.opt_present("v") {
                    println!("Revisiting Y position/velocity at step {steps}, previously {} -- {}",
                             prev_steps, steps - prev_steps);
                }
                dupes_found[1] = (true, steps - prev_steps);
                dupes_found_total += 1;
            }
            y_dupes.insert(y_key, steps);
        } else {
            y_dupes.insert(y_key, steps);
        }
        if z_dupes.contains_key(&z_key) {
            if !dupes_found[2].0 {
                let prev_steps = z_dupes[&z_key];

                if matches.opt_present("v") {
                    println!("Revisiting Z position/velocity at step {steps}, previously {} -- {}",
                             prev_steps, steps - prev_steps);
                }
                dupes_found[2] = (true, steps - prev_steps);
                dupes_found_total += 1;
            }
            z_dupes.insert(z_key, steps);
        } else {
            z_dupes.insert(z_key, steps);
        }

        if dupes_found_total == 3 {
            break;
        }

        // dbg!(&p_dupes);
        if steps == 50_000_000 {
            break;
        }
    }

    if matches.opt_present("d") {
        dbg!(&dupes_found);
    }

    let lcm_xyz = lcm(lcm(dupes_found[0].1 as i64,
                          dupes_found[1].1 as i64),
                      dupes_found[2].1 as i64);
    if matches.opt_present("v") {
        println!("lcm(lcm(x, y), z) = {lcm_xyz}");
    }

    Answer::P2(lcm_xyz)
}


fn run_part(matches: &Matches, part: u8, filename: &String, expected: Answer) -> bool {
    let mut data = parse_input(&matches, &filename);
    let answer: Answer;
    let passed: bool;

    let now = Instant::now();
    match part {
        1 => answer = run_part1(&matches, &mut data),
        2 => answer = run_part2(&matches, &mut data),
        _ => answer = Answer::P2(-1),
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
    println!("[Duration {:>7.1?}] Part {part}, filename '{filename}', answer: {answer:?}",
             duration);

    passed
}


// Main program, which parses the command line options, parses the
// file, and call the appropriate run_* functions with the parsed data
fn main() {
    let args: Vec<String> = env::args().collect();

    let matches = parse_options(&args);

    let mut answers: HashMap<&u8, HashMap<&str, Answer>> = HashMap::new();
    answers.insert(&1, HashMap::from([
        ("sample",  Answer::P1(vec![179,  293,   183])),
        ("sample2", Answer::P1(vec![706, 1940, 14645])),
        ("input",   Answer::P1(vec![887,  799, 13500])),
    ]));
    answers.insert(&2, HashMap::from([
        ("sample",  Answer::P2(2772)),
        ("sample2", Answer::P2(4686774924)),
        ("input",   Answer::P2(278013787106916)),
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
                let expected = &answers[&t_part][t_filename];
                if run_part(&matches, *t_part, &String::from(t_filename), expected.clone()) {
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

    let expected: &Answer;
    if answers.contains_key(&part) && answers[&part].contains_key(&filename.as_str()) {
        expected = &answers[&part][&filename.as_str()];
    } else {
        expected = &Answer::P2(0);
    }
    run_part(&matches, part, &filename, expected.clone());
}
