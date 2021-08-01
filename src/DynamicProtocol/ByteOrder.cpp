#include "ByteOrder.hpp"

#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#elif defined(linux) || defined(__linux) || defined(__linux__)
#define LINUX
#include <arpa/inet.h>

#else
#error "Currently only supports Windows and Linux!"
#endif

BYTE_ORDER_BEGIN

template <typename _Type>
_Type swap(_Type _data)
{
	auto low = reinterpret_cast<char*>(&_data);
	constexpr auto size = sizeof(_Type) - 1;
	auto high = low + size;
	while (low < high)
	{
		char byte = *low;
		*low++ = *high;
		*high-- = byte;
	}
	return _data;
}

uint32_t hton(float _data)
{
#ifdef _WIN32
	return htonf(_data);
#elif defined LINUX
	return htonl(*reinterpret_cast<const uint32_t*>(&_data));
#else
#error "Currently only supports Windows and Linux!"
#endif
}

uint64_t hton(double _data)
{
#ifdef _WIN32
	return htond(_data);
#elif defined LINUX
	// 小端模式
	if (auto data = static_cast<uint64_t>(1); *reinterpret_cast<const char*>(&data) != '\0')
		_data = swap(_data);
	return *reinterpret_cast<const uint64_t*>(&_data);
#else
#error "Currently only supports Windows and Linux!"
#endif
}

uint16_t hton(uint16_t _data)
{
#ifdef _WIN32
	return htons(_data);
#elif defined LINUX
	return htons(_data);
#else
#error "Currently only supports Windows and Linux!"
#endif
}

uint32_t hton(uint32_t _data)
{
#ifdef _WIN32
	return htonl(_data);
#elif defined LINUX
	return htonl(_data);
#else
#error "Currently only supports Windows and Linux!"
#endif
}

uint64_t hton(uint64_t _data)
{
#ifdef _WIN32
	return htonll(_data);
#elif defined LINUX
	// 小端模式
	if (auto data = static_cast<uint64_t>(1); *reinterpret_cast<const char*>(&data) != '\0')
		_data = swap(_data);
	return _data;
#else
#error "Currently only supports Windows and Linux!"
#endif
}

template <typename _Source, typename _Target>
_Target ntoh(_Source _data)
{
	return static_cast<_Target>(_data);
}

template <>
float ntoh(uint32_t _data)
{
#ifdef _WIN32
	return ntohf(_data);
#elif defined LINUX
	uint32_t data = ntohl(_data);
	return *reinterpret_cast<const float*>(&data);
#else
#error "Currently only supports Windows and Linux!"
#endif
}

template <>
double ntoh(uint64_t _data)
{
#ifdef _WIN32
	return ntohd(_data);
#elif defined LINUX
	// 小端模式
	if (auto data = static_cast<uint64_t>(1); *reinterpret_cast<const char*>(&data) != '\0')
		_data = swap(_data);
	return *reinterpret_cast<const double*>(&_data);
#else
#error "Currently only supports Windows and Linux!"
#endif
}

template <>
uint16_t ntoh(uint16_t _data)
{
#ifdef _WIN32
	return ntohs(_data);
#elif defined LINUX
	return ntohs(_data);
#else
#error "Currently only supports Windows and Linux!"
#endif
}

template <>
uint32_t ntoh(uint32_t _data)
{
#ifdef _WIN32
	return ntohl(_data);
#elif defined LINUX
	return ntohl(_data);
#else
#error "Currently only supports Windows and Linux!"
#endif
}

template <>
uint64_t ntoh(uint64_t _data)
{
#ifdef _WIN32
	return ntohll(_data);
#elif defined LINUX
	// 小端模式
	if (auto data = static_cast<uint64_t>(1); *reinterpret_cast<const char*>(&data) != '\0')
		_data = swap(_data);
	return _data;
#else
#error "Currently only supports Windows and Linux!"
#endif
}

BYTE_ORDER_END
