#!/usr/bin/env lua5.4

require "json"

local cwd = ""
local cmd = nil
local output = {}
local dirs = {}

function mysplit (inputstr, sep)
        if sep == nil then
                sep = "%s"
        end
        local t={}
        for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
                table.insert(t, str)
        end
        return t
end

function mark_directory(dir)
	local t = dirs
	if cwd == "" then
		dir = cwd .. dir
	else
		dir = cwd .. "/" .. dir
	end
	local t = dirs
	for i, path in ipairs(mysplit(dir, "/")) do
		if not t[path] then
			t[path] = {}
		end
		t = t[path]
	end
end

function mark_file(file, size)
	if cwd == "" then
		file = cwd .. file
	else
		file = cwd .. "/" .. file
	end
	local t = dirs
	for i, path in ipairs(mysplit(file, "/")) do
		if not t[path] then
			t[path] = {}
		end
		t = t[path]
	end
	t._size_ = size
end

function process_cmd(cmd, output)
	local exec, extra = string.match(cmd, "(%S+) (.*)")
	if exec == nil then
		exec = cmd
	end
	if exec == "cd" then
		if string.sub(extra, 1, 1) == "/" then
			cwd = string.sub(extra, 2)
		elseif extra == ".." then
			local p1, p2 = string.match(cwd, "(.*)/([^/]+)")
			if p1 and p2 then
				if p1:len() > 0 then
					cwd = p1
				else
					cwd = ""
				end
			else
				cwd = ""
			end
		else
			if cwd ~= "" then
				cwd = cwd .. "/"
			end
			cwd = cwd .. extra
		end
	elseif exec == "ls" then
		for i, line in ipairs(output) do
			local dir = string.match(line, "dir (.*)")
			if dir then
				mark_directory(dir)
			else
				local size, file = string.match(line, "(%d+)%s+(.+)");
				mark_file(file, size)
			end
		end
	end
end

for line in io.lines() do
	if string.sub(line, 1, 2) == "$ " then
		if cmd then
			process_cmd(cmd, output)
		end
		cmd = string.sub(line, 3)
		output = {}
	else
		table.insert(output, line)
	end
end
if cmd then
	process_cmd(cmd, output)
end

-- print(json.encode(dirs))

local matches = 0
function walk_dirs(top, prefix)
	if top._size_ then
		return top._size_
	end
	local total = 0
	for path in pairs(top) do
		total = total + walk_dirs(top[path], prefix..(prefix == "" and "" or "/")..path)
	end
	-- print(string.format("DIR /%s - %d", prefix, total))
	if total < 100000 then
		matches = matches + total
	end
	return total
end

local DISKUSED = walk_dirs(dirs, "")
print(string.format("SUM(Matches): %d", matches))

local DISKSIZE = 70000000
local DISKFREE = DISKSIZE - DISKUSED
local NEEDED = 30000000 - DISKFREE
print(string.format("Disk used: %d; disk free: %d; still needed: %d", DISKUSED, DISKFREE, NEEDED))

local found = -1
-- reset to 0
function walk_dirs(top, prefix)
	if top._size_ then
		-- file
		-- print(string.format("FILE /%s - %d", prefix, top._size_))
		return top._size_
	end
	local total = 0
	for path in pairs(top) do
		-- print(string.format("Recursing into %s", path))
		total = total + walk_dirs(top[path], prefix..(prefix == "" and "" or "/")..path)
	end
	-- print(string.format("DIR /%s - %d", prefix, total))
	if total >= NEEDED then
		if found == -1 or total < found then
			found = total
		end
	end
			
	return total
end
walk_dirs(dirs, "")
print(string.format("Smallest found: %s", found))
