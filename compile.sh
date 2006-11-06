#!/bin/bash

if [ ! -d bin ]; then mkdir bin; fi

echo ">>> compiling now, this could take a little time ..."
if g++ src/*.cpp src/client/*.cpp src/common/*.cpp src/server/*.cpp \
	-I include -I /usr/include/libxml2 \
	-lSDL -lSDL_image -lSDL_mixer -lNL -lz -lgd -lxml2 \
	-o bin/openlierox
then
	echo ">>> success"
	exit 0
else
	echo ">>> error(s) reported, check the output above"
	exit -1
fi
