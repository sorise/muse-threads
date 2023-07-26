cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

add_executable(poolUnit poolTest.cpp)

add_executable(exUnit executor_ut.cpp)

target_link_libraries(poolUnit PUBLIC threads )
target_link_libraries(exUnit PUBLIC threads )

add_test(
    NAME "poolUnit"
    COMMAND poolUnit
)

add_test(
    NAME "exUnit"
    COMMAND exUnit
)
