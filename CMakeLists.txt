cmake_minimum_required(VERSION 3.16)

project(KnightMazeRPG VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Curses REQUIRED)

add_executable(comp2113
    comp2113.cpp
)

target_include_directories(comp2113 PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(comp2113 PRIVATE ${CURSES_LIBRARIES})
