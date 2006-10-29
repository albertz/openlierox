#!/bin/bash

g++ src/*.cpp src/client/*.cpp src/common/*.cpp src/server/*.cpp \
	-I include -I /usr/include/libxml2 \
	-lSDL -lSDL_image -lNL -lz -lgd -lxml2 \
	-o bin/openlierox
