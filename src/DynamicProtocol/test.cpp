#include "OutputTable.hpp"
#include "InputTable.hpp"

#include <string>
#include <iostream>

std::string pack()
{
	using namespace Protocol;
	OutputTable table;
	table.setString(1, "caoliu");

	uint64_t array[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	table.setVector(2, static_cast<uint64_t*>(array), sizeof array / sizeof array[0]);

	OutputTable subTable;
	subTable.setFloat(1, 1.8F);
	subTable.setDouble(2, 1.8);
	subTable.setUint8(3, static_cast<uint8_t>(18));
	subTable.setUint16(4, static_cast<uint16_t>(18));
	subTable.setUint32(5, static_cast<uint32_t>(18));
	subTable.setUint64(6, static_cast<uint64_t>(18));
	table.setTable(3, subTable);
	return std::string(table.data(), table.size());
}

void unpack(const std::string& _stream)
{
	using namespace Protocol;
	InputTable table(_stream);
	if (std::string name; table.getString(1, name))
		std::cout << name << std::endl;

	if (std::vector<uint64_t> vector; table.getVector(2, vector))
	{
		for (decltype(vector.size()) index = 0; index < vector.size(); ++index)
			std::cout << vector[index] << ' ';
		std::cout << '\b' << std::endl;
	}

	InputTable subTable;
	if (!table.getTable(3, subTable))
		return;

	if (auto optional = subTable.getFloat(1); optional)
		std::cout << optional.value() << std::endl;
	if (auto optional = subTable.getDouble(2); optional)
		std::cout << optional.value() << std::endl;

	if (auto optional = subTable.getUint8(3); optional)
		std::cout << static_cast<uint16_t>(optional.value()) << std::endl;
	if (auto optional = subTable.getUint16(4); optional)
		std::cout << optional.value() << std::endl;
	if (auto optional = subTable.getUint32(5); optional)
		std::cout << optional.value() << std::endl;
	if (auto optional = subTable.getUint64(6); optional)
		std::cout << optional.value() << std::endl;
}

int main()
{
	unpack(pack());
	return 0;
}
