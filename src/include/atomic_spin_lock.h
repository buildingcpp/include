#pragma once

#include <include/non_copyable.h>

#include <thread>
#include <atomic>
#include <cstdint>
#include <emmintrin.h>


namespace bcpp
{

    class atomic_spin_lock :
        non_copyable
    {
    public:

        using value_type = std::thread::id;

        atomic_spin_lock() = default;
        ~atomic_spin_lock() = default;
        atomic_spin_lock(atomic_spin_lock &&) = default;
        atomic_spin_lock & operator = (atomic_spin_lock &&) = default;

        void lock();

        void unlock();

        bool try_lock();

    private:

        std::atomic<value_type> value_{};

    }; // atomic_spin_lock

} // namespace bcpp


//=============================================================================
inline void bcpp::atomic_spin_lock::lock
(
)
{
    static value_type const invalid_value = {};
    auto expected = invalid_value;
    auto desired = std::this_thread::get_id();
    while (not value_.compare_exchange_strong(expected, desired))
        _mm_pause();
}


//=============================================================================
inline void bcpp::atomic_spin_lock::unlock
(
)
{
    static value_type const invalid_value = {};
    auto expected = std::this_thread::get_id();
    value_.compare_exchange_strong(expected, invalid_value);
}


//=============================================================================
inline bool bcpp::atomic_spin_lock::try_lock
(
)
{
    static value_type const invalid_value = {};
    auto expected = invalid_value;
    auto desired = std::this_thread::get_id();
    return value_.compare_exchange_strong(expected, desired);
}
