cd Ext\resvg\c-api

:: build the static lib with dynamic linked runtime
cargo build --release

:: copy the header
copy resvg.h ..\..\..\jb_resvg\inc\

:: copy the library to the target destination
cd ..\target\release
copy resvg.lib ..\..\..\..\jb_resvg\libs\VisualStudio2019\x64\MD\

:: lets go back to the original folder
cd ..\..\..\..\