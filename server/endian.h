#pragma once

#define SHUAI_LITTLE_ENDIAN 1   // 小端
#define SHUAI_BIG_ENDIAN 2      // 大端

#include <byteswap.h>
#include <stdint.h>
#include <type_traits>

namespace shuai
{

// 字节交换（byte swapping）是将数据的字节顺序进行反转的操作。它主要用于解决不同计算机架构（小端和大端）的数据互操作问题。
template <class T>
// std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type：这部分确保只有当 T 的大小为 64 位时，这个函数才能被实例化
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type byteswap(T value)
{
    return (T)bswap_64((uint64_t)value);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type byteswap(T value)
{
    return (T)bswap_32((uint32_t)value);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type byteswap(T value)
{
    return (T)bswap_16((uint16_t)value);
}

// 根据系统的字节序来定义 SHUAI_BYTE_ORDER。如果系统字节序是大端，则定义为 SHUAI_BIG_ENDIAN，否则定义为 SHUAI_LITTLE_ENDIAN
#if BYTE_ORDER == BIG_ENDIAN
#define SHUAI_BYTE_ORDER SHUAI_BIG_ENDIAN
#else
#define SHUAI_BYTE_ORDER SHUAI_LITTLE_ENDIAN
#endif

// 如果当前机器是大端机器
#if SHUAI_BYTE_ORDER == SHUAI_BIG_ENDIAN
template <class T>
T byteswapToBigEndian(T t) 
{
    return t; // No need to swap if already big-endian
}

template <class T>
T byteswapToLittleEndian(T t) 
{
    return byteswap(t);
}

// 如果当前机器是小端机器
#else

template <class T>
T byteswapToBigEndian(T t) 
{
    return byteswap(t);
}

template <class T>
T byteswapToLittleEndian(T t) 
{
    return t; // No need to swap if already little-endian
}

#endif

}
