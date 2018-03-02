#!/bin/bash

mkdir -p bin

g++ src/*.cpp `wx-config --cflags` -o bin/LevelDecompiler `wx-config --libs` -lz
