#include <include/spsc_fixed_queue.h>

#include <atomic>
#include <barrier>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory_resource>
#include <span>
#include <string>
#include <thread>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>


namespace
{

    class test_context
    {
    public:

        void expect(bool condition, char const * description)
        {
            if ((!condition))
            {
                std::cerr << "FAILED: " << description << '\n';
                ++failureCount_;
            }
        }

        int result() const noexcept
        {
            return failureCount_;
        }

    private:

        int failureCount_{};
    };


    class fixed_memory_resource final : public std::pmr::memory_resource
    {
    public:

        explicit fixed_memory_resource(std::span<std::byte> storage) noexcept :
            storage_(storage)
        {
        }

        bool was_allocated() const noexcept
        {
            return wasAllocated_;
        }

        bool was_deallocated() const noexcept
        {
            return wasDeallocated_;
        }

    private:

        void * do_allocate
        (
            std::size_t bytes,
            std::size_t alignment
        ) override
        {
            auto const address = reinterpret_cast<std::uintptr_t>
            (
                storage_.data()
            );
            if
            (
                (wasAllocated_) ||
                (bytes > storage_.size()) ||
                ((address % alignment) != 0)
            )
            {
                throw std::bad_alloc{};
            }

            wasAllocated_ = true;
            return storage_.data();
        }

        void do_deallocate
        (
            void *,
            std::size_t,
            std::size_t
        ) override
        {
            wasDeallocated_ = true;
        }

        bool do_is_equal
        (
            std::pmr::memory_resource const & other
        ) const noexcept override
        {
            return (this == &other);
        }

        std::span<std::byte> storage_;
        bool                 wasAllocated_{};
        bool                 wasDeallocated_{};
    };


    struct alignas(128) aligned_value
    {
        std::uint64_t value_{};
    };


    struct throwing_value
    {
        throwing_value()
        {
            ++constructionCount_;
            if ((constructionCount_ == 3))
                throw 1;
        }

        ~throwing_value()
        {
            ++destructionCount_;
        }

        static inline int constructionCount_{};
        static inline int destructionCount_{};
    };


    void test_local_queue(test_context & test)
    {
        bcpp::spsc_fixed_queue<int> queue{3};

        test.expect
        (
            (queue.capacity() == 4),
            "capacity rounds to a power of two"
        );
        test.expect((queue.empty()), "new queue is empty");
        test.expect((queue.size() == 0), "new queue has size zero");

        test.expect((queue.push(1)), "first push succeeds");
        test.expect((queue.push(2)), "second push succeeds");
        test.expect((queue.emplace(3)), "emplace succeeds");
        test.expect((queue.push(4)), "push up to capacity succeeds");
        test.expect((!queue.push(5)), "push beyond capacity fails");
        test.expect((queue.front() == 1), "front returns the oldest value");
        test.expect
        (
            (queue.pop() == 1),
            "value-returning pop preserves FIFO order"
        );

        auto value = 0;
        test.expect((queue.pop(value) == 3), "pop reports size before removal");
        test.expect((value == 2), "output pop preserves FIFO order");
        test.expect
        (
            (queue.discard() == 1),
            "discard reports size after removal"
        );
        test.expect
        (
            (queue.try_pop(value) == 1),
            "try_pop reports size before removal"
        );
        test.expect((value == 4), "try_pop returns the remaining value");
        test.expect
        (
            (queue.try_pop(value) == 0),
            "try_pop reports an empty queue"
        );
        test.expect((queue.empty()), "queue is empty after all removals");

        bcpp::spsc_fixed_queue<std::string> strings{2};
        test.expect
        (
            (strings.emplace("alpha")),
            "non-trivial value emplace succeeds"
        );
        test.expect
        (
            (strings.pop() == "alpha"),
            "non-trivial value survives pop"
        );

        bcpp::spsc_fixed_queue<int> source{2};
        test.expect((source.push(71)), "move source accepts a value");
        auto moved = std::move(source);
        test.expect
        (
            (moved.pop() == 71),
            "move construction transfers ownership"
        );

        bcpp::spsc_fixed_queue<int> destination{8};
        test.expect
        (
            (destination.push(99)),
            "move destination owns initial storage"
        );
        destination = std::move(moved);
        test.expect
        (
            (destination.empty()),
            "move assignment replaces prior storage"
        );

        bcpp::spsc_fixed_queue<int> minimum{0};
        test.expect
        (
            (minimum.capacity() == 1),
            "zero requested capacity becomes one"
        );

        bcpp::spsc_fixed_queue<aligned_value> aligned{2};
        test.expect
        (
            (aligned.push(aligned_value{})),
            "over-aligned value pushes"
        );
        test.expect
        (
            ((reinterpret_cast<std::uintptr_t>(&aligned.front()) % 128) == 0),
            "over-aligned values retain their alignment"
        );
    }


