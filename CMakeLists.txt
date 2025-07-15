cmake_minimum_required(VERSION 3.0)
project(chained_list C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

add_executable(chained_list main.c
        chained_list.c
        chained_list.h
)

set(CMAKE_CXX_COMPILER_WORKS TRUE)
set(CMAKE_C_COMPILER_WORKS TRUE)