#!/bin/bash

# this is the install-script for the game
# I will install the game to your system :)
# the following variables will be used:
#   SYSTEM_DATA_DIR		- the dir, where the game-data will be installed into
#						  it should be the same as used for compilation
#						  default=/usr/share
#   BIN_DIR				- the dir, where the binary will be put into
#						  default=/usr/bin
#	DOC_DIR				- the dir, where I will place the docs into
#						  default=/usr/share/doc
#	PREFIX				- additional prefix for everything
# ( all paths are only prefixes, I will add /OpenLieroX at the end )

if [ ! -x bin/openlierox ]; then
	echo "ERROR: the binary is missing" >&2
	echo "please run ./compile.sh to compile it first"
	exit -1
fi

[ "$SYSTEM_DATA_DIR" == "" ] && SYSTEM_DATA_DIR=/usr/share
[ "$BIN_DIR" == "" ] && BIN_DIR=/usr/bin
[ "$DOC_DIR" == "" ] && DOC_DIR=/usr/share/doc

if [ "$PREFIX" != "" ]; then
	SYSTEM_DATA_DIR=$PREFIX/$SYSTEM_DATA_DIR
	BIN_DIR=$PREFIX/$BIN_DIR
	DOC_DIR=$PREFIX/$DOC_DIR
fi

cp bin/openlierox $BIN_DIR/
echo "> $BIN_DIR/openlierox copied"
mkdir -p $SYSTEM_DATA_DIR/OpenLieroX
cp -r share/gamedir/* $SYSTEM_DATA_DIR/OpenLieroX/
echo "> $SYSTEM_DATA_DIR/OpenLieroX copied"
mkdir -p $DOC_DIR/OpenLieroX
cp -r doc/* $DOC_DIR/OpenLieroX/
echo "> $DOC_DIR/OpenLieroX copied"

# cleanup
for d in $DOC_DIR/OpenLieroX $SYSTEM_DATA_DIR/OpenLieroX; do
       find $d  \( \( -true -a \
                \( -name '#*#' -o -name '.*~' -o -name '*~' -o -name DEADJOE \
                 -o -name '*.orig' -o -name '*.rej' -o -name '*.bak' \
                 -o -name '.svn' \
                 -o -name '.*.orig' -o -name .*.rej -o -name '.SUMS' \
                 -o -name TAGS -o -name core -o \( -path '*/.deps/*' -a -name '*.P' \) \
                \) -exec rm -f {} \; \) -o \
                \( -type d -a -name autom4te.cache -prune -exec rm -rf {} \; \) \)
done

echo ">>> installation ready"
