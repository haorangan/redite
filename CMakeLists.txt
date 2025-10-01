cmake_minimum_required(VERSION 3.16)
project(baby_redis LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Dependencies: Catch2
include(FetchContent)
FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(catch2)

file(GLOB_RECURSE ALL_SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
list(FILTER ALL_SRC_FILES EXCLUDE REGEX ".*/main.cpp$")

add_library(baby_redis_core STATIC ${ALL_SRC_FILES})
target_include_directories(baby_redis_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(baby_redis src/main.cpp)
target_link_libraries(baby_redis PRIVATE baby_redis_core)
target_include_directories(baby_redis PRIVATE ${CMAKE_SOURCE_DIR}/include)

include(CTest)
enable_testing()

file(GLOB_RECURSE TEST_SRC_FILES CONFIGURE_DEPENDS tests/*.cpp)
add_executable(unit_tests ${TEST_SRC_FILES})
target_link_libraries(unit_tests PRIVATE baby_redis_core Catch2::Catch2WithMain)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

include(Catch)
catch_discover_tests(unit_tests)
