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

project (ViewSVG VERSION 0.2.0)

find_package (JUCE CONFIG REQUIRED)

juce_add_gui_app (ViewSVG  PRODUCT_NAME "View SVG")

target_sources (ViewSVG PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp)

target_compile_definitions(ViewSVG PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:GuiAppExample,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:GuiAppExample,JUCE_VERSION>")

# Simply add Resvg4JUCE as subdirectory and link your target to it
add_subdirectory (../../Modules/Resvg4JUCE)
target_link_libraries (ViewSVG
    PRIVATE
        juce::juce_gui_extra
        jb::Resvg4JUCE

    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)