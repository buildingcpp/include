cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_CXX_STANDARD 20)

set(PROJECT_NAME include)
project(${PROJECT_NAME})

set(_bcpp_${PROJECT_NAME}_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/src" CACHE STRING "")

add_subdirectory(src)

