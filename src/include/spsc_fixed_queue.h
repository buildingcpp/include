#pragma once

#include "./non_copyable.h"

#include <concepts>
#include <cstddef>
#include <vector>


namespace bcpp
{

    template <typename T>
    class spsc_fixed_queue : non_copyable
    {
    public:

        using type = T;

        spsc_fixed_queue
        (
            std::size_t
        );

        spsc_fixed_queue(spsc_fixed_queue &&) = default;
        spsc_fixed_queue & operator = (spsc_fixed_queue &&) = default;
        ~spsc_fixed_queue() = default;

        type pop();

        std::size_t pop
        (
            type &
        );

        std::size_t try_pop
        (
            type &
        );

        bool push
        (
            T &
        );

        T const & front() const;
        
        T & front();

        bool empty() const;

        std::size_t capacity() const;

        std::size_t size() const;

    private:

        std::size_t volatile        front_;

        std::size_t volatile        back_;

        std::size_t                 capacity_;
        std::size_t                 capacityMask_;

        std::vector<type>           queue_;
    };

} // namespace bcpp


//==============================================================================
template <typename T>
bcpp::spsc_fixed_queue<T>::spsc_fixed_queue
(
    std::size_t capacity
):
    front_(0), 
    back_(0)
{
    capacity_ = 1;
    while (capacity_ < capacity)
        capacity_ <<= 1;
    capacityMask_ = capacity_ - 1;
    queue_.resize(capacity_);
}


//==============================================================================
template <typename T>
inline std::size_t bcpp::spsc_fixed_queue<T>::capacity
(
) const
{
    return capacity_;
}


//==============================================================================
template <typename T>
T & bcpp::spsc_fixed_queue<T>::front
(
)
{
    return queue_[front & capacityMask_];
}


//==============================================================================
template <typename T>
T const & bcpp::spsc_fixed_queue<T>::front
(
) const
{
    return queue_[front & capacityMask_];
}


//==============================================================================
template <typename T>
inline auto bcpp::spsc_fixed_queue<T>::pop
(
) -> type
{
    std::size_t front = front_;
    type ret = std::move(queue_[front++ & capacityMask_]);
    front_ = front;
    return ret;
}


//==============================================================================
template <typename T>
inline std::size_t bcpp::spsc_fixed_queue<T>::pop
(
    type & value
)
{
    auto front = front_;
    auto size = (back_ - front);
    value = std::move(queue_[front++ & capacityMask_]);
    front_ = front;
    return size;
}


//==============================================================================
template <typename T>
inline std::size_t bcpp::spsc_fixed_queue<T>::try_pop
(
    type & value
)
{
    if (auto front = front_, size = (back_ - front); size > 0)
    {
        value = std::move(queue_[front++ & capacityMask_]);
        front_ = front;
        return size;
    }
    return 0;
}


//==============================================================================
template <typename T>
inline bool bcpp::spsc_fixed_queue<T>::push
(
    T & value
)
{
    if (std::size_t back = back_; (back - front_) < capacity_)
    {
        queue_[back++ & capacityMask_] = std::move(value);
        back_ = back;
        return true;
    }
    return false;
}


//==============================================================================
template <typename T>
inline bool bcpp::spsc_fixed_queue<T>::empty
(
) const
{
    return (back_ == front_);
}


//==============================================================================
template <typename T>
inline std::size_t bcpp::spsc_fixed_queue<T>::size
(
) const
{
    return (back_ - front_);
}
