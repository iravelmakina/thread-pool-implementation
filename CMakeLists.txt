cmake_minimum_required(VERSION 3.29)
project(thread-pool-implementation)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_executable(thread-pool-implementation
        src/main.cpp
        src/ThreadPool.cpp
)
