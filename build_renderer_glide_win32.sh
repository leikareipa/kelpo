#!/bin/bash

# Build shiet's Glide 3.x renderer for Win32 using MinGW via Wine in Linux.

GCC_PATH=~/compi/mingw471
OUTPUT_FILE_NAME="shiet_renderer_glide3.dll"
BUILD_OPTIONS="-shared -nostdinc -s -ansi -pedantic -Wall -march=pentium -I ./src/ -isystem $GCC_PATH/lib/gcc/mingw32/4.7.1/include -isystem $GCC_PATH/include"

SRC_FILES="
src\\shiet_lib\\renderer\\renderer_glide3.c
src\\shiet_lib\\renderer\\rasterizer\\glide3\\rasterizer_glide3.c
src\\shiet_lib\\renderer\\rasterizer\\glide3\\surface_glide3_win32.c
src\\shiet_lib\\renderer\\window\\win32\\window_win32.c
"

wine "$GCC_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $OUTPUT_FILE_NAME $SRC_FILES -lm -lgdi32 -lglide/glide3x

if [ -e $OUTPUT_FILE_NAME ]
then
	mv $OUTPUT_FILE_NAME bin/$OUTPUT_FILE_NAME
else
	echo Build failed.
	exit 1
fi

exit 0
