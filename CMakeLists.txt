cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(OSLab1
        VERSION 0.1.1
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "-EHsc -Wall -W4 -O2")

add_library(allocator "${CMAKE_CURRENT_SOURCE_DIR}/allocator.cpp")
target_include_directories(allocator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(block "${CMAKE_CURRENT_SOURCE_DIR}/block.cpp")
target_include_directories(allocator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(kernel "${CMAKE_CURRENT_SOURCE_DIR}/kernel.cpp")
target_include_directories(allocator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(OSLab1 "${CMAKE_CURRENT_SOURCE_DIR}/main_lab1.cpp")
target_link_libraries(OSLab1 PRIVATE kernel allocator block)
target_include_directories(OSLab1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
