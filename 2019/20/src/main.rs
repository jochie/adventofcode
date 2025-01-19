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
    n: Vec<String>,
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
    let mut lines: Vec<String> = Vec::new();
    for line in content.lines() {
        lines.push(String::from(line));
    }
    Input { n: lines }
}


fn marker_code(b1: u8, b2: u8) -> u16 {
    (b1 as u16 - b'A' as u16) * 26 + b2 as u16 - b'A' as u16
}


// Take the original input, extract the actual maze bits, and where
// the donut hole starts
fn extract_maze(matches: &Matches,
                max_row: usize, max_col: usize, data: &Input,
                maze: &mut Vec<String>,
                hole_row: &mut usize, hole_col: &mut usize) {
    for row in 2..max_row - 2 {
        let mut line = String::from("");
        let bytes = data.n[row].as_bytes();
        for col in 2..max_col - 2 {
            match bytes[col] {
                b'#' => line += "#",
                b'.' => line += ".",
                _    => {
                    line += " ";
                    if *hole_row == 0 {
                        *hole_row = row;
                        *hole_col = col;
                    }
                },
            }
        }
        maze.push(line);
    }
    if matches.opt_present("d") {
        println!("Donut hole: {hole_row},{hole_col}");
    }
}


// Take the original input, and the size of the donut hole, and find
// all the [A-Z][A-Z] markers inside and around the maze, for
// "portals" as well as the start/end points.
fn extract_markers(matches: &Matches,
                   initial_max_row: usize, initial_max_col: usize,
                   hole_row: usize, hole_col: usize, data: &Input,
                   markers: &mut HashMap<(usize,usize),u16>) {
    // Top row
    let bytes0 = data.n[0].as_bytes();
    let bytes1 = data.n[1].as_bytes();
    for col in 3..initial_max_col - 3 {
        if bytes0[col] != b' ' && bytes1[col] != b' ' {
            if matches.opt_present("d") {
                println!("Top: Marker {}{} at {col}",
                         bytes0[col] as char, bytes1[col] as char);
            }
            markers.insert(
                (0, col - 2),
                marker_code(bytes0[col], bytes1[col])
            );
        }
    }
    // Bottom row
    let bytes0 = data.n[initial_max_row - 2].as_bytes();
    let bytes1 = data.n[initial_max_row - 1].as_bytes();
    for col in 3..initial_max_col - 3 {
        if bytes0[col] != b' ' && bytes1[col] != b' ' {
            if matches.opt_present("d") {
                println!("Bottom: Marker {}{} at {col}",
                         bytes0[col] as char, bytes1[col] as char);
            }
            markers.insert(
                (initial_max_row - 1 - 2 - 2, col - 2),
                marker_code(bytes0[col], bytes1[col])
            );
        }
    }
    // Left and right sides
    for row in 3..initial_max_row - 3 {
        let bytes = data.n[row].as_bytes();
        if bytes[0] != b' ' && bytes[1] != b' ' {
            if matches.opt_present("d") {
                println!("Left: Marker {}{} at {}",
                         bytes[0] as char, bytes[1] as char, row - 2);
            }
            markers.insert(
                (row - 2, 0),
                marker_code(bytes[0], bytes[1])
            );
        }
        if bytes[initial_max_col - 2] != b' ' && bytes[initial_max_col - 1] != b' ' {
            if matches.opt_present("d") {
                println!("Right: Marker {}{} at {}",
                         bytes[initial_max_col - 2] as char,
                         bytes[initial_max_col - 1] as char, row - 2);
            }
            markers.insert(
                (row - 2, initial_max_col - 1 - 2 - 2),
                marker_code(bytes[initial_max_col - 2], bytes[initial_max_col - 1])
            );
        }
    }
    // Donut hole top
    let bytes0 = data.n[hole_row].as_bytes();
    let bytes1 = data.n[hole_row + 1].as_bytes();
    for col in hole_col..initial_max_col - hole_col {
        if bytes0[col] != b' ' && bytes1[col] != b' ' {
            if matches.opt_present("d") {
                println!("Hole top: Marker {}{} at {col}",
                         bytes0[col] as char, bytes1[col] as char);
            }
            markers.insert(
                (hole_row - 1 - 2, col - 2),
                marker_code(bytes0[col], bytes1[col])
            );
        }
    }
    // Donut hole bottom
    let bytes0 = data.n[initial_max_row - hole_row - 2].as_bytes();
    let bytes1 = data.n[initial_max_row - hole_row - 1].as_bytes();
    for col in hole_col..initial_max_col - hole_col {
        if bytes0[col] != b' ' && bytes1[col] != b' ' {
            if matches.opt_present("d") {
                println!("Hole bottom: Marker {}{} at {col}",
                         bytes0[col] as char, bytes1[col] as char);
            }
            markers.insert(
                (initial_max_row - hole_row - 2, col - 2),
                marker_code(bytes0[col], bytes1[col])
            );
        }
    }
    // Donut hole left
    for row in hole_row..initial_max_row - hole_row {
        let bytes = data.n[row].as_bytes();
        if bytes[hole_col] != b' ' && bytes[hole_col + 1] != b' ' {
            if matches.opt_present("d") {
                println!("Hole left: Marker {}{} at {row}",
                         bytes[hole_col] as char, bytes[hole_col + 1] as char);
            }
            markers.insert(
                (row - 2, hole_col - 1 - 2),
                marker_code(bytes[hole_col], bytes[hole_col + 1])
            );
        }
    }
    // Donut hole right
    for row in hole_row..initial_max_row - hole_row {
        let bytes = data.n[row].as_bytes();
        if bytes[initial_max_col - hole_col - 2] != b' ' && bytes[initial_max_col - hole_col - 1] != b' ' {
            if matches.opt_present("d") {
                println!("Hole right: Marker {}{} at {row}",
                         bytes[initial_max_col - hole_col - 2] as char, bytes[initial_max_col - hole_col - 1] as char);
            }
            markers.insert(
                (row - 2, initial_max_col - hole_col - 2),
                marker_code(bytes[initial_max_col - hole_col - 2],
                            bytes[initial_max_col - hole_col - 1])
            );
        }
    }
}


