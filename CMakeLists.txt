cmake_minimum_required(VERSION 3.14)

project(Labs)

set(CMAKE_CXX_STANDARD 17)

set(TOP_DIR ${CMAKE_SOURCE_DIR})


option(BUILD_LAB1 "build lab work #1" OFF)
if (BUILD_LAB1)
    add_subdirectory(${TOP_DIR}/Lab1)
endif()

option(BUILD_TESTS "build tests for lab" OFF)