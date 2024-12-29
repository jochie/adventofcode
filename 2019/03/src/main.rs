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
fn parse_input(matches: &Matches, filename: &String) -> (Vec<String>, Vec<String>) {

    let content = fs::read_to_string(&filename)
        .expect("Missing file");

    let mut lines = content.lines();
    let mut wire1: Vec<String> = Vec::new();

    for segment in lines.next().unwrap().split(",") {
        if matches.opt_present("d") {
            println!("Wire1: {segment}");
        }
        wire1.push(String::from(segment));
    }

    let mut wire2: Vec<String> = Vec::new();
    for segment in lines.next().unwrap().split(",") {
        if matches.opt_present("d") {
            println!("Wire2: {segment}");
        }
        wire2.push(String::from(segment));
    }

    (wire1, wire2)
}

#[derive(Debug)]
struct Line {
    ix: i64,
    r0: i64,
    c0: i64,
    r1: i64,
    c1: i64,
    s0: i64,
    s1: i64,
}

// Break the directions down into a list of horizontal and vertical lines
fn analyze_directions(matches: &Matches, wire: &Vec<String>) -> (Vec<Line>, Vec<Line>) {
    let mut c_row = 0;
    let mut c_col = 0;
    let mut steps = 0;

    let mut h_lines: Vec<Line> = Vec::new();
    let mut v_lines: Vec<Line> = Vec::new();


    let mut ix = 0;
    for dir in wire {
        let val = dir[1..].parse::<i64>().unwrap();
        match dir.as_bytes()[0] {
            b'U' => {
                if matches.opt_present("d") {
                    println!("Wire 1: Moving up {val} positions.");
                }
                v_lines.push(Line {
                    ix: ix,
                    r0: c_row - val, c0: c_col, s0: steps + val,
                    r1: c_row,       c1: c_col, s1: steps,

                });
                c_row -= val;
                steps += val;
            },
            b'D' => {
                if matches.opt_present("d") {
                    println!("Wire 1: Moving down {val} positions.");
                }
                v_lines.push(Line {
                    ix: ix,
                    r0: c_row,       c0: c_col, s0: steps,
                    r1: c_row + val, c1: c_col, s1: steps + val,
                });
                c_row += val;
                steps += val;
            },
            b'L' => {
                if matches.opt_present("d") {
                    println!("Wire 1: Moving left {val} positions.");
                }
                h_lines.push(Line {
                    ix: ix,
                    r0: c_row,       c0: c_col - val, s0: steps + val,
                    r1: c_row,       c1: c_col,       s1: steps,
                });
                c_col -= val;
                steps += val;
            },
            b'R' => {
                if matches.opt_present("d") {
                    println!("Wire 1: Moving right {val} positions.");
                }
                h_lines.push(Line {
                    ix: ix,
                    r0: c_row,       c0: c_col,       s0: steps,
                    r1: c_row,       c1: c_col + val, s1: steps + val,
                });
                c_col += val;
                steps += val;
            },
            _ => {
                println!("Wire 1: Unexpected direction: {dir}");
                process::exit(1);
            }
        }
        ix += 1;
    }

    (h_lines, v_lines)
}

