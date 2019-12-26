#!/bin/bash

# Build shiet for Win32 using MinGW via Wine under Linux.
# Note that this will place the executable under shiet's main bin/ directory.

GCC_PATH=~/compi/mingw471
OUTPUT_FILE_NAME="rotating_triangle.exe"
BUILD_OPTIONS="-nostdinc -s -ansi -pedantic -Wall -march=pentium -I../../src/ -isystem $GCC_PATH/lib/gcc/mingw32/4.7.1/include -isystem $GCC_PATH/include"

SRC_FILES="
src\\main.c
..\\common_src\\transform_and_rotate_triangles.c
..\\..\\src\\shiet\\renderer_interface.c
"

wine "$GCC_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $OUTPUT_FILE_NAME $SRC_FILES -lm

if [ -e $OUTPUT_FILE_NAME ]
then
	mv $OUTPUT_FILE_NAME ../../bin/$OUTPUT_FILE_NAME
else
	echo Build failed.
	exit 1
fi

exit 0
