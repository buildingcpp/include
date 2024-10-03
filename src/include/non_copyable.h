#pragma once


namespace bcpp
{

    struct non_copyable
    {
        non_copyable() noexcept = default;
        ~non_copyable() noexcept = default;
        non_copyable(non_copyable const &) = delete;
        non_copyable & operator = (non_copyable const &) = delete;
        non_copyable(non_copyable &&) = default;
        non_copyable & operator = (non_copyable &&) = default;
    }; // struct non_copyable

} // namespace bcpp
