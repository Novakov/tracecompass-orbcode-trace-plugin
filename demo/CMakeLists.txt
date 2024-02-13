cmake_minimum_required(VERSION 3.21)

if(CMAKE_TOOLCHAIN_FILE)
    message(WARNING "You are using custom toolchain file. Make sure you have set the correct compiler and options")
else()
    include(cmake/toolchain.cmake)
endif()

project(TraceCompassTraceDemoApp C CXX ASM)

include(openocd/functions.cmake)

find_package(Python3)

add_subdirectory(execs)
add_subdirectory(libs)
add_subdirectory(openocd)
