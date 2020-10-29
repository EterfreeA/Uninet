--[[
local _G = _G
module(...)
--]]

local coder = {
	mappings = {},
}

function coder:map(name, object)
	self.mappings[name] = object
end

function coder:update(names)
	if not next(self.mappings) then
		return
	end

	for _, object in pairs(self.mappings) do
		if type(object) == "table" and object.__update then
			local state, result = xpcall(object.__update, debug.traceback, object, names)
			if not state then
				print(result)
			end
		end
	end
end

return coder