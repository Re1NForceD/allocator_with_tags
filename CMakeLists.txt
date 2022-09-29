cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(OSLab1
        VERSION 0.0.1
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
#
# if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
#     message(STATUS "Target platform is ${CMAKE_SYSTEM_NAME}")
# elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
#     message(STATUS "Target platform is ${CMAKE_SYSTEM_NAME}")
# endif()
#
# function(resolve_build_type)
#     if(CMAKE_BUILD_TYPE STREQUAL "Release")
#         message(STATUS "BUILD RELEASE EXECUTABLE")
#     elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
#         message(STATUS "BUILD DEBUG EXECUTABLE")
#         add_compile_definitions(DEBUG)
#     endif()
# endfunction()
#
# if((CMAKE_BUILD_TYPE STREQUAL "") OR (NOT (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Debug")))
#     message(NOTICE "Unknown value of CMAKE_BUILD_TYPE variable: '${CMAKE_BUILD_TYPE}', set value to 'Debug'" )
#     set(CMAKE_BUILD_TYPE "Debug")
# endif()
#
# resolve_build_type()
#
# include_directories("${CMAKE_CURRENT_SOURCE_DIR}/externalFiles/inc" "${CMAKE_CURRENT_SOURCE_DIR}/include")
# link_directories("${CMAKE_CURRENT_SOURCE_DIR}/externalFiles/src")
#
# find_package(OpenGL REQUIRED)
# add_library(glad "${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c")
#
# function(configure_hpp target)
#     set(FILENAME "${target}.hpp")
#     configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/${FILENAME}.in" ${FILENAME})
# endfunction()
#
# function(configure_cpp target)
#     set(FILENAME "${target}.cpp")
#     configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/${FILENAME}" ${FILENAME})
# endfunction()
#
# function(configure_hpp_cpp target)
#     configure_hpp(${target})
#     configure_cpp(${target})
#     add_library(${target} "${PROJECT_BINARY_DIR}/${target}.cpp")
#     target_include_directories(${target} PRIVATE ${PROJECT_BINARY_DIR})
# endfunction()
#
# configure_hpp_cpp(errors)
# configure_hpp_cpp(logger)
# configure_hpp_cpp(fonts)
# configure_hpp_cpp(rawopengl)
# configure_hpp_cpp(callbacks)
# configure_hpp_cpp(widgets)
# configure_hpp_cpp(tabs)
#
# configure_cpp(main)
add_library(allocator "${CMAKE_CURRENT_SOURCE_DIR}/allocator.cpp")
target_include_directories(allocator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(OSLab1 "${CMAKE_CURRENT_SOURCE_DIR}/main_lab1.cpp")
target_link_libraries(OSLab1 PRIVATE allocator)
target_include_directories(OSLab1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
