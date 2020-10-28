local coder = require "coder"

local names = {
	"utility",
	"ranking_list",
}

local modules = {}
for _, name in ipairs(names) do
	modules[name] = require(name)
end

local function add(name)
	local module = require(name)
	modules[name] = module
	coder:map(name, module)
end

local function remove(name)
	modules[name] = nil
	coder:map(name, nil)
	package.loaded[name] = nil
end

local function get(name)
	local module = modules[name]
	if not module then
		add(name)
	end
	return modules[name]
end

local function launch()
	for name, module in pairs(modules) do
		coder:map(name, module)
	end
end

local function single(name)
	remove(name)
	add(name)
end

local function batch(names)
	if not next(names) then
		return
	end

	for _, name in ipairs(names) do
		single(name)
	end
end

local UPDATE_MAPPINGS = {
	["string"] = single,
	["table"] = batch,
}

local function update(names)
	if not names then
		return
	end

	local type = type(names)
	local update = assert(UPDATE_MAPPINGS[type], type)
	update(names)
	coder:update(names)
end

local function dump(ranking_list)
	print("ranking_list")
	for _, record in ipairs(ranking_list.cache) do
		print(record.id, record.data)
	end
end

launch()
local ranking_list = get "ranking_list"
ranking_list:update(1, 10)
ranking_list:update(2, 9)

local utility = get "utility"
ranking_list = utility.copy(ranking_list)
dump(ranking_list)

local snapshot = ranking_list:backup()
update("ranking_list")
ranking_list = get "ranking_list"
dump(ranking_list)

ranking_list:recover(snapshot)
dump(ranking_list)