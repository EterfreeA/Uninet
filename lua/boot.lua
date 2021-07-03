execute = {}
function execute.file(...)
	local status, result = xpcall(dofile, debug.traceback, ...)
	if not status then
		print(result)
	end
	return status, result
end

function execute.functor(functor, ...)
	local result = { xpcall(functor, debug.traceback, ...) }
	if not result[1] then
		print(result[2])
	end
	return result
end

local function batch(names)
	local sources = {}
	for index = #names, 1, -1 do
		sources[index] = require(names[index])
	end
	return sources
end

local IMPORT_MAPPING = {
	["string"] = require,
	["table"] = batch,
}

path = {}
function path.config(index)
	local counter = 1
	for separator in string.gmatch(package.config, "[^\n]") do
		if (counter >= index) then
			return separator
		end
		counter = counter + 1
	end
end

function path.import(names, path)
	if not names then
		return
	end

	local type = type(names)
	local import = assert(IMPORT_MAPPING[type], type)
	if path then
		path, package.path = package.path, string.format("%s%s.lua", path, _ENV.path.config(3))
	end

	local result = execute.functor(import, names)
	if path then
		package.path = path
	end
	return result[2]
end

function path.current()
	local info = debug.getinfo(2, "S")
	local path = string.sub(info.source, 2)
	--return string.match(path, "^.*[\\/]") or ""

	local pattern = string.format(".*[%s]", _ENV.path.config(1))
	return string.match(path, pattern) or ""
end

local current = path.current()
package.path = string.format("%s%s%s%s.lua", package.path, path.config(2), current, path.config(3))
execute.file(current .. "main.lua")
