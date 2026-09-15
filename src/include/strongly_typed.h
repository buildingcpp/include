#pragma once

#include "underlying.h"

#include <compare>
#include <concepts>
#include <cstdint>
#include <ostream>
#include <type_traits>
#include <utility>

namespace bcpp
{
    namespace detail
    {
        // Supplies T{} without requiring T itself to be a template value type.
        template <typename T>
        struct strongly_typed_default
        {
            constexpr operator T() const noexcept(std::is_nothrow_default_constructible_v<T>)
            {
                return T{};
            }
        };
    }

    template <typename T, typename Tag,
        std::convertible_to<T> auto DefaultValue = detail::strongly_typed_default<T>{}, bool Validatable = false>
    class strongly_typed
    {
    public:
        using value_type = T;
        using tag_type = Tag;
        static constexpr auto default_value = DefaultValue;

        constexpr strongly_typed() = default;

        template <typename U = T>
        constexpr explicit strongly_typed(U && value)
            noexcept(std::is_nothrow_constructible_v<T, U &&>)
            requires (not std::same_as<std::remove_cvref_t<U>, strongly_typed> && std::convertible_to<U &&, T>):
            value_(std::forward<U>(value))
        {
        }

        constexpr bool is_valid() const
            requires Validatable
        {
            return not (value_ == static_cast<T>(default_value));
        }

        constexpr explicit operator value_type &() noexcept
        {
            return value_;
        }

        constexpr explicit operator value_type const &() const noexcept
        {
            return value_;
        }

        constexpr decltype(auto) operator ==(strongly_typed const & other) const
            noexcept(noexcept(value_ == other.value_))
            requires requires (T const & left, T const & right) { left == right; }
        {
            return value_ == other.value_;
        }

        constexpr decltype(auto) operator !=(strongly_typed const & other) const
            noexcept(noexcept(value_ != other.value_))
            requires requires (T const & left, T const & right) { left != right; }
        {
            return value_ != other.value_;
        }

        constexpr decltype(auto) operator <(strongly_typed const & other) const
            noexcept(noexcept(value_ < other.value_))
            requires requires (T const & left, T const & right) { left < right; }
        {
            return value_ < other.value_;
        }

        constexpr decltype(auto) operator <=(strongly_typed const & other) const
            noexcept(noexcept(value_ <= other.value_))
            requires requires (T const & left, T const & right) { left <= right; }
        {
            return value_ <= other.value_;
        }

        constexpr decltype(auto) operator >(strongly_typed const & other) const
            noexcept(noexcept(value_ > other.value_))
            requires requires (T const & left, T const & right) { left > right; }
        {
            return value_ > other.value_;
        }

        constexpr decltype(auto) operator >=(strongly_typed const & other) const
            noexcept(noexcept(value_ >= other.value_))
            requires requires (T const & left, T const & right) { left >= right; }
        {
            return value_ >= other.value_;
        }

        constexpr decltype(auto) operator <=>(strongly_typed const & other) const
            noexcept(noexcept(value_ <=> other.value_))
            requires requires (T const & left, T const & right) { left <=> right; }
        {
            return value_ <=> other.value_;
        }

        constexpr strongly_typed operator +(strongly_typed const & other) const
            noexcept(noexcept(strongly_typed(value_ + other.value_)))
            requires requires (T const & left, T const & right) { strongly_typed(left + right); }
        {
            return strongly_typed(value_ + other.value_);
        }

        constexpr strongly_typed operator -(strongly_typed const & other) const
            noexcept(noexcept(strongly_typed(value_ - other.value_)))
            requires requires (T const & left, T const & right) { strongly_typed(left - right); }
        {
            return strongly_typed(value_ - other.value_);
        }

        constexpr strongly_typed operator *(strongly_typed const & other) const
            noexcept(noexcept(strongly_typed(value_ * other.value_)))
            requires requires (T const & left, T const & right) { strongly_typed(left * right); }
        {
            return strongly_typed(value_ * other.value_);
        }

        constexpr strongly_typed operator /(strongly_typed const & other) const
            noexcept(noexcept(strongly_typed(value_ / other.value_)))
            requires requires (T const & left, T const & right) { strongly_typed(left / right); }
        {
            return strongly_typed(value_ / other.value_);
        }

