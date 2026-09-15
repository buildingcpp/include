#pragma once

#include "./non_copyable.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <memory_resource>
#include <new>
#include <span>
#include <utility>


namespace bcpp
{

    template <typename T>
    class spsc_fixed_queue : non_copyable
    {
    public:

        using type = T;
        using value_type = T;
        using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

        explicit spsc_fixed_queue
        (
            std::size_t,
            allocator_type = {}
        );

        static spsc_fixed_queue join
        (
            std::span<std::byte>
        ) noexcept;

        static constexpr std::size_t required_storage_size
        (
            std::size_t
        ) noexcept;

        static constexpr std::size_t required_storage_alignment() noexcept;

        spsc_fixed_queue(spsc_fixed_queue &&) noexcept;

        spsc_fixed_queue & operator =
        (
            spsc_fixed_queue &&
        ) noexcept;

        ~spsc_fixed_queue() noexcept;

        type pop();

        std::size_t pop
        (
            type &
        );

        std::size_t try_pop
        (
            type &
        );

        template <typename T_>
        bool push
        (
            T_ &&
        );

        template <typename ... Ts>
        bool emplace
        (
            Ts && ...
        );

        T const & front() const noexcept;

        T & front() noexcept;

        bool empty() const noexcept;

        std::size_t capacity() const noexcept;

        std::size_t size() const noexcept;

        std::size_t discard();

    private:

        static constexpr std::size_t cache_line_size = 64;

        struct alignas(cache_line_size) queue_metadata
        {
            std::size_t capacity_;
            std::size_t capacityMask_;
        };

        struct alignas(cache_line_size) consumer_state
        {
            std::atomic<std::size_t> front_;
        };

        struct alignas(cache_line_size) producer_state
        {
            std::atomic<std::size_t> back_;
            std::size_t              cachedFront_;
        };

        struct alignas(cache_line_size) instance
        {
            explicit instance(std::size_t) noexcept;

            queue_metadata metadata_;
            consumer_state consumer_;
            producer_state producer_;
        };

        spsc_fixed_queue
        (
            instance *,
            std::pmr::memory_resource *,
            std::size_t
        ) noexcept;

        static constexpr std::size_t round_capacity
        (
            std::size_t
        ) noexcept;

        static constexpr std::size_t get_queue_offset() noexcept;

        static instance * initialize_instance
        (
            std::span<std::byte>,
            std::size_t
        );

        static instance * get_instance
        (
            std::span<std::byte>
        ) noexcept;

        static T * get_queue(instance *) noexcept;

        static void destroy_instance(instance *) noexcept;

        void release() noexcept;

        instance *                 instance_;
        T *                        queue_;
        std::pmr::memory_resource * allocator_;
        std::size_t                storageSize_;
        std::size_t                capacity_;
        std::size_t                capacityMask_;

    }; // class spsc_fixed_queue

} // namespace bcpp


//=============================================================================
template <typename T>
inline bcpp::spsc_fixed_queue<T>::instance::instance
(
    std::size_t capacity
) noexcept :
    metadata_
    {
        .capacity_ = capacity,
        .capacityMask_ = capacity - 1
    },
    consumer_
    {
        .front_ = 0
    },
    producer_
    {
        .back_ = 0,
        .cachedFront_ = 0
    }
{
}


//=============================================================================
template <typename T>
inline bcpp::spsc_fixed_queue<T>::spsc_fixed_queue
(
    std::size_t capacity,
    allocator_type allocator
) :
    instance_(nullptr),
    queue_(nullptr),
    allocator_(allocator.resource()),
    storageSize_(required_storage_size(capacity)),
    capacity_(round_capacity(capacity)),
    capacityMask_(capacity_ - 1)
{
    auto * storage = static_cast<std::byte *>
    (
        allocator_->allocate
        (
            storageSize_,
            required_storage_alignment()
        )
    );

    try
    {
        instance_ = initialize_instance
        (
            std::span<std::byte>{storage, storageSize_},
            capacity_
        );
        queue_ = get_queue(instance_);
    }
    catch (...)
    {
        allocator_->deallocate
        (
            storage,
            storageSize_,
            required_storage_alignment()
        );
        throw;
    }
}


//=============================================================================
template <typename T>
inline bcpp::spsc_fixed_queue<T>::spsc_fixed_queue
(
    instance * queueInstance,
    std::pmr::memory_resource * allocator,
    std::size_t storageSize
) noexcept :
    instance_(queueInstance),
    queue_(get_queue(queueInstance)),
    allocator_(allocator),
    storageSize_(storageSize),
    capacity_(queueInstance->metadata_.capacity_),
    capacityMask_(queueInstance->metadata_.capacityMask_)
{
}


