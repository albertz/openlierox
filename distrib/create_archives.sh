#!/bin/bash

function usage() {
	echo "$0 [--win32zip] [--srctarbz] [--all]"
	exit
}

ENABLE_WIN32ZIP=0
ENABLE_SRCTARBZ=0

for arg in $*; do
	case $arg in
		--help) usage;;
		--win32zip) ENABLE_WIN32ZIP=1;;
		--srctarbz) ENABLE_SRCTARBZ=1;;
		--all) ENABLE_WIN32ZIP=1; ENABLE_SRCTARBZ=1;;
		*) usage;;
	esac
done

if [ $ENABLE_WIN32ZIP == 0 ] && [ $ENABLE_SRCTARBZ == 0 ]; then
	echo "you have to enable at least one target"
	usage
fi

VERSION="$(cat ../VERSION)"
echo ">>> preparing $VERSION archives ..."

cd ..
SRC_FILES="src include libs"
STD_FILES="VERSION CMakeLists.txt *.sh *.bat build/Xcode/OpenLieroX-Info.plist build/Xcode/OpenLieroX.xcodeproj debian"
DOC_FILES="COPYING.LIB DEPS doc"
DAT_FILES="share"

export SRC_RELEASE="$SRC_FILES $STD_FILES $DOC_FILES $DAT_FILES"
export WIN32_RELEASE="doc COPYING.LIB share/gamedir/* distrib/win32/*"

export ARCHIVE_PREFIX="distrib/tarball/OpenLieroX_${VERSION}"
export SRC_PREFIX="${ARCHIVE_PREFIX}.src"
export WIN32_PREFIX="${ARCHIVE_PREFIX}.win32"

# cleaning up
rm -rf distrib/OpenLieroX 2>/dev/null
rm ${SRC_PREFIX}.tar* 2>/dev/null
rm ${WIN32_PREFIX}.zip 2>/dev/null
mkdir -p distrib/tarball

if [ $ENABLE_SRCTARBZ == 1 ]; then
echo ">>> creating source tar.bz ..."
ln -s .. distrib/OpenLieroX
for FILE in $SRC_RELEASE; do
	[ -e ${SRC_PREFIX}.tar ] && MOD_FLAG=-r || MOD_FLAG=-c
	cd distrib
	tar --exclude=.svn --exclude=*~ $MOD_FLAG -hf \
		../${SRC_PREFIX}.tar \
		OpenLieroX/$FILE
	cd ..
done
rm distrib/OpenLieroX
bzip2 -9 ${SRC_PREFIX}.tar
fi

# echo ">>> creating source zip ..."
# [ -d distrib/srctmp ] && rm -rf distrib/srctmp
# mkdir -p distrib/srctmp
# tar -xjf ${SRC_PREFIX}.tar.bz2 -C distrib/srctmp
# cd distrib/srctmp
# zip -r -9 ../../${SRC_PREFIX}.zip * >/dev/null
# cd ../..
# rm -rf distrib/srctmp

if [ $ENABLE_WIN32ZIP == 1 ]; then
echo ">>> creating win32 zip ..."
cd distrib
[ -d OpenLieroX ] && rm -rf OpenLieroX
mkdir OpenLieroX
cd ..
tar --exclude=.svn -c $WIN32_RELEASE | tar -x -C distrib/OpenLieroX
cd distrib/OpenLieroX
mv share/gamedir/* .
rm -r share
mv distrib/win32/* .
rm -r distrib
cd ..
zip -r -9 ../${WIN32_PREFIX}.zip OpenLieroX >/dev/null
rm -rf OpenLieroX
# we are now in distrib again
fi
