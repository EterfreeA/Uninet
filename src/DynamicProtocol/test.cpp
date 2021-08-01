#include "Packer.hpp"
#include "Unpacker.hpp"

#include <string>
#include <iostream>

std::string pack()
{
	using namespace Protocol;
	Packer::Table packer;
	packer.setString(1, "caoliu");

	uint64_t array[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	packer.setVector(2, static_cast<uint64_t*>(array), sizeof array / sizeof array[0]);

	Packer::Table table;
	table.setFloat(1, 1.8F);
	table.setDouble(2, 1.8);
	table.setUint8(3, static_cast<uint8_t>(18));
	table.setUint16(4, static_cast<uint16_t>(18));
	table.setUint32(5, static_cast<uint32_t>(18));
	table.setUint64(6, static_cast<uint64_t>(18));
	packer.setTable(3, table);
	return std::string(packer.data(), packer.size());
}

void unpack(const std::string& _stream)
{
	using namespace Protocol;
	Unpacker::Table unpacker(_stream);
	if (std::string name; unpacker.getString(1, name))
		std::cout << name << std::endl;

	if (std::vector<uint64_t> vector; unpacker.getVector(2, vector))
	{
		for (decltype(vector.size()) index = 0; index < vector.size(); ++index)
			std::cout << vector[index] << ' ';
		std::cout << '\b' << std::endl;
	}

	Unpacker::Table table;
	if (!unpacker.getTable(3, table))
		return;

	if (auto optional = table.getFloat(1); optional)
		std::cout << optional.value() << std::endl;
	if (auto optional = table.getDouble(2); optional)
		std::cout << optional.value() << std::endl;

	if (auto optional = table.getUint8(3); optional)
		std::cout << static_cast<uint16_t>(optional.value()) << std::endl;
	if (auto optional = table.getUint16(4); optional)
		std::cout << optional.value() << std::endl;
	if (auto optional = table.getUint32(5); optional)
		std::cout << optional.value() << std::endl;
	if (auto optional = table.getUint64(6); optional)
		std::cout << optional.value() << std::endl;
}

int main()
{
	unpack(pack());
	return 0;
}
