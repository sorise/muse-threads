cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

project(threadpool
        VERSION         0.0.1
        DESCRIPTION     "A Simple RPC Application Framework Based on Connection UDP"
        HOMEPAGE_URL    "https://github.com/muse"
        LANGUAGES       C CXX
)

include_directories(include)
include_directories(dependency/concurrent_queue)
include_directories(dependency/catch2)


set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED  ON)

add_executable(pool main.cpp)

# 启动单元测试
enable_testing()
# 进入单元测试
add_subdirectory(tests)