// The main code for part 1
fn run_part1(matches: &Matches, data: &Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let initial_max_row: usize = data.n.len();
    let initial_max_col: usize = data.n[0].len();

    let mut maze: Vec<String> = Vec::new();
    let mut hole_row: usize = 0;
    let mut hole_col: usize = 0;

    extract_maze(matches, initial_max_row, initial_max_col, data, &mut maze,
                 &mut hole_row, &mut hole_col);

    let max_row = initial_max_row - 4;
    let max_col = initial_max_col - 4;

    let mut markers: HashMap<(usize,usize),u16> = HashMap::new();

    extract_markers(matches, initial_max_row, initial_max_col, hole_row, hole_col,
                    data, &mut markers);

    let mut start: (usize,usize) = (0,0);
    let mut end: (usize,usize) = (0,0);

    for ((row, col), mark) in &markers {
        if *mark == 0 as u16 {
            if matches.opt_present("v") {
                println!("({row:3},{col:3}) - {}{} [{mark}] <START>",
                         ((mark / 26) as u8 + b'A') as char,
                         ((mark % 26) as u8 + b'A') as char);
            }
            start = (*row, *col);
        } else if *mark == (25 + 26 * 25) as u16 {
            if matches.opt_present("v") {
                println!("({row:3},{col:3}) - {}{} [{mark}] <END>",
                         ((mark / 26) as u8 + b'A') as char,
                         ((mark % 26) as u8 + b'A') as char);
            }
            end = (*row, *col);
        } else {
            if matches.opt_present("v") {
                println!("({row:3},{col:3}) - {}{} [{mark}]",
                         ((mark / 26) as u8 + b'A') as char,
                         ((mark % 26) as u8 + b'A') as char);
            }
        }
    }
    markers.remove(&start);
    markers.remove(&end);

    let mut seen: HashMap<(usize,usize),bool> = HashMap::new();
    seen.insert(start, true);
    let mut checking: Vec<(usize,usize,usize)> = Vec::new();
    checking.push((start.0, start.1, 0));

    let rel_dirs = [(-1, 0), (0, 1), (1, 0), (0, -1)];

    loop {
        let mut new_checking: Vec<(usize,usize,usize)> = Vec::new();

        for (row, col, steps) in checking {
            if end == (row, col) {
                // Made it!
                return steps as i64;
            }
            for (rel_row, rel_col) in rel_dirs {
                if rel_row + (row as isize) < 0 || rel_col + (col as isize) < 0 || rel_row + (row as isize) >= max_row as isize || rel_col + (col as isize) >= max_col as isize {
                    continue;
                }
                let mut new_row: usize = (rel_row + row as isize) as usize;
                let mut new_col: usize = (rel_col + col as isize) as usize;
                if maze[new_row].as_bytes()[new_col] != b'.' {
                    continue;
                }
                if seen.contains_key(&(new_row, new_col)) {
                    continue;
                }
                if markers.contains_key(&(new_row, new_col)) {
                    let portal = markers[&(new_row, new_col)];
                    if matches.opt_present("d") {
                        println!("{new_row},{new_col} has a portal: {}", portal);
                    }
                    // Find the matching portal end point
                    for ((p_row, p_col), mark) in &markers {
                        if *mark == portal && (*p_row != new_row || *p_col != new_col) {
                            new_row = *p_row;
                            new_col = *p_col;
                            if matches.opt_present("d") {
                                println!("Warped to {new_row},{new_col}");
                            }
                            new_checking.push((new_row, new_col, steps + 2));
                            break;
                        }
                    }
                } else {
                    new_checking.push((new_row, new_col, steps + 1));
                }
                seen.insert((new_row, new_col), true);
            }
        }

        if new_checking.len() == 0 {
            break;
        }

        checking = new_checking;
    }

    -1
}


