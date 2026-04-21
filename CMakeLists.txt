cmake_minimum_required(VERSION 3.10)
project(cgraph C)

set(CMAKE_C_STANDARD 11)

file(GLOB CORE_SOURCES "src/core/*.c")
file(GLOB_RECURSE ALG_SOURCES "src/alg/*.c" "src/struct/*.c")

if (CGRAPH_ONLY_CORE)
    add_library(cgraph_core STATIC ${CORE_SOURCES})
    target_include_directories(cgraph_core PRIVATE include/cgraph)
    return()
endif ()

add_library(cgraph SHARED ${CORE_SOURCES} ${ALG_SOURCES})
target_include_directories(cgraph PRIVATE include)
set_target_properties(cgraph PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_executable(cgraph_test test/test.c)
target_include_directories(cgraph_test PRIVATE include)
target_link_libraries(cgraph_test PRIVATE cgraph)