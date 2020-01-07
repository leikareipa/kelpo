# Build shiet's OpenGL renderer for Win32 using TDM-GCC (MinGW) 4.4.1 via Wine in Linux.

SRC_FILES="
src/shiet_lib/renderer/renderer_opengl_1_2.c
src/shiet_lib/renderer/rasterizer/opengl_1_2/rasterizer_opengl_1_2.c
src/shiet_lib/renderer/rasterizer/opengl_1_2/surface_opengl_1_2_win32.c
src/shiet_lib/renderer/window/win32/window_win32.c
"

OUTPUT_FILE="bin/shiet_renderer_opengl_1_2.dll"

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

wine "$MINGW_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $OUTPUT_FILE $SRC_FILES -lm -lgdi32 -lopengl32 -lglu32
