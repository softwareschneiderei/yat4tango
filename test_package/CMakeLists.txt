cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(YAT4TANGO CONFIG REQUIRED)

add_executable(test_package src/test_package.cpp)
target_link_libraries(test_package ${YAT4TANGO_LIBRARIES})

