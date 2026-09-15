#pragma once

#include "underlying.h"

#include <array>
#include <charconv>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

namespace bcpp
{
    template <typename T>
    concept enum_type = std::is_enum_v<T>;

    template <enum_type Enum>
    struct enum_to_string
    {
        static constexpr std::array<std::pair<Enum, std::string_view>, 0> values{};
    };

    namespace detail
    {
        template <enum_type Enum, typename Integer>
        constexpr std::errc enum_from_integer(Integer input, Enum & output) noexcept
            requires std::is_integral_v<Integer>
        {
            using underlying_type = std::underlying_type_t<Enum>;
            using limits = std::numeric_limits<underlying_type>;
            if constexpr (std::is_signed_v<Integer>)
            {
                if (input < 0)
                {
                    if constexpr (!std::is_signed_v<underlying_type>)
                        return std::errc::result_out_of_range;
                    else if (input < static_cast<std::int64_t>(limits::min()))
                        return std::errc::result_out_of_range;
                }
                else if (static_cast<std::uint64_t>(input) > static_cast<std::uint64_t>(limits::max()))
                    return std::errc::result_out_of_range;
            }
            else if (static_cast<std::uint64_t>(input) > static_cast<std::uint64_t>(limits::max()))
                return std::errc::result_out_of_range;

            auto raw = static_cast<underlying_type>(input);
            for (auto const & [value, name] : enum_to_string<Enum>::values)
            {
                if (bcpp::to_underlying(value) == raw)
                {
                    output = value;
                    return {};
                }
            }

            // Only unscoped enums with a fixed underlying type admit arbitrary
            // values of that type. Direct list initialization detects that rule.
            if constexpr (std::is_convertible_v<Enum, underlying_type> &&
                requires (underlying_type value) { Enum{value}; })
            {
                output = Enum{raw};
                return {};
            }
            return std::errc::invalid_argument;
        }
    }

    template <typename Enum>
    std::string to_string(Enum value)
        requires enum_type<Enum>
    {
        for (auto const & [enum_value, name] : enum_to_string<Enum>::values)
        {
            if (enum_value == value)
                return std::string(std::string_view(name));
        }
        using underlying_type = std::underlying_type_t<Enum>;
        if constexpr (std::is_convertible_v<Enum, underlying_type>)
            return std::to_string(bcpp::to_underlying(value));
        else
            throw std::invalid_argument("unknown scoped enum value");
    }

    template <typename Enum>
    std::pair<Enum, std::errc> from_string(std::span<char const> input) noexcept
        requires enum_type<Enum> && std::is_same_v<Enum, std::decay_t<Enum>>
    {
        if (input.empty())
            return {Enum{}, std::errc::invalid_argument};

        std::string_view text(input.data(), input.size());
        for (auto const & [value, name] : enum_to_string<Enum>::values)
        {
            if (text == std::string_view(name))
                return {value, {}};
        }

        using underlying_type = std::underlying_type_t<Enum>;
        // from_chars has no bool overload, but enums can have bool as their base.
        using parse_type = std::conditional_t<std::is_same_v<underlying_type, bool>, std::uint8_t, underlying_type>;
        parse_type raw{};
        auto last = input.data() + input.size();
        auto result = std::from_chars(input.data(), last, raw);
        if (result.ec != std::errc{})
            return {Enum{}, result.ec};
        if (result.ptr != last)
            return {Enum{}, std::errc::invalid_argument};

        Enum value{};
        auto error = detail::enum_from_integer(raw, value);
        return {value, error};
    }
}
