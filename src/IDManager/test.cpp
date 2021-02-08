#include "IDManager.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>

int main()
{
	std::cout << "size: " << (99999999 >> 6) - (10000000 >> 6) << std::endl;
	IDManager<uint32_t> idManager(8, 0, 6);
	uint32_t pool[10];
	for (size_t index = 0; index < 10; ++index)
		pool[index] = idManager.get();

	uint32_t extra = 0x1UL << 5 | 0x1FUL;
	std::cout << "extra: " << extra << std::endl;
	std::cout << "ids:" << std::endl;
	for (size_t index = 0; index < 10; ++index)
		std::cout << pool[index] << ' ' << (pool[index] | extra) << std::endl;

	std::cout << "put: 9" << std::endl;
	for (size_t index = 0; index < 9; ++index)
		idManager.put(pool[index]);
	std::vector<uint32_t> ids;
	idManager.backup(ids);

	uint32_t id = idManager.get();
	std::cout << "valid: " << std::boolalpha << idManager.valid(id) << std::endl;
	std::cout << "id:" << std::endl;
	std::cout << id << ' ' << (id | extra) << std::endl;

	idManager.recover(ids);
	idManager.put(pool[9]);

	std::cout << "Recover to invalid." << std::endl;
	ids.assign(1, 99999999 >> 6);
	idManager.recover(ids);

	id = idManager.get();
	std::cout << "valid: " << std::boolalpha << idManager.valid(id) << std::endl;
	std::cout << "id:" << std::endl;
	std::cout << id << ' ' << (id | extra) << std::endl;
	return 0;
}
