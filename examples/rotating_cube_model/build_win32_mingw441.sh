# Build a shiet render example for Win32 using TDM-GCC (MinGW) 4.4.1 via Wine in Linux.
# Note that this will place the executable under shiet's root bin/ directory.

MINGW441_PATH=~/compi/mingw441

OUTPUT_FILE="../../bin/rotating_cube_model.exe"

SRC_FILES="
src/main.c
../common_src/transform_and_rotate_triangles.c
../common_src/load_kac_1_0_mesh.c
../common_src/parse_command_line.c
../common_src/kac/import_kac_1_0.c
../../src/shiet_interface/polygon/triangle/triangle_stack.c
../../src/shiet_interface/interface.c
"

BUILD_OPTIONS="
-nostdinc
-g
-std=c89
-pedantic
-Wall
-march=pentium
-I../../src/
-isystem $MINGW441_PATH/lib/gcc/mingw32/4.4.1/include
-isystem $MINGW441_PATH/include
"

wine "$MINGW441_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $OUTPUT_FILE $SRC_FILES -lm