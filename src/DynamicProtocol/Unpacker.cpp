#include "Unpacker.hpp"

#define NEXT(type, typeSize, data, totalSize, offset) \
if (auto size = sizeof(Size); totalSize - offset >= size) \
{ \
	auto length = *reinterpret_cast<const Size*>(data + offset); \
	size += ByteOrder::ntoh<Size, Size>(length) * sizeof(type); \
	if (totalSize - offset >= size) \
		return size + typeSize; \
}

PROTOCOL_BEGIN

using Table = Unpacker::Table;

std::optional<size_t> Table::next(size_t offset)
{
	auto totalSize = _stream.size();
	auto typeSize = sizeof(char);
	if (totalSize - offset < typeSize)
		return std::nullopt;

	auto type = static_cast<TYPE>(*(_stream.data() + offset));
	offset += typeSize;

	switch (type)
	{
	case TYPE::UINT8:
		if (auto size = sizeof(uint8_t); totalSize - offset >= size)
			return size + typeSize;
		break;
	case TYPE::UINT16:
		if (auto size = sizeof(uint16_t); totalSize - offset >= size)
			return size + typeSize;
		break;
	case TYPE::FLOAT:
	case TYPE::UINT32:
		if (auto size = sizeof(uint32_t); totalSize - offset >= size)
			return size + typeSize;
		break;
	case TYPE::DOUBLE:
	case TYPE::UINT64:
		if (auto size = sizeof(uint64_t); totalSize - offset >= size)
			return size + typeSize;
		break;
	case TYPE::STRING:
	case TYPE::TABLE:
		if (auto size = sizeof(Size); totalSize - offset >= size)
		{
			auto length = *reinterpret_cast<const Size*>(_stream.data() + offset);
			size += ByteOrder::ntoh<Size, Size>(length);

			if (totalSize - offset >= size)
				return size + typeSize;
		}
		break;
	case TYPE::VECTOR_UINT16:
		NEXT(uint16_t, typeSize, _stream.data(), totalSize, offset);
		break;
	case TYPE::VECTOR_FLOAT:
	case TYPE::VECTOR_UINT32:
		NEXT(uint32_t, typeSize, _stream.data(), totalSize, offset);
		break;
	case TYPE::VECTOR_DOUBLE:
	case TYPE::VECTOR_UINT64:
		NEXT(uint64_t, typeSize, _stream.data(), totalSize, offset);
		break;
	default:
		break;
	}
	return std::nullopt;
}

void Table::parse()
{
	auto size = _stream.size();
	decltype(size) offset = 0;
	while (offset < size)
	{
		constexpr auto length = sizeof(Number);
		if (size - offset < length)
			break;

		auto number = *reinterpret_cast<const Number*>(_stream.data() + offset);
		number = ByteOrder::ntoh<Number, Number>(number);

		auto optional = next(offset += length);
		if (!optional)
			break;

		_fields.insert(std::make_pair(number, offset));
		offset += optional.value();
	}
}

std::optional<uint8_t> Table::getUint8(Number _number)
{
	auto iterator = _fields.find(_number);
	if (iterator == _fields.end())
		return std::nullopt;

	auto data = _stream.data() + iterator->second;
	auto type = static_cast<TYPE>(*data);
	if (type != TYPE::UINT8)
		return std::nullopt;
	return *reinterpret_cast<const uint8_t*>(data + sizeof(char));
}

std::optional<std::string> Table::getString(Number _number)
{
	auto iterator = _fields.find(_number);
	if (iterator == _fields.end())
		return std::nullopt;

	auto data = _stream.data();
	auto offset = iterator->second;
	auto type = static_cast<TYPE>(*(data + offset));
	if (type != TYPE::STRING)
		return std::nullopt;

	offset += sizeof(char);
	auto size = *reinterpret_cast<const Size*>(data + offset);
	offset += sizeof size;
	return std::string(data + offset, ByteOrder::ntoh<Size, Size>(size));
}

bool Table::getString(Number _number, std::string& _string)
{
	auto iterator = _fields.find(_number);
	if (iterator == _fields.end())
		return false;

	auto data = _stream.data();
	auto offset = iterator->second;
	auto type = static_cast<TYPE>(*(data + offset));
	if (type != TYPE::STRING)
		return false;

	offset += sizeof(char);
	auto size = *reinterpret_cast<const Size*>(data + offset);
	offset += sizeof size;
	_string.append(data + offset, ByteOrder::ntoh<Size, Size>(size));
	return true;
}

bool Table::getTable(Number _number, Table& _table)
{
	auto iterator = _fields.find(_number);
	if (iterator == _fields.end())
		return false;

	auto data = _stream.data();
	auto offset = iterator->second;
	auto type = static_cast<TYPE>(*(data + offset));
	if (type != TYPE::TABLE)
		return false;

	offset += sizeof(char);
	auto size = *reinterpret_cast<const Size*>(data + offset);
	offset += sizeof size;
	_table.set(data + offset, ByteOrder::ntoh<Size, Size>(size));
	return true;
}

PROTOCOL_END