//=============================================================================
template <typename T>
inline auto bcpp::spsc_fixed_queue<T>::join
(
    std::span<std::byte> storage
) noexcept -> spsc_fixed_queue
{
    return spsc_fixed_queue{get_instance(storage), nullptr, storage.size()};
}


//=============================================================================
template <typename T>
inline constexpr std::size_t bcpp::spsc_fixed_queue<T>::required_storage_size
(
    std::size_t capacity
) noexcept
{
    return get_queue_offset() + (round_capacity(capacity) * sizeof(T));
}


//=============================================================================
template <typename T>
inline constexpr std::size_t bcpp::spsc_fixed_queue<T>::
required_storage_alignment
(
) noexcept
{
    return (alignof(T) > alignof(instance)) ? alignof(T) : alignof(instance);
}


//=============================================================================
template <typename T>
inline bcpp::spsc_fixed_queue<T>::spsc_fixed_queue
(
    spsc_fixed_queue && other
) noexcept :
    instance_(std::exchange(other.instance_, nullptr)),
    queue_(std::exchange(other.queue_, nullptr)),
    allocator_(std::exchange(other.allocator_, nullptr)),
    storageSize_(std::exchange(other.storageSize_, 0)),
    capacity_(std::exchange(other.capacity_, 0)),
    capacityMask_(std::exchange(other.capacityMask_, 0))
{
}