    void test_failed_construction(test_context & test)
    {
        using queue_type = bcpp::spsc_fixed_queue<throwing_value>;
        constexpr auto capacity = std::size_t{4};
        alignas(64) std::byte storage
        [
            queue_type::required_storage_size(capacity)
        ];
        fixed_memory_resource resource{storage};

        throwing_value::constructionCount_ = 0;
        throwing_value::destructionCount_ = 0;
        auto exceptionCaught = false;
        try
        {
            auto queue = queue_type
            {
                capacity,
                queue_type::allocator_type{&resource}
            };
        }
        catch (...)
        {
            exceptionCaught = true;
        }

        test.expect
        (
            (exceptionCaught),
            "element construction failure propagates"
        );
        test.expect
        (
            (throwing_value::destructionCount_ == 2),
            "constructed elements are destroyed after partial failure"
        );
        test.expect
        (
            (resource.was_deallocated()),
            "injected allocation is released after partial failure"
        );
    }


    void test_injected_storage(test_context & test)
    {
        using queue_type = bcpp::spsc_fixed_queue<std::uint64_t>;

        auto const storageSize = queue_type::required_storage_size(17);
        auto * mapping = static_cast<std::byte *>
        (
            ::mmap
            (
                nullptr,
                storageSize,
                PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS,
                -1,
                0
            )
        );
        test.expect
        (
            (mapping != MAP_FAILED),
            "anonymous shared mapping succeeds"
        );
        if ((mapping == MAP_FAILED))
            return;

        fixed_memory_resource resource
        {
            std::span<std::byte>{mapping, storageSize}
        };
        {
            queue_type creator
            {
                17,
                queue_type::allocator_type{&resource}
            };
            auto joined = queue_type::join
            (
                std::span<std::byte>{mapping, storageSize}
            );

            test.expect
            (
                (resource.was_allocated()),
                "injected allocator supplies storage"
            );
            test.expect
            (
                (creator.capacity() == 32),
                "creator records rounded capacity"
            );
            test.expect
            (
                (joined.capacity() == 32),
                "joined handle reads instance metadata"
            );
            test.expect
            (
                (creator.push(1234)),
                "creator pushes through injected storage"
            );

            auto value = std::uint64_t{};
            test.expect
            (
                (joined.try_pop(value) == 1),
                "joined handle observes creator state"
            );
            test.expect((value == 1234), "joined handle observes creator data");
        }

        test.expect
        (
            (resource.was_deallocated()),
            "owning handle releases through allocator"
        );
        test.expect
        (
            (::munmap(mapping, storageSize) == 0),
            "anonymous mapping unmaps"
        );
    }


    void test_threaded_spsc(test_context & test)
    {
        constexpr auto iterationCount = std::uint64_t{5'000'000};
        bcpp::spsc_fixed_queue<std::uint64_t> queue{1024};
        std::barrier start{2};
        std::atomic<bool> valid{true};

        std::thread producer
        {
            [&]
            {
                start.arrive_and_wait();
                for (auto value = std::uint64_t{1};
                        (value <= iterationCount); ++value)
                {
                    while ((!queue.push(value)))
                        std::this_thread::yield();
                }
            }
        };

        std::thread consumer
        {
            [&]
            {
                start.arrive_and_wait();
                for (auto expected = std::uint64_t{1};
                        (expected <= iterationCount); ++expected)
                {
                    auto value = std::uint64_t{};
                    while ((queue.try_pop(value) == 0))
                        std::this_thread::yield();

                    if ((value != expected))
                    {
                        valid = false;
                        return;
                    }
                }
            }
        };

        producer.join();
        consumer.join();

        test.expect((valid), "threaded SPSC preserves every value in order");
        test.expect((queue.empty()), "threaded SPSC drains completely");
    }


