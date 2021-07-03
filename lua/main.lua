local module = require "module"

local names = {
	"utility",
	"ranking_list",
}

for _, name in ipairs(names) do
	module:add(name)
end

local function dump(ranking_list)
	print("ranking_list")
	for _, record in ipairs(ranking_list.cache) do
		print(record.id, record.data)
	end
end

local ranking_list = module:get "ranking_list"
ranking_list:update(1, 10)
ranking_list:update(2, 9)

local utility = module:get "utility"
ranking_list = utility.copy(ranking_list)
dump(ranking_list)

local snapshot = ranking_list:backup()
--module.update()
--module.update(names)
module.update("ranking_list")
ranking_list = module:get "ranking_list"
dump(ranking_list)

ranking_list:recover(snapshot)
dump(ranking_list)
