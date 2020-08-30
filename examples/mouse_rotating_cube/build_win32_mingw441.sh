# Build a Kelpo render example for Win32 using TDM-GCC (MinGW) 4.4.1 via Wine in Linux.
# Note that this will place the executable under Kelpo's root bin/ directory.

MINGW441_PATH=~/compi/mingw441

OUTPUT_FILE="../../bin/mouse_rotating_cube.exe"

SRC_FILES="
src/main.c
../common_src/parse_command_line.c
../common_src/default_window_message_handler.c
../../src/kelpo_auxiliary/triangle_preparer.c
../../src/kelpo_auxiliary/generic_stack.c
../../src/kelpo_auxiliary/matrix_44.c
../../src/kelpo_auxiliary/vector_3.c
../../src/kelpo_auxiliary/triangle_clipper.c
../../src/kelpo_auxiliary/load_kac_1_0_mesh.c
../../src/kelpo_auxiliary/import_kac_1_0.c
../../src/kelpo_auxiliary/text_mesh.c
../../src/kelpo_interface/interface.c
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