        constexpr strongly_typed operator %(strongly_typed const & other) const
            noexcept(noexcept(strongly_typed(value_ % other.value_)))
            requires requires (T const & left, T const & right) { strongly_typed(left % right); }
        {
            return strongly_typed(value_ % other.value_);
        }

        constexpr strongly_typed operator +() const
            noexcept(noexcept(strongly_typed(+value_)))
            requires requires (T const & value) { strongly_typed(+value); }
        {
            return strongly_typed(+value_);
        }

        constexpr strongly_typed operator -() const
            noexcept(noexcept(strongly_typed(-value_)))
            requires requires (T const & value) { strongly_typed(-value); }
        {
            return strongly_typed(-value_);
        }

        constexpr strongly_typed & operator +=(strongly_typed const & other)
            noexcept(noexcept(value_ += other.value_))
            requires requires (T & left, T const & right) { left += right; }
        {
            value_ += other.value_;
            return *this;
        }

        constexpr strongly_typed & operator -=(strongly_typed const & other)
            noexcept(noexcept(value_ -= other.value_))
            requires requires (T & left, T const & right) { left -= right; }
        {
            value_ -= other.value_;
            return *this;
        }

        constexpr strongly_typed & operator *=(strongly_typed const & other)
            noexcept(noexcept(value_ *= other.value_))
            requires requires (T & left, T const & right) { left *= right; }
        {
            value_ *= other.value_;
            return *this;
        }

        constexpr strongly_typed & operator /=(strongly_typed const & other)
            noexcept(noexcept(value_ /= other.value_))
            requires requires (T & left, T const & right) { left /= right; }
        {
            value_ /= other.value_;
            return *this;
        }

        constexpr strongly_typed & operator %=(strongly_typed const & other)
            noexcept(noexcept(value_ %= other.value_))
            requires requires (T & left, T const & right) { left %= right; }
        {
            value_ %= other.value_;
            return *this;
        }

        constexpr strongly_typed & operator ++()
            noexcept(noexcept(++value_))
            requires requires (T & value) { ++value; }
        {
            ++value_;
            return *this;
        }

        constexpr strongly_typed operator ++(std::int32_t)
            noexcept(noexcept(strongly_typed(value_++)))
            requires requires (T & value) { strongly_typed(value++); }
        {
            return strongly_typed(value_++);
        }

        constexpr strongly_typed & operator --()
            noexcept(noexcept(--value_))
            requires requires (T & value) { --value; }
        {
            --value_;
            return *this;
        }

        constexpr strongly_typed operator --(std::int32_t)
            noexcept(noexcept(strongly_typed(value_--)))
            requires requires (T & value) { strongly_typed(value--); }
        {
            return strongly_typed(value_--);
        }

    private:
        T value_ = default_value;
    };

    template <typename T>
    concept strongly_typed_concept = requires (std::remove_cvref_t<T> & value)
    {
        typename underlying_type_t<std::remove_cvref_t<T>>;
        typename std::remove_cvref_t<T>::tag_type;
        std::remove_cvref_t<T>::default_value;
        static_cast<underlying_type_t<std::remove_cvref_t<T>> &>(value);
    };

    template <typename T>
    concept validatable_strongly_typed_concept = strongly_typed_concept<T> &&
        requires (std::remove_cvref_t<T> const & value)
        {
            { value.is_valid() } -> std::convertible_to<bool>;
        };

    template <typename T, typename Tag, std::convertible_to<T> auto InvalidValue>
    using validatable_strongly_typed = strongly_typed<T, Tag, InvalidValue, true>;

    template <typename T, typename Tag, auto DefaultValue, bool Validatable>
    struct underlying_type<strongly_typed<T, Tag, DefaultValue, Validatable>>
    {
        using type = T;
    };

    template <strongly_typed_concept T>
    constexpr decltype(auto) to_underlying(T & instance)
    {
        using value_type = underlying_type_t<std::remove_cv_t<T>>;
        using reference = std::conditional_t<std::is_const_v<T>, value_type const &, value_type &>;
        auto & value = static_cast<reference>(instance);
        if constexpr (requires { to_underlying(value); })
            return to_underlying(value);
        else
            return (value);
    }

