#!/bin/bash

VERSION=0.57

cd ..

SRC_FILES="src/common/*.cpp src/client/*.cpp src/server/*.cpp include/*.h"
STD_FILES="CMakeLists.txt compile.sh install.sh start.sh"
DOC_FILES="COPYING.LIB DEPS doc/*"
DAT_FILES="share/gamedir/* share/OpenLieroX.png"

SRC_RELEASE="$SRC_FILES $STD_FILES $DOC_FILES $DAT_FILES"
SRC_PREFIX="distrib/tarball/OpenLieroX_${VERSION}.src"

# TODO: BIN_RELEASE

tar -jcf ${SRC_PREFIX}.tar.bz $SRC_RELEASE
zip -r -9 ${SRC_PREFIX}.zip $SRC_RELEASE
