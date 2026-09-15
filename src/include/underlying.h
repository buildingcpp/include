#pragma once

#include <type_traits>
#include <utility>


namespace bcpp
{

    template <typename T>
    struct underlying_type;

    template <typename T>
        requires std::is_enum_v<T>
    struct underlying_type<T>
    {
        using type = std::underlying_type_t<T>;
    };


    template <typename T>
    concept has_underlying_type = requires { typename underlying_type<T>::type; };


    template <has_underlying_type T>
    using underlying_type_t = typename underlying_type<T>::type;


    template <typename T>
    struct fundamental_underlying_type
    {
        static_assert(std::is_fundamental_v<T>,
            "no fundamental underlying type exists for this type");
    };

    template <typename T>
    requires std::is_fundamental_v<T>
    struct fundamental_underlying_type<T> { using type = T; };

    template <has_underlying_type T>
    struct fundamental_underlying_type<T>: fundamental_underlying_type<underlying_type_t<T>> {};

    template <typename T>
    using fundamental_underlying_type_t = typename fundamental_underlying_type<T>::type;


    template <typename T>
    constexpr decltype(auto) to_underlying(T && instance)
        requires std::is_fundamental_v<std::remove_cvref_t<T>>
    {
        using value_type = std::remove_cvref_t<T>;
        if constexpr (std::is_lvalue_reference_v<T>)
            return (instance);
        else
            return value_type(std::forward<T>(instance));
    }

    template <typename T>
    constexpr underlying_type_t<T> to_underlying(T instance)
        requires std::is_enum_v<T>
    {
        return static_cast<underlying_type_t<T>>(instance);
    }


    template <typename T>
    constexpr fundamental_underlying_type_t<T> to_underlying_fundamental(T const & instance)
    {
        if constexpr (std::is_fundamental_v<T>)
            return instance;
        else if constexpr (std::is_enum_v<T>)
            return bcpp::to_underlying(instance);
        else
            return bcpp::to_underlying_fundamental(to_underlying(instance));
    }

} // namespace bcpp