    template <strongly_typed_concept T>
    constexpr auto to_underlying(T && instance)
        requires (not std::is_lvalue_reference_v<T>)
    {
        using value_type = underlying_type_t<std::remove_cvref_t<T>>;
        using reference = std::conditional_t<std::is_const_v<std::remove_reference_t<T>>,
            value_type const &, value_type &>;
        auto & value = static_cast<reference>(instance);
        if constexpr (requires { to_underlying(std::move(value)); })
            return to_underlying(std::move(value));
        else
            return value_type(std::move(value));
    }

    template <strongly_typed_concept T>
    std::ostream & operator <<(std::ostream & stream, T const & value)
        requires requires { stream << static_cast<underlying_type_t<T> const &>(value); }
    {
        return stream << static_cast<underlying_type_t<T> const &>(value);
    }

    namespace compile_time_validation::strongly_typed_checks
    {
        using number = strongly_typed<std::int32_t, struct number_tag>;
        using other_number = strongly_typed<std::int32_t, struct other_number_tag>;
        using identifier = validatable_strongly_typed<std::uint16_t, struct identifier_tag, -1>;
        using nested_number = strongly_typed<number, struct nested_number_tag>;

        static_assert(strongly_typed_concept<number> && strongly_typed_concept<number const &>);
        static_assert(not strongly_typed_concept<std::int32_t>);
        static_assert(validatable_strongly_typed_concept<identifier> &&
            not validatable_strongly_typed_concept<number>);

        // Arithmetic preserves the tag; mutation returns the wrapper itself.
        static_assert([]
        {
            number a{12}, b{5};
            if (a + b != number{17} || a - b != number{7} || a * b != number{60} ||
                a / b != number{2} || a % b != number{2} || +a != a || -a != number{-12})
                return false;
            if (not (a > b && a >= b && b < a && b <= a && a != b && a == a && (a <=> b) > 0))
                return false;

            if (&(a += b) != &a || a != number{17}) return false;
            if (&(a -= b) != &a || a != number{12}) return false;
            if (&(a *= b) != &a || a != number{60}) return false;
            if (&(a /= b) != &a || a != number{12}) return false;
            if (&(a %= b) != &a || a != number{2}) return false;
            if (a++ != number{2} || a != number{3} || &++a != &a || a != number{4}) return false;
            if (a-- != number{4} || a != number{3} || &--a != &a || a != number{2}) return false;

            static_cast<std::int32_t &>(a) = 42;
            auto const & readOnly = a;
            return static_cast<std::int32_t>(readOnly) == 42 &&
                &static_cast<std::int32_t const &>(readOnly) == &static_cast<std::int32_t &>(a);
        }());

        template <typename Left, typename Right>
        concept adds = requires (Left left, Right right) { left + right; };

        template <typename Left, typename Right>
        concept compares = requires (Left left, Right right) { left == right; };

        template <typename T>
        concept validates = requires (T const & value) { value.is_valid(); };

        // No implicit escape to T or mixing of tags/raw values in expressions.
        static_assert(not std::convertible_to<number, std::int32_t>);
        static_assert(not std::convertible_to<std::int32_t, number>);
        static_assert(not adds<number, other_number> && not adds<number, std::int32_t>);
        static_assert(not adds<other_number, number> && not adds<std::int32_t, number>);
        static_assert(not compares<number, other_number> && not compares<number, std::int32_t>);
        static_assert(not compares<other_number, number> && not compares<std::int32_t, number>);
        static_assert(std::same_as<decltype(number{} + number{}), number>);
        static_assert(sizeof(number) == sizeof(std::int32_t));

        // The invalid sentinel is converted to T before both storage and comparison.
        static_assert(static_cast<std::int32_t>(number{}) == 0);
        static_assert(not validates<number> && validates<identifier>);
        static_assert(not identifier{}.is_valid() && identifier{0}.is_valid());
        static_assert(static_cast<std::uint16_t>(identifier{}) == static_cast<std::uint16_t>(identifier::default_value));
        static_assert(bcpp::to_underlying(nested_number{number{42}}) == 42);
        static_assert(bcpp::to_underlying_fundamental(nested_number{number{42}}) == 42);
        static_assert(bcpp::to_underlying_fundamental(std::int32_t{42}) == 42);
        static_assert([]
        {
            nested_number nested{number{42}};
            auto & value = bcpp::to_underlying(nested);
            auto const & readOnly = nested;
            if (&value != &bcpp::to_underlying(readOnly))
                return false;
            value = 17;
            return nested == nested_number{number{17}};
        }());

