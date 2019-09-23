#!/bin/bash

# Build shiet for Win32 using MinGW via Wine in Linux.

GCC_PATH=~/compi/mingw471
OUTPUT_FILE_NAME="shiet_example.exe"
BUILD_OPTIONS="-nostdinc -s -ansi -pedantic -Wall -march=pentium -I./src/ -I$GCC_PATH/lib/gcc/mingw32/4.7.1/include -I$GCC_PATH/include"

SRC_FILES="
src\\shiet_example\\main.c
src\\shiet\\interface.c
"

wine "$GCC_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $OUTPUT_FILE_NAME $SRC_FILES -lm

if [ -e $OUTPUT_FILE_NAME ]
then
	mv $OUTPUT_FILE_NAME bin/$OUTPUT_FILE_NAME
else
	echo Build failed.
	exit 1
fi

exit 0
