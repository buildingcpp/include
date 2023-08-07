cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_CXX_STANDARD 20)

set(PROJECT_NAME include)
project(${PROJECT_NAME})

set(_${PROJECT_NAME}_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "")

add_subdirectory(src)

