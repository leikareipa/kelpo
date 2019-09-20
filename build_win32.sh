#!/bin/bash

GCC_PATH=~/compi/mingw471

EXE_NAME="shiet.exe"
BUILD_OPTIONS="-nostdinc -s -ansi -pedantic -Wall -march=pentium -I./src/ -I$GCC_PATH/lib/gcc/mingw32/4.7.1/include -I$GCC_PATH/lib/gcc/mingw32/4.7.1/include/c++/mingw32/ -I$GCC_PATH/lib/gcc/mingw32/4.7.1/include/c++ -I$GCC_PATH/include"

SRC_FILES="
src\\main.c
src\\shiet\\interface.c
"

echo "Building OpenGL..."
wine "$GCC_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $EXE_NAME $SRC_FILES -lm -lgdi32 -lwinmm

if [ -e $EXE_NAME ]
then
	mv $EXE_NAME bin/$EXE_NAME
else
	echo Build failed.
	exit 1
fi

exit 0
