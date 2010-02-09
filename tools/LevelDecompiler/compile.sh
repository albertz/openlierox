#!/bin/bash

mkdir bin 2>/dev/null

g++ src/*.cpp `wx-config --cflags` -o bin/LevelDecompiler `wx-config --libs` -lz
