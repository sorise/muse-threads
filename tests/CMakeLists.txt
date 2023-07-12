cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

add_executable(poolUnit poolTest.cpp)

add_test(
    NAME "poolUnit"
    COMMAND poolUnit
)
