cmake_minimum_required(VERSION 3.10)
project(Lab05_Iterators_Allocators VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(lab05_lib 
    src/fixed_memory_resource.cpp
)

target_include_directories(lab05_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(lab05_demo main.cpp)
target_link_libraries(lab05_demo lab05_lib)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(lab05_tests 
    tests/test_lab05.cpp
)
target_link_libraries(lab05_tests 
    lab05_lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(lab05_tests)