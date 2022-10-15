cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(OSLab1
        VERSION 0.1.1
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-EHsc -Wall -W4 -O2")

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(block "${CMAKE_CURRENT_SOURCE_DIR}/block.cpp")
target_include_directories(block PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(tree "${CMAKE_CURRENT_SOURCE_DIR}/tree.cpp")
target_include_directories(tree PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(kernel "${CMAKE_CURRENT_SOURCE_DIR}/kernel.cpp")
target_include_directories(kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(allocator "${CMAKE_CURRENT_SOURCE_DIR}/allocator.cpp")
target_link_libraries(allocator PRIVATE block tree kernel)
target_include_directories(allocator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()

add_executable(OSLab1 "${CMAKE_CURRENT_SOURCE_DIR}/gTest.cpp")
target_link_libraries(OSLab1 PRIVATE allocator GTest::gtest_main GTest::gmock)
target_include_directories(OSLab1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(OSLab1)
