#pragma once

#include "./byte_swap.h"
#include "./endian_swap.h"

#include <include/underlying.h>

#include <concepts>
#include <ostream>
#include <type_traits>
#include <bit>
#include <utility>


namespace bcpp
{

    template <typename data_type, std::endian endian_type>
    class endian;


    //=========================================================================
    template <typename T>
    concept endian_concept = std::is_same_v<T, endian<typename T::value_type, T::type>>;


    //==============================================================================
    template <typename data_type, std::endian endian_type>
    class endian
    {
    public:

        using value_type = data_type;
        static auto constexpr type = endian_type;

        template <typename, std::endian> friend class endian;

        endian() = default;

        template <typename T>
        explicit endian
        (
            T &&
        )
            requires std::same_as<std::remove_cvref_t<T>, value_type> &&
                requires (T && input) { endian_swap<std::endian::native, endian_type>(std::forward<T>(input)); };

        endian
        (
            endian const &
        ) = default;

        endian
        (
            endian &&
        ) = default;

        template <std::endian other_endian>
        explicit endian
        (
            endian<value_type, other_endian> const &
        )
            requires requires (value_type const & value) { endian_swap<other_endian, endian_type>(value); };

        template <std::endian other_endian>
        explicit endian
        (
            endian<value_type, other_endian> &&
        )
            requires requires (value_type && value) { endian_swap<other_endian, endian_type>(std::move(value)); };

        endian & operator =
        (
            endian const &
        ) = default;

        endian & operator =
        (
            endian &&
        ) = default;

        template <std::endian other_endian>
        endian & operator =
        (
            endian<value_type, other_endian> const &
        )
            requires requires (value_type & value, value_type const & input)
            {
                value = endian_swap<other_endian, endian_type>(input);
            };

        template <std::endian other_endian>
        endian & operator =
        (
            endian<value_type, other_endian> &&
        )
            requires requires (value_type & value, value_type && input)
            {
                value = endian_swap<other_endian, endian_type>(std::move(input));
            };

        template <typename T>
        endian & operator =
        (
            T &&
        ) requires std::same_as<std::remove_cvref_t<T>, value_type> &&
                requires (value_type & value, T && input)
                {
                    value = endian_swap<std::endian::native, endian_type>(std::forward<T>(input));
                };

        explicit operator value_type
        (
        ) const
            requires requires (value_type const & value) { endian_swap<endian_type, std::endian::native>(value); }
        {
            return endian_swap<endian_type, std::endian::native>(value_);
        }

        template <auto member>
        auto get() const
            requires std::is_member_object_pointer_v<decltype(member)> &&
                requires (value_type const & value) { endian_swap<endian_type, std::endian::native>(value.*member); };

        template <typename Member> requires
        (
            std::is_member_object_pointer_v<Member> &&
            requires
            (
                value_type const & value,
                Member member
            ) { endian_swap<endian_type, std::endian::native>(value.*member); }
        )
        auto get(Member member) const
        {
            return endian_swap<endian_type, std::endian::native>(value_.*member);
        }

        template <endian_concept T>
        bool operator ==
        (
            T const & other
        ) const
            requires requires
            {
                { this->operator value_type() == other.operator typename T::value_type() } -> std::convertible_to<bool>;
            }
        {
            return (this->operator value_type() == other.operator typename T::value_type());
        }

        template <typename T>
        bool operator ==
        (
            T const & other
        ) const
            requires (not endian_concept<T>) && requires
            {
                { this->operator value_type() == other } -> std::convertible_to<bool>;
            }
        {
            return (this->operator value_type() == other);
        }

        template <endian_concept T>
        auto operator <=>
        (
            T const & other
        ) const
            requires requires
            {
                this->operator value_type() <=> other.operator typename T::value_type();
            }
        {
            return (this->operator value_type() <=> other.operator typename T::value_type());
        }

        template <typename T>
        auto operator <=>
        (
            T const & other
        ) const
            requires (not endian_concept<T>) && requires
            {
                this->operator value_type() <=> other;
            }
        {
            return (this->operator value_type() <=> other);
        }

    private:

        value_type  value_;
    };


