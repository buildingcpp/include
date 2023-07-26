#pragma once

#include <cstdint>
#include <type_traits>
#include <concepts>


#ifdef __APPLE__
    #include <libkern/OSByteOrder.h>
#else
    #include <byteswap.h>
#endif


namespace bcpp
{

    //==============================================================================
    template <typename T>
    requires ((std::is_integral_v<T>) && (sizeof(T) == 1))
    constexpr T byte_swap
    (
        T value
    )
    {
        return value;
    }


    //==============================================================================
    template <typename T>
    requires ((std::is_integral_v<T>) && (sizeof(T) == 2))
    constexpr auto byte_swap
    (
        T value
    )
    {
        auto v = static_cast<std::uint16_t>(value);
        return static_cast<T>((v >> 8) | (v << 8));
    }


    //==============================================================================
    template <typename T>
    requires ((std::is_integral_v<T>) && (sizeof(T) == 4))
    constexpr auto byte_swap
    (
        T value
    )
    {
        #ifdef __APPLE__
            return static_cast<T>(OSSwapInt32(static_cast<uint32_t>(value)));
        #else
            return static_cast<T>(__builtin_bswap32(static_cast<uint32_t>(value)));
        #endif
    }


    //==============================================================================
    template <typename T>
    requires ((std::is_integral_v<T>) && (sizeof(T) == 8))
    constexpr auto byte_swap
    (
        T value
    )
    {
        #ifdef __APPLE__
            return static_cast<T>(OSSwapInt64(static_cast<uint64_t>(value)));
        #else
            return static_cast<T>(__builtin_bswap64(static_cast<uint64_t>(value)));
        #endif
    }


    //==============================================================================
    template <typename T>
    requires std::is_enum_v<T>
    constexpr auto byte_swap
    (
        T value
    )
    {
        return static_cast<T>(byte_swap(static_cast<std::underlying_type_t<T>>(value)));
    }

} // namespace bcpp
