#!/bin/bash

cd "$(dirname "$0")"
mkdir -p bin

OLX_SRCS="$(sed -n -e "/SET(OLX_SRCS/,/)/ { s/SET(OLX_SRCS //g; s/[[:blank:]]/\n/g; s/)//g; p; }" CMakeLists.txt)"
INCLUDES="$(sed -n -e "/INCLUDE_DIRECTORIES(/ { s/INCLUDE_DIRECTORIES(/-I/g; s/[[:blank:]]/\n/g; s/)//g; p; }" CMakeLists.txt)"

g++ src/*.cpp $OLX_SRCS \
$INCLUDES \
-o bin/gamedecompiler \
`sdl-config --cflags --libs` -Wno-write-strings -ggdb \
-lSDL -lSDL_image -lpthread -lz -lxml2
