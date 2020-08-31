# Build Kelpo's OpenGL renderer for Win32 using TDM-GCC (MinGW) 4.4.1 via Wine from Linux.

OUTPUT_FILE="bin/kelpo_renderer_opengl_1_2.dll"

MINGW441_BASE_PATH=~/compi/mingw441

SRC_FILES="
src/kelpo_renderer/renderer_opengl_1_2.c
src/kelpo_renderer/rasterizer/opengl_1_2/rasterizer_opengl_1_2.c
src/kelpo_renderer/surface/opengl_1_2/surface_opengl_1_2.c
src/kelpo_renderer/window/win32/window_win32.c
src/kelpo_auxiliary/generic_stack.c
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
-isystem $MINGW441_BASE_PATH/lib/gcc/mingw32/4.4.1/include
-isystem $MINGW441_BASE_PATH/include
"

wine "$MINGW441_BASE_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $OUTPUT_FILE $SRC_FILES -lm -lgdi32 -lopengl32 -lglu32
