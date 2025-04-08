cmake_minimum_required(VERSION 3.0)
project(chianed_list C)

set(CMAKE_C_STANDARD 11)

add_executable(chianed_list main.c
        chained_list.c
        chained_list.h)