    template <typename data_type> using big_endian = endian<data_type, std::endian::big>;
    template <typename data_type> using network_order = big_endian<data_type>;
    template <typename data_type> using little_endian = endian<data_type, std::endian::little>;
    template <typename data_type> using native_endian = endian<data_type, std::endian::native>;
    template <typename data_type> using host_order = native_endian<data_type>;


    //=========================================================================
    template <typename data_type, std::endian endian_type>
    struct underlying_type<endian<data_type, endian_type>>
    {
        using type = data_type;
    };


    //=========================================================================
    template <endian_concept T>
    constexpr auto to_underlying(T const & value)
        requires requires { value.operator typename T::value_type(); }
    {
        if constexpr (requires { to_underlying(value.operator typename T::value_type()); })
            return to_underlying(value.operator typename T::value_type());
        else
            return value.operator typename T::value_type();
    }


    //=========================================================================
    template <endian_concept T>
    std::ostream & operator <<(std::ostream & stream, T const & value)
        requires requires { stream << value.operator typename T::value_type(); }
    {
        return stream << value.operator typename T::value_type();
    }


    //=========================================================================
    template <auto member, typename T, std::endian E>
    auto get(endian<T, E> const & value)
    {
        return value.template get<member>();
    }

} // namespace bcpp


//==============================================================================
template <typename data_type, std::endian endian_type>
template <typename T>
bcpp::endian<data_type, endian_type>::endian
(
    T && input
)
    requires std::same_as<std::remove_cvref_t<T>, value_type> &&
        requires (T && input) { endian_swap<std::endian::native, endian_type>(std::forward<T>(input)); }:
    value_(endian_swap<std::endian::native, endian_type>(std::forward<T>(input)))
{
}


//==============================================================================
template <typename data_type, std::endian endian_type>
template <std::endian other_endian>
bcpp::endian<data_type, endian_type>::endian
(
    endian<value_type, other_endian> const & other
)
    requires requires (value_type const & value) { endian_swap<other_endian, endian_type>(value); }:
    value_(endian_swap<other_endian, endian_type>(other.value_))
{
}


//==============================================================================
template <typename data_type, std::endian endian_type>
template <std::endian other_endian>
bcpp::endian<data_type, endian_type>::endian
(
    endian<value_type, other_endian> && other
)
    requires requires (value_type && value) { endian_swap<other_endian, endian_type>(std::move(value)); }:
    value_(endian_swap<other_endian, endian_type>(std::move(other.value_)))
{
}


//==============================================================================
template <typename data_type, std::endian endian_type>
template <auto member>
auto bcpp::endian<data_type, endian_type>::get
(
) const
    requires std::is_member_object_pointer_v<decltype(member)> &&
        requires (value_type const & value) { endian_swap<endian_type, std::endian::native>(value.*member); }
{
    return endian_swap<endian_type, std::endian::native>(value_.*member);
}


//==============================================================================
template <typename data_type, std::endian endian_type>
template <typename T>
auto bcpp::endian<data_type, endian_type>::operator =
(
    T && input
) -> endian &
    requires std::same_as<std::remove_cvref_t<T>, value_type> &&
        requires (value_type & value, T && input)
        {
            value = endian_swap<std::endian::native, endian_type>(std::forward<T>(input));
        }
{
    value_ = endian_swap<std::endian::native, endian_type>(std::forward<T>(input));
    return *this;
}


//==============================================================================
template <typename data_type, std::endian endian_type>
template <std::endian other_endian>
auto bcpp::endian<data_type, endian_type>::operator =
(
    endian<value_type, other_endian> const & other
) -> endian &
    requires requires (value_type & value, value_type const & input)
    {
        value = endian_swap<other_endian, endian_type>(input);
    }
{
    value_ = endian_swap<other_endian, endian_type>(other.value_);
    return *this;
}


//==============================================================================
template <typename data_type, std::endian endian_type>
template <std::endian other_endian>
auto bcpp::endian<data_type, endian_type>::operator =
(
    endian<value_type, other_endian> && other
) -> endian &
    requires requires (value_type & value, value_type && input)
    {
        value = endian_swap<other_endian, endian_type>(std::move(input));
    }
{
    value_ = endian_swap<other_endian, endian_type>(std::move(other.value_));
    return *this;
}
