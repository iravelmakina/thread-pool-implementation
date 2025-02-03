cmake_minimum_required(VERSION 3.29)
project(thread_pool_implementation)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(thread_pool_implementation
        src/main.cpp
        src/ThreadPool.cpp
        src/TaskQueue.cpp)
