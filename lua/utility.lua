local MODULE = {}

local function copy(old)
	if type(old) ~= "table" then
		return old
	end

	local new = {}
	for key, value in pairs(old) do
		new[copy(key)] = copy(value)
	end
	return new
end

function MODULE.copy(table)
	return copy(table)
end

return MODULE