// The main code for part 2
fn run_part2(matches: &Matches, data: &Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }


    let initial_max_row: usize = data.n.len();
    let initial_max_col: usize = data.n[0].len();

    let mut maze: Vec<String> = Vec::new();
    let mut hole_row: usize = 0;
    let mut hole_col: usize = 0;

    extract_maze(matches, initial_max_row, initial_max_col, data, &mut maze,
                 &mut hole_row, &mut hole_col);

    let max_row = initial_max_row - 4;
    let max_col = initial_max_col - 4;

    let mut markers: HashMap<(usize,usize),u16> = HashMap::new();

    extract_markers(matches, initial_max_row, initial_max_col, hole_row, hole_col,
                    data, &mut markers);

    let mut start: (usize,usize) = (0,0);
    let mut end: (usize,usize) = (0,0);

    for ((row, col), mark) in &markers {
        if *mark == 0 as u16 {
            if matches.opt_present("v") {
                println!("({row:3},{col:3}) - {}{} [{mark}] <START>",
                         ((mark / 26) as u8 + b'A') as char,
                         ((mark % 26) as u8 + b'A') as char);
            }
            start = (*row, *col);
        } else if *mark == (25 + 26 * 25) as u16 {
            if matches.opt_present("v") {
                println!("({row:3},{col:3}) - {}{} [{mark}] <END>",
                         ((mark / 26) as u8 + b'A') as char,
                         ((mark % 26) as u8 + b'A') as char);
            }
            end = (*row, *col);
        } else {
            if matches.opt_present("v") {
                println!("({row:3},{col:3}) - {}{} [{mark}]",
                         ((mark / 26) as u8 + b'A') as char,
                         ((mark % 26) as u8 + b'A') as char);
            }
        }
    }
    markers.remove(&start);
    markers.remove(&end);

    // For "seen", we now also need to take into account at which
    // level we've visited the location already
    let mut seen: HashMap<(usize,usize,usize),bool> = HashMap::new();
    seen.insert((start.0, start.1, 0), true);

    // For "checking", we now also track the level at which we're
    // searching for a path
    let mut checking: Vec<(usize,usize,usize,usize)> = Vec::new();
    checking.push((start.0, start.1, 0, 0));

    let rel_dirs = [(-1, 0), (0, 1), (1, 0), (0, -1)];

    loop {
        let mut new_checking: Vec<(usize,usize,usize,usize)> = Vec::new();

        // dbg!(&checking);

        for (row, col, steps, level) in checking {
            if level == 0 && end == (row, col) {
                // Made it!
                return steps as i64;
            }
            for (rel_row, rel_col) in rel_dirs {
                if rel_row + (row as isize) < 0 || rel_col + (col as isize) < 0 || rel_row + (row as isize) >= max_row as isize || rel_col + (col as isize) >= max_col as isize {
                    continue;
                }
                let mut new_row: usize = (rel_row + row as isize) as usize;
                let mut new_col: usize = (rel_col + col as isize) as usize;
                if maze[new_row].as_bytes()[new_col] != b'.' {
                    continue;
                }
                if seen.contains_key(&(new_row, new_col, level)) {
                    continue;
                }
                if markers.contains_key(&(new_row, new_col)) {
                    if level == 0 && (new_row == 0 || new_row == max_row - 1 || new_col == 0 || new_col == max_col - 1) {
                        continue;
                    }
                    let portal = markers[&(new_row, new_col)];
                    if matches.opt_present("d") {
                        println!("{new_row},{new_col} has a portal: {}", portal);
                    }
                    // Find the matching portal end point
                    let new_level: usize;

                    if new_row == 0 || new_row == max_row - 1 || new_col == 0 || new_col == max_col - 1 {
                        new_level = level - 1;
                    } else {
                        new_level = level + 1;
                    }
                    for ((p_row, p_col), mark) in &markers {
                        if *mark == portal && (*p_row != new_row || *p_col != new_col) {
                            new_row = *p_row;
                            new_col = *p_col;
                            if matches.opt_present("d") {
                                println!("Warped to {new_row},{new_col} at {new_level}");
                            }
                            new_checking.push((new_row, new_col, steps + 2, new_level));
                            seen.insert((new_row, new_col, new_level), true);
                            break;
                        }
                    }
                } else {
                    new_checking.push((new_row, new_col, steps + 1, level));
                    seen.insert((new_row, new_col, level), true);
                }
            }
        }

        if new_checking.len() == 0 {
            break;
        }

        checking = new_checking;
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
        ("sample1",  23),
        ("sample2",  58),
        ("input",   544),
    ]));
    answers.insert(&2, HashMap::from([
        ("sample1",   26),
        ("sample3",  396),
        ("input",   6238),
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
