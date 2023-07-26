#pragma once


namespace bcpp
{

    struct non_copyable
    {
        non_copyable() noexcept = default;
        ~non_copyable() noexcept = default;
        non_copyable(non_copyable const &) = delete;
        non_copyable & operator = (non_copyable const &) = delete;
    }; // struct non_copyable

} // namespace bcpp
