#include <cstddef>
#include <cstdint>
#include <string>

#include "Protocol.hpp"
#include "ByteOrder.hpp"

PROTOCOL_BEGIN

// 封包器
class Packer
{
public:
	class Table;

private:
	std::string _stream;

public:
	Packer(const std::string& _stream) : _stream(_stream) {}

	const std::string& getPack() const noexcept
	{
		return _stream;
	}
};

class Packer::Table
{
	std::string _stream;

private:
	bool set(Number _number, TYPE _type, const char* _data, size_t _size);

	template <typename _Type>
	bool set(Number _number, TYPE _type, _Type _value);

	template <typename _Type>
	bool set(Number _number, TYPE _type, const _Type* _vector, size_t _size);

public:
	const char* data() const noexcept
	{
		return _stream.data();
	}

	auto size() const noexcept
	{
		return _stream.size();
	}

	bool setFloat(Number _number, float _value)
	{
		return set(_number, TYPE::FLOAT, _value);
	}

	bool setDouble(Number _number, double _value)
	{
		return set(_number, TYPE::DOUBLE, _value);
	}

	bool setUint8(Number _number, uint8_t _value);

	bool setUint16(Number _number, uint16_t _value)
	{
		return set(_number, TYPE::UINT16, _value);
	}

	bool setUint32(Number _number, uint32_t _value)
	{
		return set(_number, TYPE::UINT32, _value);
	}

	bool setUint64(Number _number, uint64_t _value)
	{
		return set(_number, TYPE::UINT64, _value);
	}

	bool setString(Number _number, const std::string& _string)
	{
		return set(_number, TYPE::STRING, _string.data(), _string.size());
	}

	bool setString(Number _number, const char* _data, size_t _size)
	{
		return set(_number, TYPE::STRING, _data, _size);
	}

	bool setTable(Number _number, const Table& _table)
	{
		return set(_number, TYPE::TABLE, _table.data(), _table.size());
	}

	bool setTable(Number _number, const char* _data, size_t _size)
	{
		return set(_number, TYPE::TABLE, _data, _size);
	}

	bool setVector(Number _number, const float* _vector, size_t _size)
	{
		return set(_number, TYPE::VECTOR_FLOAT, _vector, _size);
	}

	bool setVector(Number _number, const double* _vector, size_t _size)
	{
		return set(_number, TYPE::VECTOR_DOUBLE, _vector, _size);
	}

	bool setVector(Number _number, const uint16_t* _vector, size_t _size)
	{
		return set(_number, TYPE::VECTOR_UINT16, _vector, _size);
	}

	bool setVector(Number _number, const uint32_t* _vector, size_t _size)
	{
		return set(_number, TYPE::VECTOR_UINT32, _vector, _size);
	}

	bool setVector(Number _number, const uint64_t* _vector, size_t _size)
	{
		return set(_number, TYPE::VECTOR_UINT64, _vector, _size);
	}
};

template <typename _Type>
bool Packer::Table::set(Number _number, TYPE _type, _Type _value)
{
	using ByteOrder::hton;
	constexpr auto size = sizeof _number + sizeof(char) + sizeof hton(_value);

	if (MAX_SIZE - _stream.size() < size)
		return false;

	_stream.reserve(_stream.size() + size);

	_number = hton(_number);
	_stream.append(reinterpret_cast<const char*>(&_number), sizeof _number);
	_stream.push_back(static_cast<char>(_type));

	auto value = hton(_value);
	_stream.append(reinterpret_cast<const char*>(&value), sizeof value);
	return true;
}

template <typename _Type>
bool Packer::Table::set(Number _number, TYPE _type, const _Type* _vector, size_t _size)
{
	if (_vector == nullptr && _size > 0)
		_size = 0;

	using ByteOrder::hton;
	auto length = hton(static_cast<Size>(_size));
	constexpr auto size = sizeof _number + sizeof(char) + sizeof length;

	auto maxSize = MAX_SIZE - _stream.size();
	if (maxSize < size)
		return false;

	if ((maxSize - size) / sizeof hton(*_vector) < _size)
		return false;

	_stream.reserve(_stream.size() + size + _size * sizeof hton(*_vector));

	_number = hton(_number);
	_stream.append(reinterpret_cast<const char*>(&_number), sizeof _number);
	_stream.push_back(static_cast<char>(_type));
	_stream.append(reinterpret_cast<const char*>(&length), sizeof length);

	for (decltype(_size) index = 0; index < _size; ++index)
	{
		auto element = hton(_vector[index]);
		_stream.append(reinterpret_cast<const char*>(&element), sizeof element);
	}
	return true;
}

PROTOCOL_END
