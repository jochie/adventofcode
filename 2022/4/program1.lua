#!/usr/bin/env lua5.4

function contains(line, p1a, p1b, p2a, p2b)
	if p1a <= p2a and p2b <= p1b then
		return true
	end
	return false
end

local sum = 0
for line in io.lines() do
	local i1, i2, i3, i4 = string.match(line, "(%d+)-(%d+),(%d+)-(%d+)")
	i1 = tonumber(i1)
	i2 = tonumber(i2)
	i3 = tonumber(i3)
	i4 = tonumber(i4)
	if contains(line, i1, i2, i3, i4) then
		sum = sum + 1
	elseif contains(line, i3, i4, i1, i2) then
		sum = sum + 1
	end
end
print(string.format("Subsets: %d", sum))
