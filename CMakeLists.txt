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
set (RESVG_LIB_OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/target/release)

if (WIN32)
    set (RESVG_LIB_OUTPUT ${RESVG_LIB_OUTPUT_DIR}/resvg.lib)
else()
    set (RESVG_LIB_OUTPUT ${RESVG_LIB_OUTPUT_DIR}/libresvg.a)
endif()

file (MAKE_DIRECTORY ${RESVG_LIB_OUTPUT_DIR})
file (TOUCH ${RESVG_LIB_OUTPUT})

# The target to trigger the rust cargo based compilation of the resvg library used.
# For macOS we need to ensure the C-Parts of it being built for the right deployment
# target by setting an environment variable first
if (APPLE)
    add_custom_target (resvg_cargo_build_arm64
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/c-api
            COMMAND ${CMAKE_COMMAND} -E env MACOSX_DEPLOYMENT_TARGET=10.7 cargo build --release --target aarch64-apple-darwin)

    add_custom_target (resvg_cargo_build_x86_64
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/c-api
            COMMAND ${CMAKE_COMMAND} -E env MACOSX_DEPLOYMENT_TARGET=10.7 cargo build --release --target x86_64-apple-darwin)

    add_custom_target (resvg_cargo_build
            WORKING_DIRECTORY ${RESVG_LIB_OUTPUT_DIR}
            COMMAND lipo -create -output libresvg.a ../aarch64-apple-darwin/release/libresvg.a ../x86_64-apple-darwin/release/libresvg.a)

    add_dependencies (resvg_cargo_build resvg_cargo_build_arm64 resvg_cargo_build_x86_64)
else()
     add_custom_target (resvg_cargo_build
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/c-api
            COMMAND cargo build --release)
endif()

# Make an imported targe that references the library just built
add_library (resvg STATIC IMPORTED)
add_dependencies (resvg resvg_cargo_build)
set_target_properties (resvg PROPERTIES IMPORTED_LOCATION ${RESVG_LIB_OUTPUT})
target_include_directories (resvg INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/c-api)

# Seems like bcrypt is a dependency on Windows
if (WIN32)
    target_link_libraries (resvg INTERFACE bcrypt)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Only apply SSE3 optimization to x86 architectures
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|AMD64|i686)")
        target_compile_options (resvg INTERFACE -mssse3)
    endif()
endif()

# Create the Resvg4JUCE juce module target. This depends on the resvg library created above and on the juce_gui_basics module
juce_add_module (Modules/Resvg4JUCE)
target_link_libraries (Resvg4JUCE INTERFACE resvg)

# Export an alias target
add_library (jb::Resvg4JUCE ALIAS Resvg4JUCE)
