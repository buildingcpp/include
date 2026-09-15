#pragma once

#include <include/underlying.h>

#include <cstddef>
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
        return static_cast<T>(byte_swap(bcpp::to_underlying(value)));
    }

    namespace compile_time_validation::byte_swap_checks
    {
        // Known byte patterns check each supported width.
        static_assert(byte_swap(std::uint8_t{0xa5}) == std::uint8_t{0xa5});
        static_assert(byte_swap(std::uint16_t{0x1234}) == std::uint16_t{0x3412});
        static_assert(byte_swap(std::uint32_t{0x12345678}) == std::uint32_t{0x78563412});
        static_assert(byte_swap(std::uint64_t{0x0123456789abcdef}) == std::uint64_t{0xefcdab8967452301});

        // Preserve the signed type and its reversed bit pattern.
        static_assert(std::same_as<decltype(byte_swap(std::int16_t{})), std::int16_t>);
        static_assert(std::same_as<decltype(byte_swap(std::int32_t{})), std::int32_t>);
        static_assert(std::same_as<decltype(byte_swap(std::int64_t{})), std::int64_t>);
        static_assert(byte_swap(std::int16_t{-2}) == std::int16_t{-257});
        static_assert(byte_swap(std::int32_t{-2}) == std::int32_t{-0x01000001});
        static_assert(byte_swap(std::int64_t{-2}) == std::int64_t{-0x0100000000000001});
        static_assert(byte_swap(byte_swap(std::uint64_t{0x0123456789abcdef})) == std::uint64_t{0x0123456789abcdef});

        enum class code : std::uint16_t { original = 0x1234, swapped = 0x3412 };
        static_assert(std::same_as<decltype(byte_swap(code::original)), code>);
        static_assert(byte_swap(code::original) == code::swapped);
        static_assert(byte_swap(code::swapped) == code::original);
        static_assert(byte_swap(std::byte{0xa5}) == std::byte{0xa5});
    }

} // namespace bcpp
