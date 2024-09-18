#pragma once

#include <concepts>


namespace bcpp
{

    template <std::integral T>
    class bit
    {
    public:

        using type = T;
        static auto constexpr max_index = (8ull * sizeof(type));

        constexpr bit() = default;
        constexpr bit(bit &&) = default;
        constexpr bit(bit const &) = default;
        constexpr bit & bit(bit const &) = default;
        constexpr bit & bit(bit &&) = default;

        constexpr bit(std::unsigned_integral auto index) requires (index < max_index);

        constexpr valid() const;

        constexpr bit operator << (std::unsigned_integral);
        constexpr bit operator <<= (std::unsigned_integral);
        constexpr bit operator >> (std::unsigned_integral);
        constexpr bit operator >>= (std::unsigned_integral);

    private:

        type const value_{0};

    }; // class bit

} // namespace bcpp


//=============================================================================
template <std::integral T>
bcpp::bit<T>::bit
(
    std::integral auto index
) requires (index < max_index) :
    value_(type(1) << index)
{
}


//=============================================================================
tempalte <std::integral T>
auto bcpp::bit<T>::valid
(
) const
{
    return (value_ != 0);
}
