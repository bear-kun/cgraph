cmake_minimum_required(VERSION 3.10)
project(cgraph C)

set(CMAKE_C_STANDARD 11)

option(CGRAPH_ASSERT_MODE
        "If ON, use assert in Debug and NO checks in Release. If OFF, always return error codes."
        OFF)

file(GLOB CORE_SOURCES "src/graph.c")
file(GLOB ALG_SOURCES "src/alg/*.c" "src/struct.c")

if (CGRAPH_ONLY_CORE)
    add_library(cgraph STATIC ${CORE_SOURCES})
    target_include_directories(cgraph PRIVATE include)
    return()
else ()
    add_library(cgraph SHARED ${CORE_SOURCES} ${ALG_SOURCES})
    target_include_directories(cgraph PRIVATE include)
    set_target_properties(cgraph PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

if (CGRAPH_ASSERT_MODE)
    target_compile_definitions(cgraph PUBLIC CGRAPH_ASSERT_MODE=1)
endif ()