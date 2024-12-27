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
fn parse_input(_matches: &Matches, filename: &String) -> Vec<u32> {
    let mut lines: Vec<u32> = Vec::new();
    let contents = fs::read_to_string(&filename)
        .expect("Missing file");
    for line in contents.lines() {
        for num in line.split(',') {
            lines.push(num.parse::<u32>().unwrap());
        }
    }
    return lines;
}


// The main code for part 1
fn run_part1(matches: &Matches, intcode: &mut Vec<u32>) -> u32 {
    if matches.opt_present("d") {
        dbg!(&intcode);
    }
    let mut pc = 0;
    loop {
        if intcode[pc] == 99 {
            if matches.opt_present("d") {
                println!("[{pc:03}] 99");
                println!("    Return {}", intcode[0]);
            }
            return intcode[0];
        }
        if intcode[pc] == 1 {
            if matches.opt_present("d") {
                println!("[{pc:03} 1 {} + {} -> {} at {}",
                         intcode[intcode[pc + 1] as usize], intcode[intcode[pc + 2] as usize],
                         intcode[intcode[pc + 1] as usize] + intcode[intcode[pc + 2] as usize],
                         intcode[pc + 3]);
            }
            let target = intcode[pc + 3];
            intcode[target as usize] = intcode[intcode[pc + 1] as usize] + intcode[intcode[pc + 2] as usize];
            pc += 4;
            continue;
        }
        if intcode[pc] == 2 {
            if matches.opt_present("d") {
                println!("[{pc:03} 1 {} * {} -> {} at {}",
                         intcode[intcode[pc + 1] as usize], intcode[intcode[pc + 2] as usize],
                         intcode[intcode[pc + 1] as usize] * intcode[intcode[pc + 2] as usize],
                         intcode[pc + 3]);
            }
            let target = intcode[pc + 3];
            intcode[target as usize] = intcode[intcode[pc + 1] as usize] * intcode[intcode[pc + 2] as usize];
            pc += 4;
            continue;
        }
    }
}


// The main code for part 2
fn run_part2(_matches: &Matches, _numbers: &mut Vec<u32>) -> u32 {
    /*
IP  Instructions    Symbolic       Dest  Symbolic result at the destination
  0 1,  0,  0,  3   [n] + [v]   => [3]   n + v
  4 1,  1,  2,  3   n + v       => [3]   n + v
  8 1,  3,  4,  3   n + v + 1   => [3]   (n + v) + 1 = n + v + 1
 12 1,  5,  0,  3   1 + 1       => [3]   2) (overwriting n + v + 1 at 3
 16 2, 10,  1, 19   4 * n       => [19]  4n
 20 2, 19,  6, 23   [19] * 2    => [23]  (4n) 2 = 8n
 24 2, 13, 23, 27   5 * [23]    => [27]  5 (8n) = 40n)
 28 1,  9, 27, 31   3 + [27]    => [31]  3 + 40n
 32 2, 31,  9, 35   [31] * 3    => [35]  (3 + 40n) 3 = 9 + 120n
 36 1,  6, 35, 39   2 + [35]    => [39]  2 + (9 + 120) = 11 + 120n
 40 2, 10, 39, 43   4 * [39]    => [43]  4 * (11 + 120n) = 44 + 480n
 44 1,  5, 43, 47   1 + [43]    => [47]  1 + (44 + 480n) = 45 + 480n
 48 1,  5, 47, 51   1 + [47]    => [51]  1 + (45 + 480n) = 46 + 480n
 52 2, 51,  6, 55   [51] * 2    => [55]  (46 + 480n) 2 = 92 + 960n
 56 2, 10, 55, 59   4 * [55]    => [59]  4 (92 + 960n) = 368 + 3840n
 60 1, 59,  9, 63   [49] + 3    => [63]  (368 + 3840n) + 3 = 371 + 3840n
 64 2, 13, 63, 67   5 * [63]    => [67]  5 (371 + 3840n) = 1855 + 19200n
 68 1, 10, 67, 71   4 + [67]    => [71]  4 + (1855 + 19200 * n) = 1859 + 19200 * n
 72 1, 71,  5, 75   [71] + 1    => [75]  (1859 + 19200 * n) + 1 = 1860 + 19200 * n
 76 1, 75,  6, 79   [75] + 2    => [79]  (1860 + 19200 * n) + 2 = 1862 + 19200 * n
 80 1, 10, 79, 83   4 + [79]    => [83]  4 + (1862 + 19200 * n) = 1866 + 19200 * n
 84 1,  5, 83, 87   1 + [83]    => [87]  1 + (1866 + 19200 * n) = 1867 + 19200 * n
 88 1,  5, 87, 91   1 + [87]    => [91]  1 + (1867 + 19200 * n) = 1868 + 19200 * n
 92 2, 91,  6, 95   [92] * 2    => [95]  (1868 + 19200 * n) * 2 = 3736 + 38400 * n
 96 2,  6, 95, 99   2 * [95]    => [99]  2 * (3736 + 38400 * n) = 7472 + 76800 * n
100 2, 10, 99,103   4 * [99]    => [103] 4 * (7472 + 76800 * n) = 29888 + 307200 * n
104 1,103,  5,107   [103] + 1   => [107] (29888 + 307200 * n) + 1 = 29889 + 307200 * n
108 1,  2,107,111   [v] + [107] => [111] v + (14865 + 307200 * n) = v + 29889 + 307200 * n
112 1,  6,111,  0   [2] + [111] => [0]   (v + 29889 + 307200 * n) + 2 = v + 29891 + 307200 * n
116 99,             EXIT
120 2, 14,  0,  0   UNUSED?

So in effect, we need to find:

   19690720 = v + 29891 + 307200 * n
=> 19660829 = v + 307200 * n

Which can be solved by using the modulo and division of 19660829 and 307200
*/
    let noun: u32 = 19660829 / 307200;
    let verb: u32 = 19660829 % 307200;
    return noun * 100 + verb;
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

    let answers: Vec<u32> = vec![  29891, 6429 ];

    let filename = matches.opt_str("f").unwrap();
    let mut numbers: Vec<u32> = parse_input(&matches, &filename);

    if matches.opt_present("p") {
        let part = matches.opt_str("p").unwrap().parse::<u32>().unwrap();
        let answer: u32;
        let now = Instant::now();

        if part == 1 {
            answer = run_part1(&matches, &mut numbers);
            if answer == answers[0] {
                if matches.opt_present("v") {
                    println!("Confirmed expected value from part {part}, filename '{filename}'");
                }
            } else {
                println!("Warning: Unexpected value from part {part}, filename '{filename}'");
            }
        } else if part == 2 {
            answer = run_part2(&matches, &mut numbers);
            if answer == answers[1] {
                if matches.opt_present("v") {
                    println!("Confirmed expected value from part {part}, filename '{filename}'");
                }
            } else {
                println!("Warning: Unexpected value from part {part}, filename '{filename}'");
            }
        } else {
            answer = 0;
        }
        let duration = now.elapsed();
        println!("[Duration {:.2?}] Part {part}, filename '{filename}', answer: {answer}",
                 duration);
    }
}
