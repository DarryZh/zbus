# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)

project(benchmark)

include_directories(
    ../../inc
    ../../config
    src/
)

link_directories(
    .
)

# aux_source_directory(../../ ZBUS_SRC_LIST)
list(APPEND ZBUS_SRC_LIST
    ../../zbus.c
    ../../zbus_iterable_sections.c
)
aux_source_directory(../../port/posix PORT_SRC_LIST)

list(APPEND SRC_LIST
    ${ZBUS_SRC_LIST}
    ${PORT_SRC_LIST}
)

add_executable(benchmark 
    ${CMAKE_CURRENT_LIST_DIR}/src/benchmark.c
    ${SRC_LIST}
)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -lrt -T ${CMAKE_SOURCE_DIR}/link.lds")
set(CMAKE_CXX_FLAGS   "-std=c++11")             # c++11
set(CMAKE_CXX_FLAGS   "-g")                     # 调试信息
set(CMAKE_CXX_FLAGS   "-Wall")                  # 开启所有警告
