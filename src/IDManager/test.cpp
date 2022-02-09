#include "IDManager.hpp"

#include <cstdint>
#include <vector>
#include <iostream>

int main()
{
	using IDManager = IDManager<uint32_t>;
	using IDType = IDManager::IDType;
	using SizeType = IDManager::SizeType;

	IDManager idManager(8, 0, 6);
	std::cout << "size: " << (99999999 >> 6) - (10000000 >> 6) << std::endl;

	IDType extra = 0x1U << 5 | 0x1FU;
	std::cout << "extra: " << extra << std::endl;

	const SizeType SIZE = 10;
	IDType pool[SIZE];
	for (SizeType index = 0; index < SIZE; ++index)
		pool[index] = idManager.get();

	std::cout << "ids:" << std::endl;
	for (SizeType index = 0; index < SIZE; ++index)
		std::cout << pool[index] << ' ' << (pool[index] | extra) << std::endl;

	auto size = SIZE - 1;
	std::cout << "put: " << size << std::endl;
	for (decltype(size) index = 0; index < size; ++index)
		idManager.put(pool[index]);

	std::vector<IDType> vector;
	idManager.backup(vector);

	auto id = idManager.get();
	std::cout << "valid: " << std::boolalpha << idManager.valid(id) << std::endl;
	std::cout << "id:" << std::endl;
	std::cout << id << ' ' << (id | extra) << std::endl;

	idManager.recover(vector);
	idManager.put(pool[size]);

	std::cout << "Recover to invalid." << std::endl;
	vector.assign(1, 99999999 >> 6);
	idManager.recover(vector);

	id = idManager.get();
	std::cout << "valid: " << std::boolalpha << idManager.valid(id) << std::endl;
	std::cout << "id:" << std::endl;
	std::cout << id << ' ' << (id | extra) << std::endl;
	return 0;
}
