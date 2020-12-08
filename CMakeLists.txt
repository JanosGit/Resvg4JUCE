# ====================================================================
#
# This file is part of Resvg4JUCE.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ====================================================================

cmake_minimum_required (VERSION 3.16)

project (RESVG4JUCE VERSION 0.2.0)

# Some CMake exporters (e.g. ninja) need a dummy library file to be present before the 
# build target referencing this file and triggering the prior build has been executed.
# Therefore we create a dummy library output file here first
set (RESVG_LIB_OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/target/release/deps)

if (WIN32)
    if (CMAKE_BUILD_TYPE MATCHES DEBUG)
        set (RESVG_LIB_OUTPUT ${RESVG_LIB_OUTPUT_DIR}/resvg.dll)
    else()
        set (RESVG_LIB_OUTPUT ${RESVG_LIB_OUTPUT_DIR}/resvg.lib)
    endif()
else()
    sset (RESVG_LIB_OUTPUT ${RESVG_LIB_OUTPUT_DIR}/libresvg.a)
endif()

file (MAKE_DIRECTORY ${RESVG_LIB_OUTPUT_DIR})
file (TOUCH ${RESVG_LIB_OUTPUT})

# Except for Windows debug builds we always want a static library
if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL DEBUG))
    set (RESVG_CRATE_TYPE cdylib)
    add_library (resvg SHARED IMPORTED)
    message ("Linking against dynamic resvg library for debug build")
else()
    set (RESVG_CRATE_TYPE staticlib)
    add_library (resvg STATIC IMPORTED)
    message ("sddjhc")
endif()


# The target to trigger the rust cargo based compilation of the resvg library used
if (APPLE)
    add_custom_target (resvg_cargo_build
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/c-api
            COMMAND ${CMAKE_COMMAND} -E env MACOSX_DEPLOYMENT_TARGET=10.7 cargo rustc --release -- --crate-type ${RESVG_CRATE_TYPE})
else()
     add_custom_target (resvg_cargo_build
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/c-api
            COMMAND cargo rustc --release -- --crate-type ${RESVG_CRATE_TYPE})
endif()

# Make the imported target created above referencing the library just built
add_dependencies (resvg resvg_cargo_build)
set_target_properties (resvg PROPERTIES IMPORTED_LOCATION ${RESVG_LIB_OUTPUT})
target_include_directories (resvg INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/c-api)

# Create the Resvg4JUCE juce module target. This depends on the resvg library created above and on the juce_gui_basics module
juce_add_module (Modules/Resvg4JUCE)
target_link_libraries (Resvg4JUCE INTERFACE resvg)

# Export an alias target
add_library (jb::Resvg4JUCE ALIAS Resvg4JUCE)
