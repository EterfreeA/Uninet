#include <utility>
#include <cstddef>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "Protocol.hpp"
#include "ByteOrder.hpp"

PROTOCOL_BEGIN

class InputTable
{
	std::string _stream;
	std::unordered_map<Number, size_t> _fields;

private:
	std::optional<size_t> next(size_t offset);

	void parse();

	template <typename _Source, typename _Target>
	std::optional<_Target> get(Number _number, TYPE _type);

	template <typename _Source, typename _Target>
	bool getVector(Number _number, TYPE _type, std::vector<_Target>& _vector);

public:
	InputTable() = default;

	explicit InputTable(const std::string& _stream)
		: _stream(_stream)
	{
		parse();
	}

	explicit InputTable(std::string&& _stream) noexcept
		: _stream(std::move(_stream))
	{
		parse();
	}

	InputTable(const char* _data, size_t _size)
		: _stream(_data, _size)
	{
		parse();
	}

	InputTable& operator=(const std::string& _stream)
	{
		this->_stream = _stream;
		_fields.clear();
		parse();
		return *this;
	}

	InputTable& operator=(std::string&& _stream)
	{
		this->_stream = std::move(_stream);
		_fields.clear();
		parse();
		return *this;
	}

	InputTable& set(const char* _data, size_t _size)
	{
		_stream.assign(_data, _size);
		_fields.clear();
		parse();
		return *this;
	}

	std::optional<float> getFloat(Number _number)
	{
		return get<uint32_t, float>(_number, TYPE::FLOAT);
	}

	std::optional<double> getDouble(Number _number)
	{
		return get<uint64_t, double>(_number, TYPE::DOUBLE);
	}

	std::optional<uint8_t> getUint8(Number _number);

	std::optional<uint16_t> getUint16(Number _number)
	{
		return get<uint16_t, uint16_t>(_number, TYPE::UINT16);
	}

	std::optional<uint32_t> getUint32(Number _number)
	{
		return get<uint32_t, uint32_t>(_number, TYPE::UINT32);
	}

	std::optional<uint64_t> getUint64(Number _number)
	{
		return get<uint64_t, uint64_t>(_number, TYPE::UINT64);
	}

	std::optional<std::string> getString(Number _number);

	bool getString(Number _number, std::string& _string);

	bool getTable(Number _number, InputTable& _table);

	bool getVector(Number _number, std::vector<float>& _vector)
	{
		return getVector<uint32_t, float>(_number, TYPE::VECTOR_FLOAT, _vector);
	}

	bool getVector(Number _number, std::vector<double>& _vector)
	{
		return getVector<uint64_t, double>(_number, TYPE::VECTOR_DOUBLE, _vector);
	}

	bool getVector(Number _number, std::vector<uint16_t>& _vector)
	{
		using Type = std::remove_reference_t<decltype(_vector[0])>;
		return getVector<Type, Type>(_number, TYPE::VECTOR_UINT16, _vector);
	}

	bool getVector(Number _number, std::vector<uint32_t>& _vector)
	{
		using Type = std::remove_reference_t<decltype(_vector[0])>;
		return getVector<Type, Type>(_number, TYPE::VECTOR_UINT32, _vector);
	}

	bool getVector(Number _number, std::vector<uint64_t>& _vector)
	{
		using Type = std::remove_reference_t<decltype(_vector[0])>;
		return getVector<Type, Type>(_number, TYPE::VECTOR_UINT64, _vector);
	}
};

template <typename _Source, typename _Target>
std::optional<_Target> InputTable::get(Number _number, TYPE _type)
{
	auto iterator = _fields.find(_number);
	if (iterator == _fields.end())
		return std::nullopt;

	auto data = _stream.data() + iterator->second;
	auto type = static_cast<TYPE>(*data);
	if (type != _type)
		return std::nullopt;

	auto value = *reinterpret_cast<const _Source*>(data + sizeof(char));
	return ByteOrder::ntoh<_Source, _Target>(value);
}

template <typename _Source, typename _Target>
bool InputTable::getVector(Number _number, TYPE _type, std::vector<_Target>& _vector)
{
	auto iterator = _fields.find(_number);
	if (iterator == _fields.end())
		return false;

	auto data = _stream.data();
	auto offset = iterator->second;
	auto type = static_cast<TYPE>(*(data + offset));
	if (type != _type)
		return false;

	offset += sizeof(char);
	using SizeType = Size;
	auto size = *reinterpret_cast<const SizeType*>(data + offset);
	using ByteOrder::ntoh;
	size = ntoh<SizeType, SizeType>(size);
	offset += sizeof size;

	_vector.reserve(_vector.size() + size);
	for (SizeType index = 0; index < size; ++index)
	{
		auto element = *reinterpret_cast<const _Source*>(data + offset);
		offset += sizeof element;
		_vector.push_back(ntoh<_Source, _Target>(element));
	}
	return true;
}

PROTOCOL_END
