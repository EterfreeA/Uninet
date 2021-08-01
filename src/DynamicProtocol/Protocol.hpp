/*
* 模块：dynamic protocol
*/

#pragma once

#include <cstdint>

#define PROTOCOL_BEGIN namespace Protocol {
#define PROTOCOL_END }

PROTOCOL_BEGIN

enum class TYPE : uint8_t
{
	FLOAT, DOUBLE, UINT8, UINT16, UINT32, UINT64, STRING, TABLE, \
	VECTOR_FLOAT, VECTOR_DOUBLE, VECTOR_UINT16, VECTOR_UINT32, VECTOR_UINT64
};

using Number = uint16_t;
using Size = uint32_t;
inline constexpr Size MAX_SIZE = UINT32_MAX;

PROTOCOL_END
