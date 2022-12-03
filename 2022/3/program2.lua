#!/usr/bin/env lua5.4

function process_compartment(str)
	local map = 0

	-- print(string.format("  Compartment: %s", str))
	for i = 1,str:len() do
		local byte = string.byte(str, i)
		if byte >= 96 then
			-- a..z = 97..122
			map = map | (2 ^ (byte - 97))
		else
			-- A..Z = 65..90
			map = map | (2 ^ (byte - 65 + 26))
		end
	end
	return map
end

function analyze_overlap(map)
	local priorities = 0
	local bit = 1
	for i = 1,26 do
		if map & bit > 0 then
			-- print(string.format("    Overlap: %s (priority: %d)", string.char(97 - 1 + i), i))
			priorities = priorities + i
		end
		bit = bit * 2
	end
	for i = 1,26 do
		if map & bit > 0 then
			-- print(string.format("    Overlap: %s (priority: %d)", string.char(65 - 1 + i), i + 26))
			priorities = priorities + i + 26
		end
		bit = bit * 2
	end
	return priorities
end

local sum = 0
local grouping = 0
local group_overlap = nil
for line in io.lines() do
	-- print(string.format("Grouping %d", grouping))
	-- print(string.format("%s -> %d", line, line:len()))
	local compartment1 = process_compartment(line:sub(1, line:len() / 2))
	local compartment2 = process_compartment(line:sub(line:len() / 2 + 1))
	-- print(string.format("  Compartment 1 vs: %d / %d", compartment1, compartment2))
	local overlap = compartment1 | compartment2
	if grouping == 0 then
		group_overlap = overlap
	else
		group_overlap = group_overlap & overlap
	end
	if grouping == 2 then
		-- print(string.format("  Overlap for group: %d", group_overlap))
		sum = sum + analyze_overlap(group_overlap)
	end
	grouping = (grouping + 1) % 3
end
print(string.format("Sum of priorities: %d", sum))
