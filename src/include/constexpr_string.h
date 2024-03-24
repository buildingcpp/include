#pragma once

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <string_view>
#include <algorithm>


namespace bcpp
{

    template <std::size_t N>
    struct constexpr_string
    {
        using element_type = char;
        using value_type = element_type[N];

        constexpr constexpr_string
        (
            element_type const (&value)[N]
        ) noexcept
        {
            std::copy_n(value, N, value_);
        }

        constexpr value_type const & get() const noexcept;

        constexpr operator value_type const & () const noexcept;

        constexpr operator std::string_view const() const noexcept;

        static auto constexpr size() noexcept {return N;}

        value_type value_;
    };


    //=========================================================================
    template <typename T>
    concept constexpr_string_concept = std::is_same_v<T, constexpr_string<T::size()>>;

} // namespace bcpp


//=============================================================================
template <std::size_t N>
constexpr auto bcpp::constexpr_string<N>::get
(
) const noexcept -> value_type const & 
{
    return value_;
}


//=============================================================================
template <std::size_t N>
constexpr bcpp::constexpr_string<N>::operator value_type const & 
(
) const noexcept
{
    return value_;
}


//=============================================================================
template <std::size_t N>
constexpr bcpp::constexpr_string<N>::operator std::string_view const
(
) const noexcept
{
    return value_;
}


//=============================================================================
static constexpr auto operator <=>
(
    bcpp::constexpr_string_concept auto const & first,
    bcpp::constexpr_string_concept auto const & second
) noexcept
{
    return (std::string_view(first) <=> std::string_view(second));
}


//=============================================================================
static constexpr auto operator ==
(
    bcpp::constexpr_string_concept auto const & first,
    bcpp::constexpr_string_concept auto const & second
) noexcept
{
    return (std::string_view(first) == std::string_view(second));
}