# Build Kelpo's Glide 3.x renderer for Win32 using TDM-GCC (MinGW) 4.4.1 via Wine from Linux.

OUTPUT_FILE="bin/kelpo_renderer_glide_3.dll"

MINGW441_BASE_PATH=~/compi/mingw441

# Glide 3.0 SDK.
# Expects include files in ./include/glide/*
# Expects lib files in ./lib/*
GLIDE3_BASE_PATH=~/sdk/glide3

SRC_FILES="
src/kelpo_renderer/renderer_glide_3.c
src/kelpo_renderer/rasterizer/glide_3/rasterizer_glide_3.c
src/kelpo_renderer/surface/glide_3/surface_glide_3.c
src/kelpo_renderer/window/win32/window_win32.c
src/kelpo_auxiliary/generic_stack.c
src/kelpo_interface/interface.c
src/kelpo_interface/error.c
"

BUILD_OPTIONS="
-shared
-nostdinc
-g
-O2
-std=c89
-pedantic
-Wall
-march=pentium
-I ./src/
-I $GLIDE3_BASE_PATH/include
-isystem $MINGW441_BASE_PATH/lib/gcc/mingw32/4.4.1/include
-isystem $MINGW441_BASE_PATH/include
-L $GLIDE3_BASE_PATH/lib
"

wine "$MINGW441_BASE_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $OUTPUT_FILE $SRC_FILES -lm -lgdi32 -lglide3x