// The main code for part 1
fn run_part1(matches: &Matches, wire1: &Vec<String>, wire2: &Vec<String>) -> i64 {
    if matches.opt_present("d") {
        dbg!(&wire1);
        dbg!(&wire2);
    }

    let mut intersection_distance = -1;

    let (h_lines, v_lines) = analyze_directions(&matches, &wire1);

    if matches.opt_present("d") {
        dbg!(&h_lines);
        dbg!(&v_lines);
    }

    // Reset the starting point:
    let mut c_row = 0;
    let mut c_col = 0;

    for dir in wire2 {
        let val = dir[1..].parse::<i64>().unwrap();
        match dir.as_bytes()[0] {
            b'U' => {
                if matches.opt_present("d") {
                    println!("Wire 2: Moving up {val} positions.");
                }

                // Compare to all horizontal lines
                //
                //                 c_col
                //                 + c_row - val
                //                 |
                // (r0,c0) +-------+-+ (r1,c1)
                //                 |
                //                 + c_row

                for line in &h_lines {
                    if c_row - val <= line.r0 && line.r0 <= c_row {
                        if line.c0 <= c_col && c_col <= line.c1 {
                            let new_distance = line.r0.abs() + c_col.abs();
                            if new_distance > 0 && intersection_distance == -1 || new_distance < intersection_distance {
                                if matches.opt_present("v") {
                                    println!("Intersection found at {},{}",
                                             line.r0, c_col);
                                }
                                intersection_distance = new_distance;
                            }
                        }
                    }
                }
                c_row -= val;
            },
            b'D' => {
                if matches.opt_present("d") {
                    println!("Wire 2: Moving down {val} positions.");
                }
                for line in &h_lines {
                    if c_row <= line.r0 && line.r0 <= c_row + val {
                        if line.c0 <= c_col && c_col <= line.c1 {
                            let new_distance = line.r0.abs() + c_col.abs();
                            if new_distance > 0 && (intersection_distance == -1 || new_distance < intersection_distance) {
                                if matches.opt_present("v") {
                                    println!("Intersection found at {},{}",
                                             line.r0, c_col);
                                }
                                intersection_distance = new_distance;
                            }
                        }
                    }
                }
                c_row += val;
            },
            b'L' => {
                if matches.opt_present("d") {
                    println!("Wire 2: Moving left {val} positions.");
                }
                for line in &v_lines {
                    if c_col - val <= line.c0 && line.c0 <= c_col {
                        if line.r0 <= c_row && c_row <= line.r1 {
                            let new_distance = line.c0.abs() + c_row.abs();
                            if new_distance > 0 && (intersection_distance == -1 || new_distance < intersection_distance) {
                                if matches.opt_present("v") {
                                    println!("Intersection found at {},{}",
                                             c_row, line.c0);
                                }
                                intersection_distance = new_distance;
                            }
                        }
                    }
                }
                c_col -= val;
            },
            b'R' => {
                if matches.opt_present("d") {
                    println!("Wire 2: Moving right {val} positions.");
                }
                for line in &v_lines {
                    if c_col <= line.c0 && line.c0 <= c_col + val {
                        if line.r0 <= c_row && c_row <= line.r1 {
                            let new_distance = line.c0.abs() + c_row.abs();
                            if new_distance > 0 && (intersection_distance == -1 || new_distance < intersection_distance) {
                                if matches.opt_present("v") {
                                    println!("Intersection found at {},{}",
                                             c_row, line.c0);
                                }
                                intersection_distance = new_distance;
                            }
                        }
                    }
                }
                c_col += val;
            },
            _ => {
                println!("Wire 2: Unexpected direction: {dir}");
                process::exit(1);
            }
        }
    }

    intersection_distance
}


