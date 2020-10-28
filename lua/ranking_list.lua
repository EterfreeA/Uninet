--local utility = require "utility"

-- 原型模式 + 享元模式
local prototype = {
	max = 1000,
	cache = {},
	mapper = {},
	synchronized = nil,
}

function prototype.compare(left, right)
	return left.data > right.data
		or left.data == right.data
		and left.time < right.time
end

function prototype:init()
	local cache = self.cache
	local mapper = self.mapper
end

function prototype:get(id, number)
	local list
	local cache = self.cache
	if number then
		list = {}
		for ranking = 1, number do
			local record = cache[ranking]
			if not record then
				break
			end
			--list[ranking] = utility.copy(record)
			list[ranking] = record
		end
	else
		list = cache
	end

	local own
	if id then
		for ranking = 1, #cache do
			local record = cache[ranking]
			if record.id == id then
				own = {ranking = ranking}
				for key, value in pairs(record) do
					own[key] = value
				end
				break
			end
		end
	end
	return list, own
end

local function update(self, id, data)
	local cache = self.cache
	local mapper = self.mapper
	local counter = #cache

	local record = mapper[id]
	if record then
		record.data = data
		record.time = os.time()
	else
		record = {
			id = id,
			data = data,
			time = os.time(),
		}
		counter = counter + 1
		cache[counter] = record
		mapper[id] = record
	end

	table.sort(cache, self.compare)

	if counter > self.max then
		local removed = cache[counter].id
		cache[counter] = nil
		mapper[removed] = nil
		counter = counter - 1
	end
end

function prototype:update(id, data)
	local synchronized = self.synchronized
	if synchronized then
		return synchronized(update, self, id, data)
	else
		return update(self, id, data)
	end
end

local function remove(self, id)
	local index
	local cache = self.cache
	for ranking = 1, #cache do
		if cache[ranking].id == id then
			index = ranking
			break
		end
	end

	if index then
		local record = table.remove(cache, index)
		self.mapper[id] = nil
		table.sort(cache, self.compare)
		return record
	end
end

function prototype:remove(id)
	local synchronized = self.synchronized
	if synchronized then
		return synchronized(remove, self, id)
	else
		return remove(self, id)
	end
end

local function clear(self, limit)
	self.cache = {}
	self.mapper = {}
	self.max = limit or self.max
end

function prototype:clear(limit)
	local synchronized = self.synchronized
	if synchronized then
		return synchronized(clear, self, limit)
	else
		return clear(self, limit)
	end
end

function prototype:backup(number)
	local pointer
	if not number then
		pointer = self.cache
	else
		pointer = {}
		local cache = self.cache
		for index = 1, #cache do
			pointer[index] = cache[index]
		end
	end

	return {
		max = number or self.max,
		cache = pointer,
	}
end

local function recover(self, snapshot)
	self:clear(snapshot.max)
	local cache = snapshot.cache
	self.cache = cache
	local mapper = {}
	self.mapper = mapper

	if not next(cache) then
		return
	end

	for index = 1, #cache do
		local record = cache[index]
		mapper[record.id] = record
	end
end

function prototype:recover(snapshot)
	local synchronized = self.synchronized
	if synchronized then
		return synchronized(recover, self, snapshot)
	else
		return recover(self, snapshot)
	end
end

return prototype