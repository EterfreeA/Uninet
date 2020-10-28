local function single(name)
	return require(name)
end

local function batch(names)
	local sources = {}
	for index = 1, #names do
		sources[index] = require(names[index])
	end
	return sources
end

local IMPORT_MAPPINGS = {
	["string"] = single,
	["table"] = batch,
}

path = {}
function path.import(names, path)
	if not names then
		return
	end

	local type = type(names)
	if type == "table" and not next(names) then
		return
	end

	local import = assert(IMPORT_MAPPINGS[type], type)
	if path then
		path, package.path = package.path, path .. "?.lua"
	end

	local state, sources = xpcall(import, debug.traceback, names)
	if not state then
		print(sources)
	end

	if path then
		package.path = path
	end
	return sources
end

function path.current()
	local info = debug.getinfo(2, "S")
	local path = string.sub(info.source, 2)
	return string.match(path, "^.*[/\\]")
end

package.path = package.path .. ';' .. path.current() .. "?.lua"
dofile(path.current() .. "/main.lua")