//=============================================================================
template <typename T>
inline auto bcpp::spsc_fixed_queue<T>::operator =
(
    spsc_fixed_queue && other
) noexcept -> spsc_fixed_queue &
{
    if ((this != &other))
    {
        release();
        instance_ = std::exchange(other.instance_, nullptr);
        queue_ = std::exchange(other.queue_, nullptr);
        allocator_ = std::exchange(other.allocator_, nullptr);
        storageSize_ = std::exchange(other.storageSize_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        capacityMask_ = std::exchange(other.capacityMask_, 0);
    }
    return *this;
}


//=============================================================================
template <typename T>
inline bcpp::spsc_fixed_queue<T>::~spsc_fixed_queue
(
) noexcept
{
    release();
}


//=============================================================================
template <typename T>
inline constexpr std::size_t bcpp::spsc_fixed_queue<T>::round_capacity
(
    std::size_t capacity
) noexcept
{
    auto result = std::size_t{1};
    while ((result < capacity))
        result <<= 1;
    return result;
}


//=============================================================================
template <typename T>
inline constexpr std::size_t bcpp::spsc_fixed_queue<T>::get_queue_offset
(
) noexcept
{
    return
    (
        (sizeof(instance) + alignof(T) - 1) /
        alignof(T)
    ) * alignof(T);
}


//=============================================================================
template <typename T>
inline auto bcpp::spsc_fixed_queue<T>::initialize_instance
(
    std::span<std::byte> storage,
    std::size_t capacity
) -> instance *
{
    auto * queueInstance = std::construct_at
    (
        get_instance(storage),
        capacity
    );

    try
    {
        std::uninitialized_value_construct_n
        (
            get_queue(queueInstance),
            capacity
        );
    }
    catch (...)
    {
        std::destroy_at(queueInstance);
        throw;
    }
    return queueInstance;
}


//=============================================================================
template <typename T>
inline auto bcpp::spsc_fixed_queue<T>::get_instance
(
    std::span<std::byte> storage
) noexcept -> instance *
{
    return reinterpret_cast<instance *>(storage.data());
}


//=============================================================================
template <typename T>
inline T * bcpp::spsc_fixed_queue<T>::get_queue
(
    instance * queueInstance
) noexcept
{
    if ((queueInstance == nullptr))
        return nullptr;

    auto * storage = reinterpret_cast<std::byte *>(queueInstance);
    return reinterpret_cast<T *>(storage + get_queue_offset());
}


//=============================================================================
template <typename T>
inline void bcpp::spsc_fixed_queue<T>::destroy_instance
(
    instance * queueInstance
) noexcept
{
    std::destroy_n
    (
        get_queue(queueInstance),
        queueInstance->metadata_.capacity_
    );
    std::destroy_at(queueInstance);
}


//=============================================================================
template <typename T>
inline void bcpp::spsc_fixed_queue<T>::release
(
) noexcept
{
    if ((allocator_ != nullptr))
    {
        destroy_instance(instance_);
        allocator_->deallocate
        (
            instance_,
            storageSize_,
            required_storage_alignment()
        );
    }

    instance_ = nullptr;
    queue_ = nullptr;
    allocator_ = nullptr;
    storageSize_ = 0;
    capacity_ = 0;
    capacityMask_ = 0;
}


//=============================================================================
template <typename T>
inline std::size_t bcpp::spsc_fixed_queue<T>::capacity
(
) const noexcept
{
    return capacity_;
}


//=============================================================================
template <typename T>
inline T & bcpp::spsc_fixed_queue<T>::front
(
) noexcept
{
    static_cast<void>
    (
        instance_->producer_.back_.load(std::memory_order_acquire)
    );
    return queue_
    [
        instance_->consumer_.front_.load(std::memory_order_relaxed) &
        capacityMask_
    ];
}


//=============================================================================
template <typename T>
inline T const & bcpp::spsc_fixed_queue<T>::front
(
) const noexcept
{
    static_cast<void>
    (
        instance_->producer_.back_.load(std::memory_order_acquire)
    );
    return queue_
    [
        instance_->consumer_.front_.load(std::memory_order_relaxed) &
        capacityMask_
    ];
}


//=============================================================================
template <typename T>
inline auto bcpp::spsc_fixed_queue<T>::pop
(
) -> type
{
    static_cast<void>
    (
        instance_->producer_.back_.load(std::memory_order_acquire)
    );
    auto front = instance_->consumer_.front_.load(std::memory_order_relaxed);

    type ret = std::move(queue_[front++ & capacityMask_]);
    instance_->consumer_.front_.store(front, std::memory_order_release);
    return ret;
}


//=============================================================================
template <typename T>
inline std::size_t bcpp::spsc_fixed_queue<T>::discard
(
)
{
    auto front = instance_->consumer_.front_.load(std::memory_order_relaxed);
    auto const back =
            instance_->producer_.back_.load(std::memory_order_acquire);

    queue_[front++ & capacityMask_] = {};
    instance_->consumer_.front_.store(front, std::memory_order_release);
    return (back - front);
}


//=============================================================================
template <typename T>
inline std::size_t bcpp::spsc_fixed_queue<T>::pop
(
    type & value
)
{
    auto front = instance_->consumer_.front_.load(std::memory_order_relaxed);
    auto const back =
            instance_->producer_.back_.load(std::memory_order_acquire);

    auto const size = (back - front);
    value = std::move(queue_[front++ & capacityMask_]);
    instance_->consumer_.front_.store(front, std::memory_order_release);
    return size;
}


//=============================================================================
template <typename T>
inline std::size_t bcpp::spsc_fixed_queue<T>::try_pop
(
    type & value
)
{
    auto front = instance_->consumer_.front_.load(std::memory_order_relaxed);
    auto const back =
            instance_->producer_.back_.load(std::memory_order_acquire);
    auto const size = (back - front);

    if ((size > 0))
    {
        value = std::move(queue_[front++ & capacityMask_]);
        instance_->consumer_.front_.store(front, std::memory_order_release);
    }
    return size;
}


//=============================================================================
template <typename T>
template <typename ... Ts>
inline bool bcpp::spsc_fixed_queue<T>::emplace
(
    Ts && ... args
)
{
    auto & producer = instance_->producer_;
    auto const back = producer.back_.load(std::memory_order_relaxed);

    if (((back - producer.cachedFront_) >= capacity_))
    {
        producer.cachedFront_ =
                instance_->consumer_.front_.load(std::memory_order_acquire);
        if (((back - producer.cachedFront_) >= capacity_))
            return false;
    }

    queue_[back & capacityMask_] = T(std::forward<Ts>(args) ...);
    producer.back_.store(back + 1, std::memory_order_release);
    return true;
}


//=============================================================================
template <typename T>
template <typename T_>
inline bool bcpp::spsc_fixed_queue<T>::push
(
    T_ && value
)
{
    auto & producer = instance_->producer_;
    auto const back = producer.back_.load(std::memory_order_relaxed);

    if (((back - producer.cachedFront_) >= capacity_))
    {
        producer.cachedFront_ =
                instance_->consumer_.front_.load(std::memory_order_acquire);
        if (((back - producer.cachedFront_) >= capacity_))
            return false;
    }

    queue_[back & capacityMask_] = std::forward<T_>(value);
    producer.back_.store(back + 1, std::memory_order_release);
    return true;
}


//=============================================================================
template <typename T>
inline bool bcpp::spsc_fixed_queue<T>::empty
(
) const noexcept
{
    auto const front =
            instance_->consumer_.front_.load(std::memory_order_acquire);
    auto const back =
            instance_->producer_.back_.load(std::memory_order_acquire);
    return (back == front);
}


//=============================================================================
template <typename T>
inline std::size_t bcpp::spsc_fixed_queue<T>::size
(
) const noexcept
{
    auto const front =
            instance_->consumer_.front_.load(std::memory_order_acquire);
    auto const back =
            instance_->producer_.back_.load(std::memory_order_acquire);
    return (back - front);
}
