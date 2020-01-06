# Build a shiet render example for Win32 using TDM-GCC (MinGW) 4.4.1 via Wine in Linux.
# Note that this will place the executable under shiet's root bin/ directory.

SRC_FILES="
src/main.c
../common_src/transform_and_rotate_triangles.c
../common_src/parse_command_line.c
../../src/shiet/renderer_interface.c
../../src/shiet/polygon/triangle/triangle_stack.c
"

OUTPUT_FILE="../../bin/rotating_triangle.exe"

MINGW_PATH=~/compi/mingw441

BUILD_OPTIONS="
-nostdinc
-g
-std=c89
-pedantic
-Wall
-march=pentium
-I../../src/
-isystem $MINGW_PATH/lib/gcc/mingw32/4.4.1/include
-isystem $MINGW_PATH/include
"

wine "$MINGW_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $OUTPUT_FILE $SRC_FILES -lm
