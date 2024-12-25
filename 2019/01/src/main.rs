use std::fs;  // fs::read_to_string()
use std::env; // env::args()

// Read the given file and return a list of integers
fn parse_input(filename: &String) -> Vec<i32> {
    let mut lines: Vec<i32> = Vec::new();
    let contents = fs::read_to_string(&filename)
	.expect("Missing file");
    for line in contents.lines() {
	let num: i32 = match line.trim().parse() {
	    Ok(num) => {
		println!("LINE: {num}>");
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

// Main program, takes the list of integers parsed from the file and
// does the necessary math calculations on them
fn main() {
    let args: Vec<String> = env::args().collect();

    let len = args.len();
    if len == 1 {
	println!("Usage: {} <filename>", &args[0]);
	return;
    }

    let mut part1_sum: i32 = 0;
    let mut part2_sum: i64 = 0;

    for num in parse_input(&args[1]) {
	let mut fuel: i32 = num / 3 - 2;

	part1_sum += fuel;

	while fuel > 0 {
	    part2_sum += i64::from(fuel);
	    fuel = fuel / 3 - 2;
	}
    }
    println!("Part 1 - Sum of the fuel requirements: {part1_sum}");
    println!("Part 2 - Sum of the fuel requirements: {part2_sum}");
}
