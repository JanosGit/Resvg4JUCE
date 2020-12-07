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

# The target to trigger the rust cargo based compilation of the resvg library used
add_custom_target (resvg_cargo
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/c-api
        COMMAND ${CMAKE_COMMAND} -E env MACOSX_DEPLOYMENT_TARGET=10.7 cargo rustc --release -- --crate-type staticlib)

# Create an imported library target to link against the lib built above and include the corresponding header
add_library (resvg STATIC IMPORTED)
set_target_properties (resvg
        PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/target/release/deps/libresvg.a)
target_include_directories(resvg
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Ext/resvg/c-api)
add_dependencies (resvg resvg_cargo)

# Create the Resvg4JUCE library target. This depends on the resvg library created above and on the juce_gui_basics module
juce_add_module (Modules/Resvg4JUCE)
target_link_libraries (Resvg4JUCE INTERFACE resvg)

# Export an alias target
add_library (jb::Resvg4JUCE ALIAS Resvg4JUCE)
