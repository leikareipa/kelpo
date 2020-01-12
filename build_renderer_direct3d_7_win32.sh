# Build shiet's Direct3D 7 renderer for Win32 using TDM-GCC (MinGW) 4.4.1 via Wine from Linux.
# Unlike the rest of shiet, which is C89, the Direct3D 7 headers require compilation with C++.

OUTPUT_FILE="bin/shiet_renderer_direct3d_7.dll"

MINGW441_BASE_PATH=~/compi/mingw441

# DirectX 7 SDK.
# Expects include files in ./include/*
# Expects lib files in ./lib/*
DX7_BASE_PATH=~/sdk/directx7

SRC_FILES="
src/shiet_renderer/renderer_direct3d_7.c
src/shiet_renderer/rasterizer/direct3d_7/enumerate_directdraw_7_devices.c
src/shiet_renderer/rasterizer/direct3d_7/create_directdraw_7_surface_from_texture.c
src/shiet_renderer/rasterizer/direct3d_7/rasterizer_direct3d_7.c
src/shiet_renderer/surface/direct3d_7/surface_direct3d_7_win32.c
src/shiet_renderer/surface/directdraw_7/surface_directdraw_7_win32.c
src/shiet_renderer/window/win32/window_win32.c
"

BUILD_OPTIONS="
-shared
-nostdinc
-g
-std=c++98
-w
-Wall
-march=pentium
-I ./src/
-I $DX7_BASE_PATH/include
-isystem $MINGW441_BASE_PATH/lib/gcc/mingw32/4.4.1/include
-isystem $MINGW441_BASE_PATH/include
-L $DX7_BASE_PATH/lib
"

wine "$MINGW441_BASE_PATH/bin/g++.exe" $BUILD_OPTIONS -o $OUTPUT_FILE $SRC_FILES -lm -lgdi32 -lddraw -ld3dim -ldxguid
