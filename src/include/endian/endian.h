#pragma once

#include "./byte_swap.h"
#include "./endian_swap.h"

#include <concepts>
#include <type_traits>
#include <bit>
#include <utility>


namespace bcpp
{

    template <typename data_type, std::endian endian_type>
    class endian;


    //=========================================================================
    template <typename T>
    concept endian_concept = std::is_same_v<T, endian<typename T::underlying_type, T::type>>;


    //==============================================================================
    template <typename data_type, std::endian endian_type>
    class endian
    {
    public:

        using underlying_type = data_type;
        static auto constexpr type = endian_type;

        template <typename, std::endian> friend class endian;

        endian() = default;

        template <typename T0, typename ... Ts>
        requires ((sizeof ... (Ts) > 1) || !std::is_same_v<data_type, T0>)
        endian
        (
            T0 &&,
            Ts && ...
        );

        endian
        (
            endian const &
        ) = default;

        endian
        (
            endian &&
        ) = default;

        endian
        (
            underlying_type const &
        );

        endian & operator =
        (
            endian const &
        ) = default;

        endian & operator =
        (
            endian &&
        ) = default;

        endian & operator =
        (
            underlying_type const &
        );

        template <typename T>
        requires (std::is_convertible_v<underlying_type, T>)
        operator T
        (
        ) const
        {
            return {get()};
        }

        underlying_type get() const;

        auto operator <=> 
        (
            endian const & other
        ) const
        {
            return (get() <=> other.get());
        }

        auto operator <=> 
        (
            underlying_type other
        ) const
        {
            return (get() <=> other);
        }

        /*
        // still considering if allowing arithmetic operators is a good idea or not.
        // hiding potential byte swaps might not be a good idea

        template <typename T>
        requires (std::integral<T> || std::floating_point<T>) &&
        requires (T t, underlying_type v){v + t;}
        auto operator +(T value) const{return (get() + value);}

        template <typename T>
        requires (std::integral<T> || std::floating_point<T>) &&
        requires (T t, underlying_type v){v += t;}
        endian & operator += (T value){*this = endian(get() + value); return *this;}

        template <typename T>
        requires (std::integral<T> || std::floating_point<T>) &&
        requires (T t, underlying_type v){v - t;}
        auto operator -(T value) const{return (get() - value);}

        template <typename T>
        requires (std::integral<T> || std::floating_point<T>) &&
        requires (T t, underlying_type v){v -= t;}
        endian & operator -= (T value){*this = endian(get() - value); return *this;}

        template <typename T>
        requires (std::integral<T> || std::floating_point<T>) &&
        requires (T t, underlying_type v){v * t;}
        auto operator *(T value) const{return (get() * value);}

        template <typename T>
        requires (std::integral<T> || std::floating_point<T>) &&
        requires (T t, underlying_type v){v *= t;}
        endian & operator *= (T value){*this = endian(get() * value); return *this;}

        template <typename T>
        requires (std::integral<T> || std::floating_point<T>) &&
        requires (T t, underlying_type v){v / t;}
        auto operator /(T value) const{return (get() / value);}

        template <typename T>
        requires (std::integral<T> || std::floating_point<T>) &&
        requires (T t, underlying_type v){v /= t;}
        endian & operator /= (T value){*this = endian(get() / value); return *this;}
        */

    private:

        underlying_type  value_;
    };


    template <typename data_type> using big_endian = endian<data_type, std::endian::big>;
    template <typename data_type> using network_order = big_endian<data_type>;
    template <typename data_type> using little_endian = endian<data_type, std::endian::little>;
    template <typename data_type> using native_endian = endian<data_type, std::endian::native>;
    template <typename data_type> using host_order = native_endian<data_type>;

} // namespace bcpp


//==============================================================================
template <typename data_type, std::endian endian_type>
template <typename T0, typename ... Ts>
requires ((sizeof ... (Ts) > 1) || !std::is_same_v<data_type, T0>)
bcpp::endian<data_type, endian_type>::endian
(
    // ctor constructs data_type from one or more arguments
    // where, in the event of only one argument, the first argument can not be endian<data_type>
    T0 && arg0,
    Ts && ... args
):
    endian(data_type(std::forward<T0>(arg0), std::forward<Ts>(args) ...))
{
}


//==============================================================================
template <typename data_type, std::endian endian_type>
bcpp::endian<data_type, endian_type>::endian
(
    data_type const & input
)
{
    value_ = endian_swap<std::endian::native, endian_type>(input);
}


//==============================================================================
template <typename data_type, std::endian endian_type>
auto bcpp::endian<data_type, endian_type>::operator =
(
    data_type const & input
) -> endian &
{
    value_ = endian_swap<std::endian::native, endian_type>(input);
    return *this;
}


//==============================================================================
template <typename data_type, std::endian endian_type>
auto bcpp::endian<data_type, endian_type>::get
(
) const -> underlying_type
{
    return endian_swap<endian_type, std::endian::native>(value_);
}
