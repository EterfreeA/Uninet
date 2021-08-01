#pragma once

#include <cstdint>

#define BYTE_ORDER_BEGIN namespace ByteOrder {
#define BYTE_ORDER_END }

BYTE_ORDER_BEGIN

uint32_t hton(float _data);

uint64_t hton(double _data);

uint16_t hton(uint16_t _data);

uint32_t hton(uint32_t _data);

uint64_t hton(uint64_t _data);

template <typename _Source, typename _Target>
_Target ntoh(_Source _data);

template <>
float ntoh(uint32_t _data);

template <>
double ntoh(uint64_t _data);

template <>
uint16_t ntoh(uint16_t _data);

template <>
uint32_t ntoh(uint32_t _data);

template <>
uint64_t ntoh(uint64_t _data);

BYTE_ORDER_END
