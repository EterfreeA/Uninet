#include "IDManager.hpp"

#include <cstdlib>
#include <cstdint>
#include <iostream>

int main()
{
	using IDManager = IDManager<std::uint32_t>;
	using IDType = IDManager::IDType;
	using SizeType = IDManager::SizeType;

	using std::cout, std::endl, std::boolalpha;

	IDManager idManager(8, 0, 6);
	cout << "size: " << idManager.size() << endl;

	IDType extra = 0x3FU;
	cout << "extra: " << extra << endl;

	constexpr SizeType SIZE = 10;
	IDType pool[SIZE];

	cout << "\nget " << SIZE << endl;
	for (SizeType index = 0; index < SIZE; ++index)
		pool[index] = idManager.get();

	cout << "ids:" << endl;
	for (SizeType index = 0; index < SIZE; ++index)
		cout << pool[index] << ' ' << (pool[index] | extra) << endl;

	IDManager::VectorType vector;

	cout << "\nbackup" << endl;
	idManager.backup(vector);

	cout << "\nput last " << SIZE - 1 << endl;
	for (SizeType index = 1; index < SIZE; ++index)
		idManager.put(pool[index]);

	cout << "\nget 1" << endl;
	auto id = idManager.get();
	cout << "valid: " << boolalpha << idManager.valid(id) << endl;
	cout << "id:" << endl;
	cout << id << ' ' << (id | extra) << endl;

	cout << "\nrecover" << endl;
	idManager.recover(vector);

	cout << "\nput first 1" << endl;
	idManager.put(pool[0]);

	cout << "\nget 1" << endl;
	id = idManager.get();
	cout << "valid: " << boolalpha << idManager.valid(id) << endl;
	cout << "id:" << endl;
	cout << id << ' ' << (id | extra) << endl;

	cout << "\nrecover to invalid" << endl;
	vector.assign(1, 100000000 >> 6);
	idManager.recover(vector);

	cout << "\nget 1" << endl;
	id = idManager.get();
	cout << "valid: " << boolalpha << idManager.valid(id) << endl;
	cout << "id:" << endl;
	cout << id << ' ' << (id | extra) << endl;
	return EXIT_SUCCESS;
}
