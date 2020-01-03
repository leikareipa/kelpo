# Build shiet's OpenGL renderer for Win32 using TDM-GCC (MinGW) 4.4.1 via Wine in Linux.

SRC_FILES="
src/shiet_lib/renderer/renderer_opengl.c
src/shiet_lib/renderer/rasterizer/opengl/rasterizer_opengl.c
src/shiet_lib/renderer/rasterizer/opengl/surface_opengl_win32.c
src/shiet_lib/renderer/window/win32/window_win32.c
"

OUTPUT_FILE="bin/shiet_renderer_opengl.dll"

MINGW_PATH=~/compi/mingw441

BUILD_OPTIONS="
-shared
-nostdinc
-s
-ansi
-pedantic
-Wall
-march=pentium
-I ./src/
-isystem $MINGW_PATH/lib/gcc/mingw32/4.4.1/include
-isystem $MINGW_PATH/include
"

wine "$MINGW_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $OUTPUT_FILE $SRC_FILES -lm -lgdi32 -lopengl32 -lglu32
