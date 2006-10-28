#!/bin/bash

g++ src/*.cpp src/client/*.cpp src/common/*.cpp src/server/*.cpp \
	-I include \
	-lSDL -lSDL_image -lNL -lz -lgd \
	-o bin/openlierox
