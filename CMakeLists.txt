cmake_minimum_required(VERSION 3.29)
project(thread_pool_implementation)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_executable(thread_pool_implementation
        src/main.cpp
        src/ThreadPool.cpp
)
