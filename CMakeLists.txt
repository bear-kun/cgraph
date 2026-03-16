cmake_minimum_required(VERSION 3.10)
project(cgraph C)

set(CMAKE_C_STANDARD 11)

file(GLOB SOURCES "src/core/*.c")
add_library(cgraph_core STATIC ${SOURCES})
target_include_directories(cgraph_core PRIVATE include/cgraph)

file(GLOB_RECURSE SOURCES "src/alg/*.c" "src/struct/*.c")
add_library(cgraph_alg STATIC ${SOURCES})
target_include_directories(cgraph_alg PRIVATE include)
target_link_libraries(cgraph_alg PUBLIC cgraph_core)

add_executable(cgraph_test test/test.c)
target_include_directories(cgraph_test PRIVATE include)
target_link_libraries(cgraph_test PRIVATE cgraph_alg)