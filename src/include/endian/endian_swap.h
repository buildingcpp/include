#pragma once

#include "./byte_swap.h"

#include <bit>
#include <type_traits>


namespace bcpp
{

    template <std::endian from_endian, std::endian to_endian, typename data_type>
    static constexpr data_type endian_swap
    (
        data_type input
    )
    {
        if constexpr (from_endian == to_endian)
            return input;
        else
            return byte_swap(input);
    }

} // namespace bcpp
