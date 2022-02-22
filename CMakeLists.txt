cmake_minimum_required(VERSION 3.21)
project(calculator C)

set(CMAKE_C_STANDARD 99)

if(UNIX)
    file(GLOB_RECURSE SOURCES calculator/*.h calculator/*.c)
    add_executable(calculator main.c ${SOURCES})
    target_include_directories(calculator PUBLIC calculator)
    target_link_libraries(calculator PUBLIC m)
else()
    message(FATAL_ERROR Linux Only)
endif()

