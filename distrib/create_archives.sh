#!/bin/bash

VERSION=0.57

cd ..

SRC_FILES="src/*.cpp src/common/*.cpp src/client/*.cpp src/server/*.cpp include/*.h"
STD_FILES="CMakeLists.txt compile.sh install.sh start.sh"
DOC_FILES="COPYING.LIB DEPS doc/*"
DAT_FILES="share/gamedir/* share/OpenLieroX.png"

export SRC_RELEASE="$SRC_FILES $STD_FILES $DOC_FILES $DAT_FILES"
export SRC_PREFIX="distrib/tarball/OpenLieroX_${VERSION}.src"

# TODO: BIN_RELEASE

[ -e ${SRC_PREFIX}.tar.bz ] && rm ${SRC_PREFIX}.tar.bz
[ -e ${SRC_PREFIX}.zip ] && rm ${SRC_PREFIX}.zip

# this is a very very dirty hack, but I don't know how to do better
export FILELIST=""
for f in $SRC_RELEASE; do
	# filter out the .svn shit
	TMP=$(find "$f" -type f -printf "\"%h/%f\"\n" | grep -v .svn)
	# remove the newlines -> fill FILELIST
	for f2 in $TMP; do
		FILELIST="$FILELIST $f2"
	done
done

echo ">>> creating zip ..."
sh -c "zip -r -9 ${SRC_PREFIX}.zip $FILELIST"

echo ">>> creating tar.bz ..."
tar --exclude=.svn -jcf ${SRC_PREFIX}.tar.bz $SRC_RELEASE
