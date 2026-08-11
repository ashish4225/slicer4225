cmake_minimum_required(VERSION 3.14)
project(slicer4225 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable warnings
add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -O2)

# ─── Main executable ─────────────────────────────────────────────────────────
add_executable(slicer4225 src/main.cpp)
target_include_directories(slicer4225 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ─── Test executable ─────────────────────────────────────────────────────────
add_executable(slicer4225_tests src/tests.cpp)
target_include_directories(slicer4225_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
