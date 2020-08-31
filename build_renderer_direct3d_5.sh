# Build Kelpo's Direct3D 5 renderer for Win32 using TDM-GCC (MinGW) 4.4.1 via Wine from Linux.
# Unlike the rest of Kelpo, which is C89, the DirectX 5 headers require compilation with C++.

OUTPUT_FILE="bin/kelpo_renderer_direct3d_5.dll"

MINGW441_BASE_PATH=~/compi/mingw441

# DirectX 5 SDK.
# Expects include files in ./include/*
# Expects lib files in ./lib/*
DX5_BASE_PATH=~/sdk/directx5

SRC_FILES="
src/kelpo_renderer/renderer_direct3d_5.c
src/kelpo_renderer/surface/directdraw_5/enumerate_directdraw_5_devices.c
src/kelpo_renderer/surface/directdraw_5/create_directdraw_5_surface_from_texture.c
src/kelpo_renderer/rasterizer/direct3d_5/rasterizer_direct3d_5.c
src/kelpo_renderer/surface/direct3d_5/surface_direct3d_5.c
src/kelpo_renderer/surface/directdraw_5/surface_directdraw_5.c
src/kelpo_renderer/window/win32/window_win32.c
src/kelpo_auxiliary/generic_stack.c
"

BUILD_OPTIONS="
-shared
-nostdinc
-g
-O2
-std=c++98
-w
-Wall
-march=pentium
-I ./src/
-I $DX5_BASE_PATH/include
-isystem $MINGW441_BASE_PATH/lib/gcc/mingw32/4.4.1/include
-isystem $MINGW441_BASE_PATH/include
-L $DX5_BASE_PATH/lib
"

wine "$MINGW441_BASE_PATH/bin/g++.exe" $BUILD_OPTIONS -o $OUTPUT_FILE $SRC_FILES -lm -lgdi32 -lddraw -ld3dim -ldxguid
