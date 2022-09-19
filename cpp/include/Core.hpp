#pragma once

#ifndef __cplusplus
#error The file requires a C++ compiler.
#endif

#include <source_location>
#include <ostream>
#include <thread>

// 字符串化
#define STRING(content) #content

// 拼接
#define SPLICE(front, back) front##back

#define DEPRECATED \
[[deprecated("The name for this item is deprecated.")]]

#define REPLACEMENT(signature) \
[[deprecated("The name for this item is deprecated. " \
"Instead, use the name: " STRING(signature) ".")]]

// 自定义名称空间
#define ETERFREE_SPACE_BEGIN namespace eterfree {
#define ETERFREE_SPACE_END }
#define USING_ETERFREE_SPACE using namespace eterfree;

ETERFREE_SPACE_BEGIN

//template <typename _Type, const decltype(sizeof(0)) _SIZE>
//constexpr auto size(_Type(&_array)[_SIZE])
//{
//	return sizeof _array / sizeof _array[0];
//}

template <typename _Type, const decltype(sizeof(0)) _SIZE>
constexpr auto size(_Type(&_array)[_SIZE])
{
	return _SIZE;
}

inline std::ostream& operator<<(std::ostream& _stream, \
	const std::source_location& _location)
{
	return _stream << " in function " << _location.function_name() << " at " \
		<< _location.file_name() << ':' << _location.line() << ": ";
}

class ThreadID
{
public:
	using IDType = std::thread::id;

private:
	IDType _id;

private:
	friend std::ostream& operator<<(std::ostream& _stream, \
		const ThreadID& _id)
	{
		return _stream << "[thread " << _id._id << ']';
	}

public:
	ThreadID(IDType _id) noexcept : _id(_id) {}
};

ETERFREE_SPACE_END