        static_assert(not std::is_reference_v<decltype(bcpp::to_underlying(nested_number{}))>);
        static_assert(not std::is_reference_v<decltype(bcpp::to_underlying(std::declval<nested_number const &&>()))>);

        // A non-structural UDT still supports the implicit T{} default.
        class value
        {
        public:
            constexpr value() = default;
            constexpr value(std::int32_t value): value_(value) {}
            constexpr bool operator ==(value const &) const = default;

        private:
            std::int32_t value_ = 7;
        };

        using tagged_value = strongly_typed<value, struct value_tag>;
        using validatable_value = validatable_strongly_typed<value, struct validatable_value_tag, 7>;
        static_assert(tagged_value{} == tagged_value{7});
        static_assert(tagged_value{9} != tagged_value{7});
        static_assert(not adds<tagged_value, tagged_value>);
        static_assert(not validatable_value{}.is_valid() && validatable_value{9}.is_valid());

        // Forward directly into T and preserve ordinary wrapper copy/move behavior,
        // even when T has a constructor that could accept the wrapper itself.
        struct counted_value
        {
            std::int32_t copies_ = 0;
            std::int32_t moves_ = 0;

            constexpr counted_value() = default;
            constexpr counted_value(counted_value const & other) noexcept(false):
                copies_(other.copies_ + 1), moves_(other.moves_) {}
            constexpr counted_value(counted_value && other) noexcept:
                copies_(other.copies_), moves_(other.moves_ + 1) {}
            constexpr counted_value & operator =(counted_value const &) = default;
            constexpr counted_value & operator =(counted_value &&) = default;

            template <typename U>
            constexpr counted_value(U const &): copies_(-1), moves_(-1) {}
        };

        using counted = strongly_typed<counted_value, struct counted_tag>;
        static_assert([]
        {
            counted_value source;
            counted copied{source};
            counted moved{std::move(source)};
            counted wrapperCopy{copied};
            counted wrapperMove{std::move(moved)};
            auto const & copy = static_cast<counted_value const &>(wrapperCopy);
            auto const & move = static_cast<counted_value const &>(wrapperMove);
            if (copy.copies_ != 2 || copy.moves_ != 0 || move.copies_ != 0 || move.moves_ != 2)
                return false;

            copied = wrapperMove;
            moved = std::move(wrapperCopy);
            return static_cast<counted_value const &>(copied).moves_ == 2 &&
                static_cast<counted_value const &>(moved).copies_ == 2;
        }());

        // Extraction borrows stored lvalues and owns results from temporary wrappers.
        static_assert([]
        {
            counted original;
            auto & reference = bcpp::to_underlying(original);
            auto const & readOnly = original;
            if (&reference != &static_cast<counted_value &>(original) ||
                &bcpp::to_underlying(readOnly) != &reference)
                return false;

            auto copy = bcpp::to_underlying(readOnly);
            auto moved = bcpp::to_underlying(std::move(original));
            auto copiedTemporary = bcpp::to_underlying(std::move(readOnly));
            return copy.copies_ == 1 && moved.moves_ == 1 && copiedTemporary.copies_ == 1;
        }());
        static_assert(not std::is_reference_v<decltype(bcpp::to_underlying(counted{}))>);
        static_assert(not std::is_reference_v<decltype(bcpp::to_underlying(std::declval<counted const &&>()))>);

        // Construction follows T's exception specification; validity may throw.
        static_assert(not noexcept(counted{std::declval<counted_value const &>()}));
        static_assert(noexcept(counted{std::declval<counted_value &&>()}));
        static_assert(noexcept(number{1} + number{2}));
        static_assert(not noexcept(std::declval<validatable_value const &>().is_valid()));

        // Underlying conversions may throw.
        static_assert(not noexcept(bcpp::to_underlying(std::declval<counted const &>())));
        static_assert(not noexcept(bcpp::to_underlying_fundamental(std::declval<nested_number const &>())));
    }
}
