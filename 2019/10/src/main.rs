// -*- mode: rust; indent-tabs-mode: nil -*-
//
// Rust code for AoC program

// Useful while developing, allows focus on the bigger picture
#![allow(dead_code, unused_variables)]

extern crate getopts;

use getopts::{Options, Matches};

use std::cmp::Ordering;        // Ordering::*
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
    m: Vec<String>,
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
        lines.push(line.to_string());
    }

    Input { m: lines }
}


fn set_markers(max_row: usize, max_col: usize,
               start_row: usize, start_col: usize,
               dir_row: i32, dir_col: i32,
               grid: &mut Vec<String>,
               v: &mut Vec<Vec<(i32,i32,usize,usize)>>) -> i64 {
    let mut seen: bool = false;
    let mut next_row: i32 = start_row.try_into().unwrap();
    let mut next_col: i32 = start_col.try_into().unwrap();
    next_row += dir_row;
    next_col += dir_col;
    let mut distance = 0;

    // First '#' found is marked with 'X', any after that with 'x'
    let mut marker = b'X';

    loop {
        if grid[next_row as usize].as_bytes()[next_col as usize] == b'#' {
            unsafe {
                grid[next_row as usize].as_bytes_mut()[next_col as usize] = marker;
                marker = b'x';
            }
            if !seen {
                seen = true;
            }
            let mut a_list: Vec<(usize,usize)>;
            if distance == v.len() {
                v.push(Vec::new());
            }
            v[distance].push((next_row - start_row as i32, next_col - start_col as i32,
                              next_row.try_into().unwrap(), next_col.try_into().unwrap()));
            distance += 1;
        }
        next_row += dir_row;
        next_col += dir_col;
        if next_row < 0 || next_col < 0 || next_row >= max_row as i32 || next_col >= max_col as i32 {
            break;
        }
    }

    if seen { 1 } else { 0 }
}


fn calculate_visibility(matches: &Matches,
                        max_row: usize, max_col: usize,
                        row: usize, col: usize,
                        data: &Input) -> (i64, Vec<Vec<(i32,i32,usize,usize)>>) {
    let mut grid: Vec<String> = data.m.clone();
    let mut v: Vec<Vec<(i32,i32,usize,usize)>> = Vec::new();

    if matches.opt_present("d") {
        dbg!(&grid, row, col);
    }

    let start_row: i32 = row.try_into().unwrap();
    let start_col: i32 = col.try_into().unwrap();

    let mut distance: i32 = 1;
    let mut counted = 0;
    loop {
        let mut inside_grid = false;

        // Top edge of the square
        let mut dir_row = -distance;
        let mut rel_row = start_row + dir_row;
        if rel_row >= 0 && rel_row < max_row as i32 {
            for dir_col in -distance..=distance {
                let rel_col = start_col + dir_col;
                if rel_col < 0 || rel_col >= max_col as i32 {
                    continue;
                }
                counted += set_markers(max_row, max_col, start_row as usize, start_col as usize,
                                       dir_row, dir_col, &mut grid, &mut v);
                inside_grid = true;
            }
        }

        // Left edge of the square
        let mut dir_col = -distance;
        let mut rel_col = start_col + dir_col;
        if rel_col >= 0 && rel_col < max_col as i32 {
            for dir_row in -(distance - 1)..=(distance - 1) {
                let rel_row = start_row + dir_row;
                if rel_row < 0 || rel_row >= max_row as i32 {
                    continue;
                }
                counted += set_markers(max_row, max_col, start_row as usize, start_col as usize,
                                       dir_row, dir_col, &mut grid, &mut v);
                inside_grid = true;
            }
        }

        // Right edge of the square
        dir_col = distance;
        rel_col = start_col + dir_col;
        if rel_col >= 0 && rel_col < max_col as i32 {
            for dir_row in -(distance - 1)..=(distance - 1) {
                let rel_row = start_row + dir_row;
                if rel_row < 0 || rel_row >= max_row as i32 {
                    continue;
                }
                counted += set_markers(max_row, max_col, start_row as usize, start_col as usize,
                                       dir_row, dir_col, &mut grid, &mut v);
                inside_grid = true;
            }
        }

        // Bottom edge of the square
        dir_row = distance;
        rel_row = start_row + dir_row;
        if rel_row >= 0 && rel_row < max_row as i32 {
            for dir_col in -distance..=distance {
                let rel_col = start_col + dir_col;
                if rel_col < 0 || rel_col >= max_col as i32 {
                    continue;
                }
                counted += set_markers(max_row, max_col, start_row as usize, start_col as usize,
                                       dir_row, dir_col, &mut grid, &mut v);
                inside_grid = true;
            }
        }
        
        if !inside_grid {
            break;
        }
        distance += 1;
    }

    if matches.opt_present("d") {
        dbg!(&grid, counted);
    }

    (counted, v)
}


// The main code for part 1
fn run_part1(matches: &Matches, data: &Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let max_row = data.m.len();
    let max_col = data.m[0].len();
    if matches.opt_present("v") {
        println!("A grid of {max_row} x {max_col}");
    }

    let mut best_total = 0;
    let mut best_row = 0;
    let mut best_col = 0;

    for row in 0..max_row {
        for col in 0..max_col {
            if data.m[row].as_bytes()[col] == b'#' {
                let (total, _) = calculate_visibility(matches, max_row, max_col, row, col, data);
                if total > best_total {
                    best_total = total;
                    best_row = row;
                    best_col = col;
                }
            }
        }
    }
    if matches.opt_present("v") {
        println!("Best is {best_col},{best_row} with {best_total} other asteroids detected:");
    }

    best_total
}


