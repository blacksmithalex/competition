cmake_minimum_required(VERSION 3.16)

project(ContestCourseWork VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(contest_core
    src/Contest.cpp
    src/Storage.cpp
)

target_include_directories(contest_core PUBLIC include)

if(MSVC)
    target_compile_options(contest_core PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(contest_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(contest src/main.cpp)
target_link_libraries(contest PRIVATE contest_core)

if(MSVC)
    target_compile_options(contest PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(contest PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(contest_tests tests/contest_tests.cpp)
    target_link_libraries(contest_tests PRIVATE contest_core)
    add_test(NAME contest_tests COMMAND contest_tests)
endif()
