#pragma once


namespace bcpp
{

    struct non_movable
    {
        non_movable() noexcept = default;
        ~non_movable() noexcept = default;
        non_movable(non_movable &&) = delete;
        non_movable & operator = (non_movable &&) = delete;
        non_movable(non_movable const &) = default;
        non_movable & operator = (non_movable const &) = default;
    }; // struct non_movable

} // namespace bcpp
