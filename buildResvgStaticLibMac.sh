#!/bin/bash

# change to the c-api folder
cd Ext/resvg/c-api

# build the library
cargo build --release

# copy the header
cp resvg.h ../../../jb_resvg/inc/

# go to the library just build
cd ../target/release

# copy the lib
cp libresvg.a ../../../../jb_resvg/libs/MacOSX/x86_64/
