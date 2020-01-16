# Build a Kelpo render example for Win32 using Microsoft Visual C++ 6 via Wine in Linux.
# Note that this will place the executable under Kelpo's root bin/ directory.

VC6_PATH=~/compi/vc60

OUTPUT_FILE="../../bin/rotating_triangle.exe"

SRC_FILES="
src/main.c
../common_src/transform_and_rotate_triangles.c
../common_src/parse_command_line.c
../../src/kelpo_interface/interface.c
../../src/kelpo_interface/generic_stack.c
"

BUILD_OPTIONS="
/O2
/G4
/Fe$OUTPUT_FILE
/Fo./bin/
/I../../src/
/I$VC6_PATH/INCLUDE
"

wine "$VC6_PATH/BIN/cl.exe" $BUILD_OPTIONS $SRC_FILES "/link /LIBPATH:$VC6_PATH/LIB"
