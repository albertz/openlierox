#!/bin/bash

mkdir -p bin
g++ src/*.cpp -o bin/udpmasterserver -O3
