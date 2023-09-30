# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

add_subdirectory(speciesinfo)
add_subdirectory(geometryXVx)

add_executable(femperiodicpoisson femperiodicpoissonsolver.cpp)
target_compile_features(femperiodicpoisson PUBLIC cxx_std_17)
target_link_libraries(femperiodicpoisson
    PUBLIC
        sll::splines
        vcx::geometry
        vcx::speciesinfo
        vcx::poisson
)
