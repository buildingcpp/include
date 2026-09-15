find_package(Threads REQUIRED)

add_executable(
    spsc_fixed_queue_test
    spsc_fixed_queue_test.cpp
)

target_link_libraries(
    spsc_fixed_queue_test
    PRIVATE
        include
        Threads::Threads
)

add_test(
    NAME spsc_fixed_queue_test
    COMMAND spsc_fixed_queue_test
)
