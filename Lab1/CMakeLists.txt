cmake_minimum_required(VERSION 3.14)

project(lab1)

set(LAB_DIR ${TOP_DIR}/Lab1)

find_package(Boost 1.74 EXACT REQUIRED COMPONENTS system coroutine thread)
find_package(Threads REQUIRED)

set(CORE_LIB_NAME ${PROJECT_NAME}core)
add_library(${CORE_LIB_NAME} STATIC)

target_include_directories(
    ${CORE_LIB_NAME}
    PUBLIC
    ${TOP_DIR}
    Boost::boost
)

target_link_libraries(
    ${CORE_LIB_NAME}
    PRIVATE
    ${Boost_LIBRARIES}
    Threads::Threads
)

target_sources(
    ${CORE_LIB_NAME}
    PRIVATE
    ${LAB_DIR}/Server/Server.cpp
    ${LAB_DIR}/Server/Session.cpp
)

add_executable(
    ${PROJECT_NAME} 
    ${LAB_DIR}/main.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    ${CORE_LIB_NAME}
)
