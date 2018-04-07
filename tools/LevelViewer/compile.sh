#!/bin/bash

mkdir -p bin

g++ src/*.cpp `wx-config --cflags` -o bin/LevelViewer `wx-config --libs` -lz
