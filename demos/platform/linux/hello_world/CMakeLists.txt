# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)

project(hello_world)

include_directories(
    ../../../../inc
    ..
)

link_directories(
    .
)

list(APPEND ZBUS_SRC_LIST
    ../../../../zbus.c
    ../../../../zbus_iterable_sections.c
)
aux_source_directory(../../../../port/posix PORT_SRC_LIST)

list(APPEND SRC_LIST
    ${ZBUS_SRC_LIST}
    ${PORT_SRC_LIST}
)

add_executable(hello_world 
    ../../../src/hello_world/main.c
    ${SRC_LIST}
)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -lrt -T ${CMAKE_SOURCE_DIR}/link.lds")

set(CMAKE_C_FLAGS   "-std=gnu99 -O0 -fno-stack-protector")             # gnu99
