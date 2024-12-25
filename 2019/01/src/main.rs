use std::fs; // fs::read_to_string()


fn parse_input(filename: &String) -> String {
    let contents = fs::read_to_string(&filename)
	.expect("File missing?");
    return contents;
}


fn main() {
    let filename = String::from("input");
    let data = parse_input(&filename);

    let mut part1_sum: i32 = 0;
    let mut part2_sum: i64 = 0;

    for line in data.lines() {
	let num: i32 = match line.trim().parse() {
	    Ok(num) => num,
	    Err(_) => {
		println!("LINE MALFORMED <{line}>");
		continue;
	    }
	};
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
