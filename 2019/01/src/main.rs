// -*- mode: rust; indent-tabs-mode: nil -*-
//
// Rust code for AoC program

extern crate getopts;

use getopts::Options;
use getopts::Matches;

use std::fs;            // fs::read_to_string()
use std::env;           // env::args()
use std::process;       // process::exit()
use std::time::Instant; // now()

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

    return matches;
}


// Read the given file and return a list of integers
fn parse_input(matches: &Matches, filename: &String) -> Vec<i32> {
    let mut lines: Vec<i32> = Vec::new();
    let contents = fs::read_to_string(&filename)
        .expect("Missing file");
    for line in contents.lines() {
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
    return lines;
}


// The main code for part 1
fn run_part1(matches: &Matches, numbers: &Vec<i32>) -> i64 {
    let mut part1_sum: i32 = 0;

    for num in numbers {
        part1_sum += num / 3 - 2;
    }
    if matches.opt_present("v") {
        println!("Part 1 - Sum of the fuel requirements: {part1_sum}");
    }
    return i64::from(part1_sum);
}


// The main code for part 2
fn run_part2(matches: &Matches, numbers: &Vec<i32>) -> i64 {
    let mut part2_sum: i64 = 0;

    for num in numbers {
        let mut fuel: i32 = num / 3 - 2;
        while fuel > 0 {
            part2_sum += i64::from(fuel);
            fuel = fuel / 3 - 2;
        }
    }
    if matches.opt_present("v") {
        println!("Part 2 - Sum of the fuel requirements: {part2_sum}");
    }
    return part2_sum;
}


// Main program, which parses the command line options, parses the
// file, and call the appropriate run_* functions with the parsed data
fn main() {
    let args: Vec<String> = env::args().collect();

    let matches = parse_options(&args);

    if !matches.opt_present("f") {
        println!("The input filename (-f/--filename) is required.");
        process::exit(1);
    }

    let answers: Vec<i64> = vec![ 3392373, 5085699 ];

    let filename = matches.opt_str("f").unwrap();
    let numbers: Vec<i32> = parse_input(&matches, &filename);

    if matches.opt_present("p") {
        let part = matches.opt_str("p").unwrap().parse::<i32>().unwrap();
        let answer: i64;
        let now = Instant::now();

        if part == 1 {
            answer = run_part1(&matches, &numbers);
            if answer == answers[0] {
                if matches.opt_present("v") {
                    println!("Confirmed expected value from part {part}, filename '{filename}'");
                }
            } else {
                println!("Warning: Unexpected value from part {part}, filename '{filename}'");
            }
        } else if part == 2 {
            answer = run_part2(&matches, &numbers);
            if answer == answers[1] {
                if matches.opt_present("v") {
                    println!("Confirmed expected value from part {part}, filename '{filename}'");
                }
            } else {
                println!("Warning: Unexpected value from part {part}, filename '{filename}'");
            }
        } else {
            answer = -1;
        }
        let duration = now.elapsed();
        println!("[Duration {:.2?}] Part {part}, filename '{filename}', answer: {answer}",
                 duration);
    }
}
