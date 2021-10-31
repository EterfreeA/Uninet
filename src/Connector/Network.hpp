#pragma once

#include <utility>
#include <cstddef>
#include <cstdint>

#define NETWORK_BEGIN namespace Network {
#define NETWORK_END }
#define NETWORK using namespace Network;

NETWORK_BEGIN

enum class INTERNET_PROTOCOL : uint8_t
{
	INVALID, IPv4, IPv6
};

using SizeType = std::size_t;
using IDType = std::size_t;
using PortType = std::uint16_t;
using SocketType = std::size_t;
using SocketPair = std::pair<SocketType, SizeType>;

inline constexpr auto SOCKET_BUFFER_SIZE = static_cast<std::size_t>(8192); // 8K

bool initNetwork();

NETWORK_END
