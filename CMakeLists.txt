cmake_minimum_required(VERSION 3.30)
project(redite)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE SOURCES
        src/*.cpp
)

add_executable(baby_redis ${SOURCES})

target_include_directories(baby_redis
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
