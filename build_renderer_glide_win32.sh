# Build shiet's Glide 3.x renderer for Win32 using TDM-GCC (MinGW) 4.4.1 via Wine in Linux.

SRC_FILES="
src/shiet_lib/renderer/renderer_glide_3.c
src/shiet_lib/renderer/rasterizer/glide_3/rasterizer_glide_3.c
src/shiet_lib/renderer/rasterizer/glide_3/surface_glide_3_win32.c
src/shiet_lib/renderer/window/win32/window_win32.c
"

OUTPUT_FILE="bin/shiet_renderer_glide_3.dll"

MINGW_PATH=~/compi/mingw441

BUILD_OPTIONS="
-shared
-nostdinc
-g
-std=c89
-pedantic
-Wall
-march=pentium
-I ./src/
-isystem $MINGW_PATH/lib/gcc/mingw32/4.4.1/include
-isystem $MINGW_PATH/include
"

wine "$MINGW_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $OUTPUT_FILE $SRC_FILES -lm -lgdi32 -lglide/glide3x
