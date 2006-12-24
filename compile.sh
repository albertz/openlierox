#!/bin/bash

# this is the compile-script for a simple compilation of the game
# the following variables will be used:
#   SYSTEM_DATA_DIR		- the global data dir for the game; default=/usr/share
#	CPPFLAGS			- some other g++ flags
#	DEBUG				- if set to 1, the game will compiled with debug-info
#	HAWKNL_BUILTIN		- if set to 1, HawkNL will be builtin

# check variables and set default values if unset
[ "$SYSTEM_DATA_DIR" == "" ] && SYSTEM_DATA_DIR=/usr/share
[ "$DEBUG" == "" ] && DEBUG=0
[ "$COMPILER" == "" ] && COMPILER=g++

function test_include_file() {
	[ -e /usr/include/$1 -o -e /usr/local/include/$1 ]
	return $?
}

# do some simple checks
type $COMPILER 1>/dev/null 2>/dev/null || \
	{ echo "ERROR: g++ not found" >&2; exit -1; }
test_include_file libxml2/libxml/parser.h || \
	{ echo "ERROR: libxml2 headers not found" >&2; exit -1; }
test_include_file SDL/SDL.h || \
	{ echo "ERROR: SDL headers not found" >&2; exit -1; }
test_include_file SDL/SDL_image.h || \
	{ echo "ERROR: SDL_image.h not found" >&2; exit -1; }
test_include_file SDL/SDL_mixer.h || \
	{ echo "ERROR: SDL_mixer.h not found" >&2; exit -1; }
test_include_file zlib.h || \
	{ echo "ERROR: zlib header not found" >&2; exit -1; }
test_include_file gd.h || \
	{ echo "ERROR: gd header not found" >&2; exit -1; }

if [ "$HAWKNL_BUILTIN" == "1" ]; then
	echo "INFO: HawkNL will be builtin"
	cd hawknl
	install.sh || \
		{ echo "ERROR: problems while installing HawkNL" >&2; exit -1; }
	cd ..
	HAWKNL_GCC_PARAM="hawknl/src/*.c -I hawknl/include"
else
	test_include_file nl.h || \
		{ echo "ERROR: HawkNL header not found" >&2; exit -1; }
	HAWKNL_GCC_PARAM="-lNL"
fi

mkdir -p bin

echo ">>> compiling now, this could take some time ..."
if $COMPILER src/*.cpp src/client/*.cpp src/common/*.cpp src/server/*.cpp \
	$HAWKNL_GCC_PARAM \
	-I include -I /usr/include/libxml2 \
	-lSDL -lSDL_image -lSDL_mixer -lz -lgd -lxml2 \
	-DSYSTEM_DATA_DIR="\"$SYSTEM_DATA_DIR\"" \
	-DDEBUG="$DEBUG" \
	$( [ "$DEBUG" == "1" ] && echo -ggdb ) \
	$CPPFLAGS \
	-o bin/openlierox
then
	echo ">>> success"
	exit 0
else
	echo ">>> error(s) reported, check the output above"
	exit -1
fi

