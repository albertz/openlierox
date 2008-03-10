#!/bin/bash

[ "$VERSION" == "" ] && VERSION=0.57_cur$(date +%Y%m%d) && ISCURRELEASE=1
echo ">>> preparing $VERSION archives ..."

cd ..
echo $VERSION > VERSION

SRC_FILES="src include hawknl pstreams boost_process"
STD_FILES="VERSION CMakeLists.txt *.sh *.bat build/Xcode debian"
DOC_FILES="COPYING.LIB DEPS doc"
DAT_FILES="share/gamedir share/*.png share/*.icns share/*.ico share/*.svg share/*.xpm"

export SRC_RELEASE="$SRC_FILES $STD_FILES $DOC_FILES $DAT_FILES"
export WIN32_RELEASE="doc COPYING.LIB share/gamedir/* distrib/win32/*"

export ARCHIVE_PREFIX="distrib/tarball/OpenLieroX_${VERSION}"
export SRC_PREFIX="${ARCHIVE_PREFIX}.src"
export WIN32_PREFIX="${ARCHIVE_PREFIX}.win32"

if [ "$ISCURRELEASE" == "1" ]; then
	echo ">>> deleting previous archives ..."
	rm distrib/tarball/OpenLieroX_0.57_cur*
fi

# cleaning up
rm -rf distrib/OpenLieroX 2>/dev/null
rm ${SRC_PREFIX}.tar* 2>/dev/null
rm ${WIN32_PREFIX}.zip 2>/dev/null

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

# echo ">>> creating source zip ..."
# [ -d distrib/srctmp ] && rm -rf distrib/srctmp
# mkdir -p distrib/srctmp
# tar -xjf ${SRC_PREFIX}.tar.bz2 -C distrib/srctmp
# cd distrib/srctmp
# zip -r -9 ../../${SRC_PREFIX}.zip * >/dev/null
# cd ../..
# rm -rf distrib/srctmp

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

[ "$ISCURRELEASE" == "1" ] && echo $VERSION > web/VERSION
