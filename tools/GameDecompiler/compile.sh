#!/bin/bash

if [ \! -d bin ] ; then mkdir bin ; fi

g++ src/*.cpp -o bin/GameDecompiler `sdl-config --cflags --libs` -Wno-write-strings