    void test_interprocess_spsc(test_context & test)
    {
        using queue_type = bcpp::spsc_fixed_queue<std::uint64_t>;
        constexpr auto iterationCount = std::uint64_t{2'000'000};
        auto const storageSize = queue_type::required_storage_size(4096);

        char path[] = "/tmp/bcpp-spsc-queue-XXXXXX";
        auto const descriptor = ::mkstemp(path);
        test.expect((descriptor >= 0), "interprocess backing file opens");
        if ((descriptor < 0))
            return;
        static_cast<void>(::unlink(path));

        if ((::ftruncate(descriptor, static_cast<off_t>(storageSize)) != 0))
        {
            test.expect(false, "interprocess backing file sizes successfully");
            static_cast<void>(::close(descriptor));
            return;
        }

        auto * parentMapping = static_cast<std::byte *>
        (
            ::mmap
            (
                nullptr,
                storageSize,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                descriptor,
                0
            )
        );
        test.expect
        (
            (parentMapping != MAP_FAILED),
            "parent shared mapping succeeds"
        );
        if ((parentMapping == MAP_FAILED))
        {
            static_cast<void>(::close(descriptor));
            return;
        }

        fixed_memory_resource resource
        {
            std::span<std::byte>{parentMapping, storageSize}
        };

        int readyPipe[2]{};
        if ((::pipe(readyPipe) != 0))
        {
            test.expect(false, "interprocess readiness pipe opens");
            static_cast<void>(::munmap(parentMapping, storageSize));
            static_cast<void>(::close(descriptor));
            return;
        }

        auto childStatus = 1;
        {
            queue_type producer
            {
                4096,
                queue_type::allocator_type{&resource}
            };

            auto const child = ::fork();
            test.expect((child >= 0), "consumer process starts");
            if ((child == 0))
            {
                static_cast<void>(::close(readyPipe[0]));
                auto * childMapping = static_cast<std::byte *>
                (
                    ::mmap
                    (
                        nullptr,
                        storageSize,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        descriptor,
                        0
                    )
                );

                if
                (
                    (childMapping == MAP_FAILED) ||
                    (childMapping == parentMapping)
                )
                {
                    ::_exit(2);
                }

                auto consumer = queue_type::join
                (
                    std::span<std::byte>{childMapping, storageSize}
                );
                auto const ready = char{'R'};
                if ((::write(readyPipe[1], &ready, 1) != 1))
                    ::_exit(3);

                for (auto expected = std::uint64_t{1};
                        (expected <= iterationCount); ++expected)
                {
                    auto value = std::uint64_t{};
                    while ((consumer.try_pop(value) == 0))
                        std::this_thread::yield();

                    if ((value != expected))
                        ::_exit(4);
                }
                ::_exit(0);
            }

            if ((child > 0))
            {
                static_cast<void>(::close(readyPipe[1]));
                auto ready = char{};
                if ((::read(readyPipe[0], &ready, 1) == 1))
                {
                    for (auto value = std::uint64_t{1};
                            (value <= iterationCount); ++value)
                    {
                        while ((!producer.push(value)))
                            std::this_thread::yield();
                    }
                }

                auto waitStatus = 0;
                if ((::waitpid(child, &waitStatus, 0) == child))
                {
                    childStatus =
                    (
                        (WIFEXITED(waitStatus)) ?
                        WEXITSTATUS(waitStatus) : 5
                    );
                }
            }
        }

        static_cast<void>(::close(readyPipe[0]));
        static_cast<void>(::close(readyPipe[1]));
        test.expect
        (
            (childStatus == 0),
            "interprocess SPSC preserves every value in order"
        );
        test.expect
        (
            (::munmap(parentMapping, storageSize) == 0),
            "parent mapping unmaps"
        );
        test.expect
        (
            (::close(descriptor) == 0),
            "interprocess backing file closes"
        );
    }

} // namespace


//=============================================================================
int main()
{
    auto test = test_context{};

    test_local_queue(test);
    test_failed_construction(test);
    test_injected_storage(test);
    test_threaded_spsc(test);
    test_interprocess_spsc(test);

    if ((test.result() == 0))
        std::cout << "spsc_fixed_queue tests passed\n";
    return test.result();
}
