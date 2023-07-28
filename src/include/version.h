#pragma once

#include <cstdint>
#include <concepts>
#include <type_traits>


namespace bcpp
{

    template <std::integral T0, std::integral T1 = 0ull, std::integral T2 = 0ull, std::integral T3 = 0ull>
    struct version_traits
    {
        static auto constexpr major     = T0;
        static auto constexpr minor     = T1;
        static auto constexpr revision  = T2;
        static auto constexpr patch     = T3;
    };

    template <typename T>
    concept version_traits_concept = std::is_same_v<T, version_traits<T::major, T::minor, T::revision, T::patch>>;

} // namespace bcpp
