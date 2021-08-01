#include "Packer.hpp"

PROTOCOL_BEGIN

using Table = Packer::Table;

bool Table::setUint8(Number _number, uint8_t _value)
{
	constexpr auto size = sizeof _number + sizeof(char) + sizeof(char);
	if (MAX_SIZE - _stream.size() < size)
		return false;

	_stream.reserve(_stream.size() + size);

	_number = ByteOrder::hton(_number);
	_stream.append(reinterpret_cast<const char*>(&_number), sizeof _number);
	_stream.push_back(static_cast<char>(TYPE::UINT8));
	_stream.push_back(static_cast<char>(_value));
	return true;
}

bool Table::set(Number _number, TYPE _type, const char* _data, size_t _size)
{
	if (_data == nullptr && _size > 0)
		_size = 0;

	using ByteOrder::hton;
	auto length = hton(static_cast<Size>(_size));
	constexpr auto size = sizeof _number + sizeof(char) + sizeof length;

	auto maxSize = MAX_SIZE - _stream.size();
	if (maxSize < size)
		return false;

	if (maxSize - size < _size)
		return false;

	_stream.reserve(_stream.size() + size + _size);

	_number = hton(_number);
	_stream.append(reinterpret_cast<const char*>(&_number), sizeof _number);
	_stream.push_back(static_cast<char>(_type));
	_stream.append(reinterpret_cast<const char*>(&length), sizeof length);
	_stream.append(_data, _size);
	return true;
}

PROTOCOL_END