// The main code for part 2
fn run_part2(matches: &Matches, wire1: &Vec<String>, wire2: &Vec<String>) -> i64 {
    if matches.opt_present("d") {
        dbg!(&wire1);
        dbg!(&wire2);
    }

    let mut intersection_steps = -1;

    let (h_lines, v_lines) = analyze_directions(&matches, &wire1);

    if matches.opt_present("d") {
        dbg!(&h_lines);
        dbg!(&v_lines);
    }

    // Reset the starting point:
    let mut c_row = 0;
    let mut c_col = 0;
    let mut steps = 0;

    for dir in wire2 {
        let val = dir[1..].parse::<i64>().unwrap();
        match dir.as_bytes()[0] {
            b'U' => {
                if matches.opt_present("d") {
                    println!("Wire 2: Moving up {val} positions.");
                }
                // Compare to all horizontal lines
                //
                //                    c_col
                //                    + c_row - val
                //                    |
                // (r0,c0,s0) +-------+-+ (r1,c1,s1)
                //                    |
                //                    + c_row
                //

                for line in &h_lines {
                    if c_row - val <= line.r0 && line.r0 <= c_row {
                        if line.c0 <= c_col && c_col <= line.c1 {
                            let w1_steps;
                            if line.s0 < line.s1 {
                                w1_steps = line.s0 + (c_col - line.c0).abs();
                            } else {
                                w1_steps = line.s1 + (c_col - line.c1).abs();
                            }
                            let w2_steps = steps + (line.r0 - c_row).abs();
                            let new_steps = w1_steps + w2_steps;

                            if new_steps > 0 && intersection_steps == -1 || new_steps < intersection_steps {
                                if matches.opt_present("v") {
                                    println!("Intersection found at {},{}",
                                             line.r0, c_col);
                                    println!("Wire 1 segment: {}, steps {w1_steps}; Wire 2 steps: {w2_steps} => {new_steps}",
                                             line.ix);
                                }
                                intersection_steps = new_steps;
                            }
                        }
                    }
                }
                c_row -= val;
                steps += val;
            },
            b'D' => {
                if matches.opt_present("d") {
                    println!("Wire 2: Moving down {val} positions.");
                }
                for line in &h_lines {
                    if c_row <= line.r0 && line.r0 <= c_row + val {
                        if line.c0 <= c_col && c_col <= line.c1 {
                            let w1_steps;
                            if line.s0 < line.s1 {
                                w1_steps = line.s0 + (c_col - line.c0).abs();
                            } else {
                                w1_steps = line.s1 + (c_col - line.c1).abs();
                            }
                            let w2_steps = steps + (line.r0 - c_row).abs();
                            let new_steps = w1_steps + w2_steps;

                            if new_steps > 0 && (intersection_steps == -1 || new_steps < intersection_steps) {
                                if matches.opt_present("v") {
                                    println!("Intersection found at {},{}",
                                             line.r0, c_col);
                                    println!("Wire 1 segment: {}, steps {w1_steps}; Wire 2 steps: {w2_steps} => {new_steps}",
                                             line.ix);
                                }
                                intersection_steps = new_steps;
                            }
                        }
                    }
                }
                c_row += val;
                steps += val;
            },
            b'L' => {
                // Compare to all horizontal lines
                //
                //                    + (r1,c1,s1)
                //                    |
                // (c_col, c_row) +---+-------+ (c_col + val)
                //                    |
                //                    + (r0,c0,s0)
                //

                if matches.opt_present("d") {
                    println!("Wire 2: Moving left {val} positions.");
                }
                for line in &v_lines {
                    if c_col - val <= line.c0 && line.c0 <= c_col {
                        if line.r0 <= c_row && c_row <= line.r1 {
                            let w1_steps;
                            if line.s0 < line.s1 {
                                w1_steps = line.s0 + (c_row - line.r0).abs();
                            } else {
                                w1_steps = line.s1 + (c_row - line.r1).abs();
                            }
                            let w2_steps = steps + (line.c0 - c_col).abs();
                            let new_steps = w1_steps + w2_steps;

                            if new_steps > 0 && (intersection_steps == -1 || new_steps < intersection_steps) {
                                if matches.opt_present("v") {
                                    println!("Intersection found at {},{}",
                                             line.r0, c_col);
                                    println!("Wire 1 segment: {}, steps {w1_steps}; Wire 2 steps: {w2_steps} => {new_steps}",
                                             line.ix);
                                }
                                intersection_steps = new_steps;
                            }
                        }
                    }
                }
                c_col -= val;
                steps += val;
            },
            b'R' => {
                if matches.opt_present("d") {
                    println!("Wire 2: Moving right {val} positions.");
                }
                for line in &v_lines {
                    if c_col <= line.c0 && line.c0 <= c_col + val {
                        if line.r0 <= c_row && c_row <= line.r1 {
                            let w1_steps;
                            if line.s0 < line.s1 {
                                w1_steps = line.s0 + (c_row - line.r0).abs();
                            } else {
                                w1_steps = line.s1 + (c_row - line.r1).abs();
                            }
                            let w2_steps = steps + (line.c0 - c_col).abs();
                            let new_steps = w1_steps + w2_steps;

                            if new_steps > 0 && (intersection_steps == -1 || new_steps < intersection_steps) {
                                if matches.opt_present("v") {
                                    println!("Intersection found at {},{}",
                                             line.r0, c_col);
                                    println!("Wire 1 segment: {}, steps {w1_steps}; Wire 2 steps: {w2_steps} => {new_steps}",
                                             line.ix);
                                }
                                intersection_steps = new_steps;
                            }
                        }
                    }
                }
                c_col += val;
                steps += val;
            },
            _ => {
                println!("Wire 2: Unexpected direction: {dir}");
                process::exit(1);
            }
        }
    }

    intersection_steps
}


// Main program, which parses the command line options, parses the
// file, and call the appropriate run_* functions with the parsed data
fn main() {
    let args: Vec<String> = env::args().collect();

    let matches = parse_options(&args);

    let mut answers: HashMap<&u8, HashMap<&str, i64>> = HashMap::new();
    answers.insert(&1, HashMap::from([
        ("sample",    6),
        ("sample2", 159),
        ("sample3", 135),
        ("input",   209),
    ]));
    answers.insert(&2, HashMap::from([
        ("sample",     30),
        ("sample2",   610),
        ("sample3",   410),
        ("input",   43258),
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
                let (wire1, wire2) = parse_input(&matches, &String::from(t_filename));

                let answer: i64;
                let now = Instant::now();

                // Still hardcoding which function to call
                match t_part {
                    1 => answer = run_part1(&matches, &wire1, &wire2),
                    2 => answer = run_part2(&matches, &wire1, &wire2),
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
    let (wire1, wire2) = parse_input(&matches, &filename);

    let part_str = matches.opt_str("p").unwrap();
    let part = part_str.parse::<u8>().unwrap();
    let answer: i64;
    let now = Instant::now();

    match part {
        1 => answer = run_part1(&matches, &wire1, &wire2),
        2 => answer = run_part2(&matches, &wire1, &wire2),
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
