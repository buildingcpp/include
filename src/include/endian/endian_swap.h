#pragma once

#include "./byte_swap.h"

#include <bit>
#include <type_traits>
#include <utility>


namespace bcpp
{

    template <std::endian from_endian, std::endian to_endian, typename data_type>
    static constexpr std::remove_cvref_t<data_type> endian_swap
    (
        data_type && input
    )
        requires ((from_endian == to_endian &&
            std::is_convertible_v<data_type &&, std::remove_cvref_t<data_type>>) ||
            (from_endian != to_endian && requires
            {
                byte_swap(std::forward<data_type>(input));
                // An exact prvalue return needs no copy or move constructor.
                requires std::is_same_v<std::remove_cv_t<decltype(byte_swap(std::forward<data_type>(input)))>,
                    std::remove_cvref_t<data_type>> ||
                    std::is_convertible_v<decltype(byte_swap(std::forward<data_type>(input))),
                        std::remove_cvref_t<data_type>>;
            }))
    {
        if constexpr (from_endian == to_endian)
            return std::forward<data_type>(input);
        else
            return byte_swap(std::forward<data_type>(input));
    }

} // namespace bcpp