// The main code for part 2
fn run_part2(matches: &Matches, data: &Input) -> i64 {
    if matches.opt_present("d") {
        dbg!(&data);
    }

    let max_row = data.m.len();
    let max_col = data.m[0].len();
    if matches.opt_present("v") {
        println!("A grid of {max_row} x {max_col}");
    }

    let mut best_total = 0;
    let mut best_row = 0;
    let mut best_col = 0;
    let mut best_v: Vec<Vec<(i32,i32,usize,usize)>> = Vec::new();

    for row in 0..max_row {
        for col in 0..max_col {
            if data.m[row].as_bytes()[col] == b'#' {
                let (total, v) = calculate_visibility(matches, max_row, max_col, row, col, data);
                if total > best_total {
                    best_total = total;
                    best_row = row;
                    best_col = col;
                    best_v = v;
                }
            }
        }
    }

    if matches.opt_present("v") {
        println!("Best is {best_col},{best_row} with {best_total} other asteroids detected:");
    }

    if matches.opt_present("d") {
        for ix in 0..best_v.len() {
            println!("best_v[{ix}] len = {}", best_v[ix].len());
        }
    }

    // Now let the vaporizing begin...
    //
    // In the sample and actual input, #200 is actually in the first
    // group each time, but this tries to do the right thing for the
    // case where it isn't.

    let entry_needed = 200;

    let mut ix_total = 0;
    let mut ix_needed = 0;
    let mut ix_entry = 0;

    for ix in 0..best_v.len() {
        if ix_total <= entry_needed && entry_needed <= ix_total + best_v[ix].len() {
            ix_needed = ix;
            ix_entry = entry_needed - 1 - ix_total;
            break;
        }
        ix_total += best_v[ix].len();
    }
    if matches.opt_present("v") {
        println!("Section needed: {ix_needed}; Offset within that section: {ix_entry}");
    }

    let mut sort_v: Vec<(i32,i32,usize,usize,u8,f32)> = Vec::new();

    // Calculate the quadrants and row-col ratios such the the vectors
    // are ordered appropriately for our purpose in each of the
    // quadrants. In this exercise I'm calling the 100%
    // up/down/left/right vectors quadrants as well, to avoid needing
    // to add exceptions for them later.
    //
    // This does require floating point math but avoids diving into
    // angle calculations, at this time.

    for (row, col, row_abs, col_abs) in &best_v[ix_needed] {
        let quadrant: u8;
        let ratio: f32;

        // Note, *row 0 and *col 0 should not be possible, that's the
        // point of origin, since these are relative coordinates

        if *row < 0 && *col == 0 {
            // Up
            quadrant = 0;
            ratio = 0.0;
        } else if *row < 0 && *col > 0 {
            // Up and right
            quadrant = 1;
            ratio = (*col as f32)/-(*row as f32);
        } else if *row == 0 && *col > 0 {
            // Right
            quadrant = 2;
            ratio = 0.0;
        } else if *row > 0 && *col > 0 {
            // Down and right
            quadrant = 3;
            ratio = (*row as f32)/(*col as f32);
        } else if *row > 0 && *col == 0 {
            // Down
            quadrant = 4;
            ratio = 0.0;
        } else if *row > 0 && *col < 0 {
            // Down and left
            quadrant = 5;
            ratio = -(*col as f32)/(*row as f32);
        } else if *row == 0 && *col < 0 {
            // Left
            quadrant = 6;
            ratio = 0.0;
        } else if *row < 0 && *col < 0 {
            // Up and left
            quadrant = 7;
            ratio = -(*row as f32)/-(*col as f32);
        } else {
            // Wut
            quadrant = 8;
            ratio = -1.0;
        }
        sort_v.push((*row, *col, *row_abs, *col_abs, quadrant, ratio));
    }
    // Sort primarily by quadrant, and secondarily by ratio
    sort_v.sort_by(|a, b| {
        match a.4.cmp(&b.4) {
            Ordering::Equal => {
                if a.5 < b.5 {
                    Ordering::Less
                } else if a.5 > b.5 {
                    Ordering::Greater
                } else {
                    Ordering::Equal
                }
            },
            Ordering::Less => Ordering::Less,
            Ordering::Greater => Ordering::Greater,
        }
    });

    if matches.opt_present("d") {
        for (row, col, row_abs, col_abs, quadrant, ratio) in &sort_v {
            if *row == 0 || *col == 0 {
                println!("({row},{col}) -> {quadrant} N/A");
            } else {
                println!("({row},{col}) -> {quadrant} {ratio}");
            }
        }
    }

    if matches.opt_present("v") {
        println!("Selection: Relative {},{}; Absolute {},{}",
                 sort_v[ix_entry].1,
                 sort_v[ix_entry].0,
                 sort_v[ix_entry].3,
                 sort_v[ix_entry].2,
        );
    }
    
    ((best_col as i32 + sort_v[ix_entry].1) * 100 + (best_row as i32  + sort_v[ix_entry].0)).into()
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
        ("sample1",   8),
        ("sample2",  33),
        ("sample3",  35),
        ("sample4",  41),
        ("sample5", 210),
        ("input",   276),
    ]));
    answers.insert(&2, HashMap::from([
        ("sample5", 802),
        ("input",  1321),
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
