#!/bin/bash

# change to the c-api folder
cd Ext/resvg/c-api

# Exporting this is important as resvg compiles C sources during build at the current state.
# While rustc is guranteed to create libraries with a deployment target of macOS 10.7 from
# rust sources, the C files compiled in this process are built with the macOS version of the
# build machine as default deployment target if not specified differently
export MACOSX_DEPLOYMENT_TARGET=10.7

# build a static C library
cargo rustc --release -- --crate-type staticlib

# copy the header
cp resvg.h ../../../Include

# go to the library just build
cd ../target/release

# copy the lib
cp libresvg.a ../../../../Lib/

