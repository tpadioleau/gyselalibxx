# SPDX-License-Identifier: MIT

add_library("poisson_xy" STATIC
    chargedensitycalculator.cpp
    electricfield.cpp
    nullpoissonsolver.cpp
    fftpoissonsolver.cpp
)

target_compile_features("poisson_xy"
    PUBLIC
        cxx_std_17
)

target_include_directories("poisson_xy"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries("poisson_xy"
    PUBLIC
        DDC::DDC
        sll::splines
        vcx::geometry_xyvxvy
        vcx::speciesinfo
)

add_library("vcx::poisson_xy" ALIAS "poisson_xy")
