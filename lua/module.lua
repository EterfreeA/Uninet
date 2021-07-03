--[[
local _G = _G
module(...)
--]]

local module = { mappings = {} }
function module:get(name)
	return self.mappings[name]
end

function module:add(name, ...)
	local mappings = self.mappings
	local object = mappings[name]
	if object then
		return object
	end

	object = require(name)
	if object then
		mappings[name] = object

		if type(object) == "table" and object.__recovery then
			execute.functor(object.__recovery, object, ...)
		end
	end
	return object
end

function module:remove(name, ...)
	local mappings = self.mappings
	local object = mappings[name]
	mappings[name] = nil
	package.loaded[name] = nil

	if object and type(object) == "table" and object.__backup then
		execute.functor(object.__backup, object, ...)
	end
	return object
end

local function single(name, ...)
	module:remove(name, ...)
	module:add(name, ...)
end

local function batch(names, ...)
	if not next(names) then
		return
	end

	for _, name in ipairs(names) do
		single(name, ...)
	end
end

local function all(names, ...)
	names = {}
	for name in pairs(module.mappings) do
		table.insert(names, name)
	end

	batch(names, ...)
end

local UPDATE_MAPPING = {
	["string"] = single,
	["table"] = batch,
	["nil"] = all,
}

function module.update(names, ...)
	local type = type(names)
	local update = assert(UPDATE_MAPPING[type], type)
	update(names, ...)
end

return module
