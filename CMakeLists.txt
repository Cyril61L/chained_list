cmake_minimum_required(VERSION 3.0)
project(chained_list C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
#set(SYSROOT /opt/fslc-framebuffer/4.1-snapshot-20250425)
#set(TOOLCHAIN_PATH_FSL ${SYSROOT}/sysroots/x86_64-fslcsdk-linux/usr/bin/aarch64-fslc-linux)
#set(CROSS_COMPILER_FSL ${TOOLCHAIN_PATH_FSL}/aarch64-fslc-linux-gcc)

#set(CMAKE_C_COMPILER ${CROSS_COMPILER_FSL})
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${SYSROOT}/sysroots/cortexa55-fslc-linux")

#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -s -DNDEBUG")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -D_GNU_SOURCE -std=gnu17")

add_executable(chained_list main_fifo.c
        chained_list.c
        chained_list.h
)

set(CMAKE_CXX_COMPILER_WORKS TRUE)
set(CMAKE_C_COMPILER_WORKS TRUE)