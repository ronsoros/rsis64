#!/bin/sh
CC=gcc
LDFLAGS='-Lwinsdl/SDL2-2.0.5/i686-w64-mingw32/bin -lSDL2'
$CC -std=c99 -pedantic -Iwinsdl/SDL2-2.0.5/i686-w64-mingw32/include/SDL2 -Ofast vm.c -o vm -DTEST $LDFLAGS
$CC asm.c -o asm
$CC -Ofast -c vm.c -o libvm.o
#$CC rsis64-cc.c -o rsis64-cc
cd cucu
./build.sh
cd ..
