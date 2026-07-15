# Deliberately enable only the native C++ language.  This project does not
# compile HIP device code and therefore never invokes hipcc.

cmake_minimum_required(VERSION 3.16)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(HIP REQUIRED)

# If the user didn't explicitly choose a compiler,
# switch to the HIP SDK's clang.
if(NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "${HIP_CLANG_COMPILER}"
        CACHE FILEPATH "" FORCE)
else()
    message(FATAL_ERROR "HIPCC compiler is not found")
endif()

message(STATUS "HIP_CLANG_COMPILER: ${HIP_CLANG_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
project(hip_mem_alloc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hip_mem_alloc src/main.cpp)
target_link_libraries(hip_mem_alloc PRIVATE HIP::runtime)

if(MSVC)
    target_compile_options(hip_mem_alloc PRIVATE /W4 /permissive-)
else()
    target_compile_options(hip_mem_alloc PRIVATE -Wall -Wextra -Wpedantic)
endif()