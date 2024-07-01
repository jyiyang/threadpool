# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(ThreadPool VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Threads REQUIRED)

add_library(thread_pool thread_pool.cc)

# Add an executable
add_executable(test_thread_pool main.cc)
target_link_libraries(test_thread_pool PRIVATE thread_pool Threads::Threads)
