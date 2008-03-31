#!/bin/bash

if [ -n bin ] ; then mkdir bin ; fi

g++ src/*.cpp `wx-config --cflags` -o bin/LevelCompiler `wx-config --libs`
