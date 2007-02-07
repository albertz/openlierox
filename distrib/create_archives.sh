#!/bin/bash

[ "$VERSION" == "" ] && VERSION=0.57_cur$(date +%Y%m%d) && ISCURRELEASE=1
echo ">>> preparing $VERSION archives ..."

cd ..
echo $VERSION > VERSION

SRC_FILES="src/*.cpp src/common/*.cpp src/client/*.cpp src/server/*.cpp include/*.h"
STD_FILES="VERSION CMakeLists.txt compile.sh install.sh start.sh hawknl/install.sh hawknl/download.sh"
DOC_FILES="COPYING.LIB DEPS doc/*"
DAT_FILES="share/gamedir/* share/OpenLieroX.png"

export SRC_RELEASE="$SRC_FILES $STD_FILES $DOC_FILES $DAT_FILES"
export WIN32_RELEASE="doc/* COPYING.LIB share/gamedir/* distrib/win32/*"

export ARCHIVE_PREFIX="distrib/tarball/OpenLieroX_${VERSION}"
export SRC_PREFIX="${ARCHIVE_PREFIX}.src"
export WIN32_PREFIX="${ARCHIVE_PREFIX}.win32"

if [ "$ISCURRELEASE" == "1" ]; then
	echo ">>> deleting previous archives ..."
	rm distrib/tarball/OpenLieroX_0.57_cur*
fi

echo ">>> collecting file list ..."
# this is a very very dirty hack, but I don't know how to do better
export FILELIST=""
for f in $SRC_RELEASE; do
	# filter out the .svn stuff
	TMP=$(find "$f" -type f -printf "\"%h/%f\"\n" | grep -v .svn)
	# remove the newlines -> fill FILELIST
	for f2 in $TMP; do
		FILELIST="$FILELIST $f2"
	done
done

echo ">>> creating source zip ..."
sh -c "zip -r -9 ${SRC_PREFIX}.zip $FILELIST" >/dev/null

echo ">>> creating source tar.bz ..."
tar --exclude=.svn -jcf ${SRC_PREFIX}.tar.bz $SRC_RELEASE

echo ">>> creating win32 zip ..."
cd distrib
[ -d win32tmp ] && rm -rf win32tmp
mkdir win32tmp
cd ..
tar --exclude=.svn -c $WIN32_RELEASE | tar -x -C distrib/win32tmp
cd distrib/win32tmp
mv share/gamedir/* .
rm -r share
mv distrib/win32/* .
rm -r distrib
zip -r -9 ../../${WIN32_PREFIX}.zip * >/dev/null
cd ..
rm -rf win32tmp
# we are now in distrib again

[ "$ISCURRELEASE" == "1" ] && echo $VERSION > web/VERSION
