#!/bin/bash

# this is the compile-script for a simple compilation of the game
# the following variables will be used:
#	SYSTEM_DATA_DIR		- the global data dir for the game; default=/usr/share
#	COMPILER			- sets the compiler
#	CXXFLAGS			- some other compiler flags
#	LDFLAGS				- some other linker flags
#	INCLUDE_PATH		- adds one or more include paths
#	LIB_PATH			- adds one or more lib paths
#	DEBUG				- if set to 1, the game will compiled with debug-info
#	ACTIVATE_GDB		- sets the -ggdb flag
#						( it will automatically be activated, if you haven't
#						  set it manually and DEBUG==1 )
#	HAWKNL_BUILTIN		- if set to 1, HawkNL will be builtin
#	VERSION				- version number; like 0.57_beta2
#						  if not set, the file VERSION will be read
#						  if file VERSION does not exist, the default
#						  (defined in LieroX.h) will be used

# check variables and set default values if unset
[ "$SYSTEM_DATA_DIR" == "" ] && SYSTEM_DATA_DIR=/usr/share
[ "$DEBUG" == "" ] && DEBUG=0
[ "$COMPILER" == "" ] && COMPILER=g++
[ "$ACTIVATE_GDB" == "" ] && [ "$DEBUG" == "1" ] && ACTIVATE_GDB=1
[ "$VERSION" == "" ] && [ -e VERSION ] && VERSION=$(cat VERSION)

# add standards to include path list
INCLUDE_PATH="$INCLUDE_PATH /usr/include /usr/local/include /sw/include"
LIB_PATH="$LIB_PATH /sw/lib"

# some simple existance-test-function
function test_include_file() {
	for p in $INCLUDE_PATH; do
		[ -e $p/$1 ] && return 0
	done
	return -1
}

echo "--- OpenLieroX compile.sh ---"

# do some simple checks
ALL_FINE=1
type $COMPILER 1>/dev/null 2>/dev/null || \
	{ echo "ERROR: g++ not found" >&2; ALL_FINE=0; }
test_include_file libxml2/libxml/parser.h || \
	{ echo "ERROR: libxml2 headers not found" >&2; ALL_FINE=0; }
test_include_file SDL/SDL.h || \
	{ echo "ERROR: SDL headers not found" >&2; ALL_FINE=0; }
test_include_file SDL/SDL_image.h || \
	{ echo "ERROR: SDL_image.h not found" >&2; ALL_FINE=0; }
test_include_file SDL/SDL_mixer.h || \
	{ echo "ERROR: SDL_mixer.h not found" >&2; ALL_FINE=0; }
test_include_file zlib.h || \
	{ echo "ERROR: zlib header not found" >&2; ALL_FINE=0; }
test_include_file gd.h || \
	{ echo "ERROR: gd header not found" >&2; ALL_FINE=0; }

if [ "$HAWKNL_BUILTIN" == "1" ]; then
	if [ $ALL_FINE == 1 ]; then
		cd hawknl
		./install.sh || \
			{ echo "ERROR: problems while installing HawkNL" >&2; exit -1; }
		cd ..
	fi
	HAWKNL_GCC_PARAM="\
		hawknl/src/crc.c \
		hawknl/src/errorstr.c \
		hawknl/src/nl.c \
		hawknl/src/sock.c \
		hawknl/src/group.c \
		hawknl/src/loopback.c \
		hawknl/src/err.c \
		hawknl/src/thread.c \
		hawknl/src/mutex.c \
		hawknl/src/condition.c \
		hawknl/src/nltime.c \
 		-I hawknl/include"
else
	test_include_file nl.h || \
	test_include_file hawknl/nl.h || \
		{ echo "ERROR: HawkNL header not found" >&2; ALL_FINE=0; }
	HAWKNL_GCC_PARAM="-lNL"
fi

[ $ALL_FINE == 0 ] && \
	{ echo "errors detected, exiting"; exit -1; }


# report the used settings
[ "$VERSION" != "" ] && echo "* version $(cat VERSION)"
echo "* the global search-path of the game will be $SYSTEM_DATA_DIR/OpenLieroX"
[ "$DEBUG" == "1" ] && \
	echo "* debug-thingies in the game will be activated" || \
	echo "* you will not see any debug-crap"
echo "* $COMPILER will be used for compilation"
[ "$ACTIVATE_GDB" == "1" ] && \
	echo "* gdb-data will not been added to the bin, if they doesn't occurs here:" || \
	echo "* gdb-data will be included in the bin"
[ "$CXXFLAGS" == "" ] && \
	echo "* none additional compiler-flags will be used" || \
	echo "* the following additional compiler-flags will be used: $CXXFLAGS"
[ "$HAWKNL_BUILTIN" == "1" ] && \
	echo "* HawkNL support will be downloaded automaticaly and built into the binary" || \
	echo "* the binary will be linked dynamically against the HawkNL-lib"


mkdir -p bin

# build full include path list
INCLUDE_ADDITIONAL=". libxml2 hawknl"
INCLUDE_STRING=""
for p in $INCLUDE_PATH; do
	for a in $INCLUDE_ADDITIONAL; do
		INCLUDE_STRING="$INCLUDE_STRING -I $p/$a"
	done
done

# build full lib path list
LIB_STRING=""
for p in $LIB_PATH; do
	LIB_STRING="$LIB_STRING -L$p"
done

echo ">>> compiling now, this could take some time ..."
if $COMPILER src/*.cpp src/client/*.cpp src/common/*.cpp src/server/*.cpp \
	$HAWKNL_GCC_PARAM \
	-I include \
	$INCLUDE_STRING \
	$LIB_STRING \
	-lSDL -lSDL_image -lSDL_mixer -lz -lgd -lxml2 \
	-DSYSTEM_DATA_DIR="\"$SYSTEM_DATA_DIR\"" \
	-DDEBUG="$DEBUG" \
	$( [ "$VERSION" != "" ] && echo -DLX_VERSION="\"$VERSION\"" ) \
	$( [ "$ACTIVATE_GDB" == "1" ] && echo -ggdb ) \
	$CXXFLAGS \
	$LDFLAGS \
	-o bin/openlierox
then
	echo ">>> success"
	exit 0
else
	echo ">>> error(s) reported, check the output above"
	exit -1
fi

