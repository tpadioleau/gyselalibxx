# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

add_library("geometry" INTERFACE)
target_include_directories("geometry"
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)
target_link_libraries("geometry" INTERFACE
    DDC::DDC
    sll::splines
    vcx::speciesinfo
)
add_library("vcx::geometry" ALIAS "geometry")

target_compile_definitions(geometry INTERFACE PERIODIC_RDIMX)
