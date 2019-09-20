#!/bin/bash

# USE MINGW/WINE TO BUILD RGEO FOR WIN32, WITH 3D RENDERING VIA LEGACY OPENGL.

GCC_PATH=~/compi/mingw471

EXE_NAME="shiet_render_opengl.dll"
BUILD_OPTIONS="-shared -nostdinc -s -ansi -pedantic -Wall -march=pentium -I./src/ -I$GCC_PATH/lib/gcc/mingw32/4.7.1/include -I$GCC_PATH/lib/gcc/mingw32/4.7.1/include/c++/mingw32/ -I$GCC_PATH/lib/gcc/mingw32/4.7.1/include/c++ -I$GCC_PATH/include"

SRC_FILES="
src\\shiet_lib\\render\\render_opengl.c
src\\shiet_lib\\render\\rasterizer\\opengl\\rasterizer_opengl.c
src\\shiet_lib\\render\\rasterizer\\opengl\\surface_opengl.c
src\\shiet_lib\\render\\window\\win32\\window_win32.c
"

echo "Building OpenGL..."
wine "$GCC_PATH/bin/gcc.exe" $BUILD_OPTIONS -o $EXE_NAME $SRC_FILES -lm -lgdi32 -lwinmm -lopengl32 -lglu32

if [ -e $EXE_NAME ]
then
	mv $EXE_NAME bin/$EXE_NAME
else
	echo Build failed.
	exit 1
fi

exit 0
