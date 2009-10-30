#!/bin/sh

# this is the compile-script for a simple compilation of the game
# the following variables will be used:
#	SYSTEM_DATA_DIR	- the global data dir for the game; default=/usr/share
#	COMPILER		- sets the compiler
#	CXXFLAGS		- some other compiler flags
#	LDFLAGS			- some other linker flags
#	INCLUDE_PATH	- adds one or more include paths
#	LIB_PATH		- adds one or more lib paths
#	DEBUG			- if set to 1, the game will compiled with debug-info
#					( deactivated by default )
#	DEBUGINFO	- adds debug info
#					( enabled by default )
#	HAWKNL_BUILTIN	- if set to 1, HawkNL will be builtin
#					( disabled by default )
#	LIBZIP_BUILTIN	- if set to 1, libzip will be builtin
#					( disabled by default )
#	X11				- if set to 1, X11 clipboard/notifying will be used (and linked against libX11)
#					( activated by default )
#	G15				- if set to 1, G15 support will be builtin (and linked against required libraries)
#					( disabled by default )
#	BREAKPAD		- if set to 1, Google Breakpad is used
#					( enabled by default in case of x86, disabled otherwise by default )
#	GCOREDUMPER		- if set to 1, Google Coredumper is used
#					( disabled by default )
#	VERSION			- version number; like 0.57_beta2
#					  if not set, the function functions.sh:get_olx_version
#					  generates the string automatically

cd "$(dirname "$0")"

. ./functions.sh

# check variables and set default values if unset
[ "$SYSTEM_DATA_DIR" = "" ] && SYSTEM_DATA_DIR=/usr/share/games
[ "$DEBUG" = "" ] && DEBUG=0
[ "$COMPILER" = "" ] && COMPILER=g++
[ "$DEBUGINFO" = "" ] && DEBUGINFO=1
[ "$X11" = "" ] && X11=1
[ "$G15" = "" ] && G15=0
[ "$BREAKPAD" = "" ] && {
	BREAKPAD=0
	[ "$(uname -m)" = "i486" ] && BREAKPAD=1
	[ "$(uname -m)" = "i586" ] && BREAKPAD=1
	[ "$(uname -m)" = "i686" ] && BREAKPAD=1
}
[ "$GCOREDUMPER" = "" ] && GCOREDUMPER=0
[ "$VERSION" = "" ] && VERSION=$(get_olx_version)

# add standards to include path list
INCLUDE_PATH="$INCLUDE_PATH /usr/include /usr/local/include /sw/include /opt/local/include"
LIB_PATH="$LIB_PATH /sw/lib /opt/local/lib"

# handle SDL specific compiler settings and get include-dirs
sdlconfig=""
test_exec pkg-config && pkg-config sdl && sdlconfig="pkg-config sdl"
[ "$sdlconfig" = "" ] && test_exec sdl-config && sdlconfig="sdl-config"
[ "$sdlconfig" = "" ] && sdlconfig="own_sdl_config"
xmlconfig=""
test_exec pkg-config && pkg-config libxml-2.0 && xmlconfig="pkg-config libxml-2.0"
[ "$xmlconfig" = "" ] && test_exec xml2-config && xmlconfig="xml2-config"
[ "$xmlconfig" = "" ] && xmlconfig="own_xml2_config"

INCLUDE_PATH="$INCLUDE_PATH $($sdlconfig --cflags | grep_param -I)"
LIB_PATH="$LIB_PATH $($sdlconfig --libs | grep_param -L)"
INCLUDE_PATH="$INCLUDE_PATH $($xmlconfig --cflags | grep_param -I)"
LIB_PATH="$LIB_PATH $($xmlconfig --libs | grep_param -L)"

echo "--- OpenLieroX compile.sh ---"

# do some simple checks
ALL_FINE=1
type $COMPILER 1>/dev/null 2>/dev/null || \
	{ echo "ERROR: g++ not found" >&2; ALL_FINE=0; }
test_include_file libxml/parser.h || \
	{ echo "ERROR: libxml2 headers not found" >&2; ALL_FINE=0; }
test_include_file curl/curl.h || \
	{ echo "ERROR: libcurl headers not found" >&2; ALL_FINE=0; }
test_include_file SDL.h || \
	{ echo "ERROR: SDL headers not found" >&2; ALL_FINE=0; }
test_include_file SDL_image.h || \
	{ echo "ERROR: SDL_image.h not found" >&2; ALL_FINE=0; }
test_include_file SDL_mixer.h || \
	{ echo "ERROR: SDL_mixer.h not found" >&2; ALL_FINE=0; }
test_include_file zlib.h || \
	{ echo "ERROR: zlib header not found" >&2; ALL_FINE=0; }
test_include_file gd.h || \
	{ echo "ERROR: gd header not found" >&2; ALL_FINE=0; }


if [ "$G15" = "1" ]; then
    test_include_file g15daemon_client.h || \
	{ echo "ERROR: g15daemon_client header not found" >&2; ALL_FINE=0; }
    test_include_file libg15render.h || \
	{ echo "ERROR: libg15render header not found" >&2; ALL_FINE=0; }
fi

if [ "$HAWKNL_BUILTIN" = "1" ]; then
	HAWKNL_GCC_PARAM="\
		libs/hawknl/src/crc.c \
		libs/hawknl/src/errorstr.c \
		libs/hawknl/src/nl.c \
		libs/hawknl/src/sock.c \
		libs/hawknl/src/group.c \
		libs/hawknl/src/loopback.c \
		libs/hawknl/src/err.c \
		libs/hawknl/src/thread.c \
		libs/hawknl/src/mutex.c \
		libs/hawknl/src/condition.c \
		libs/hawknl/src/nltime.c \
 		-I libs/hawknl/include"
else
	test_include_file nl.h || \
	test_include_file hawknl/nl.h || \
		{	echo "ERROR: HawkNL header not found" >&2;
			echo "try the following: export HAWKNL_BUILTIN=1";
			ALL_FINE=0; }
	HAWKNL_GCC_PARAM="-lNL"
fi

if [ "$LIBZIP_BUILTIN" = "1" ]; then
	LIBZIP_GCC_PARAM="libs/libzip/*.c -I libs/libzip"
else
	test_include_file zip.h || \
		{	echo "ERROR: libzip header not found" >&2;
			echo "try the following: export LIBZIP_BUILTIN=1";
			ALL_FINE=0; }
	LIBZIP_GCC_PARAM="-lzip"
fi

[ $ALL_FINE = 0 ] && \
	{ echo "errors detected, exiting"; exit 1; }


# report the used settings
[ "$VERSION" != "" ] && echo "* version $VERSION"
echo "* the global search-path of the game will be $SYSTEM_DATA_DIR/OpenLieroX"
[ "$DEBUG" = "1" ] && \
	echo "* debug-thingies in the game will be activated" || \
	echo "* you will not see any debug-crap"
echo "* $COMPILER will be used for compilation"
[ "$DEBUGINFO" = "1" ] && \
	echo "* debugging-data will be included in the bin" || \
	echo "* debugging-data will not been added explicitly to the bin"
[ "$CXXFLAGS" = "" ] && \
	echo "* none additional compiler-flags will be used" || \
	echo "* the following additional compiler-flags will be used: $CXXFLAGS"
[ "$X11" = "1" ] && \
	echo "* X11 clipboard/notify support is activated" || \
	echo "* X11 clipboard/notify support is not activated"
[ "$G15" = "1" ] && \
        echo "* G15 support is activated" || \
        echo "* G15 support is not activated"
[ "$HAWKNL_BUILTIN" = "1" ] && \
	echo "* HawkNL support will be built into the binary" || \
	echo "* the binary will be linked dynamically against HawkNL-lib"
[ "$LIBZIP_BUILTIN" = "1" ] && \
	echo "* libzip support will be built into the binary" || \
	echo "* the binary will be linked dynamically against libzip"
[ "$BREAKPAD" = "1" ] && \
	echo "* Google Breakpad will be used" || \
	echo "* fallback crashhandler will be used"
[ "$GCOREDUMPER" = "1" ] && \
	echo "* Googles coredumper will be used" || \
	echo "* debuggers coredumper will be used"



echo ">>> compiling now, this could take some time ..."
mkdir -p bin
if $COMPILER \
	src/*.cpp src/client/*.cpp \
	src/client/DeprecatedGUI/*.cpp src/client/SkinnedGUI/*.cpp \
	src/common/*.cpp src/server/*.cpp \
	libs/coredumper/src/*.c \
	$LIBZIP_GCC_PARAM \
	$HAWKNL_GCC_PARAM \
	-I include -I libs/pstreams \
	-I libs/coredumper/src \
	$(build_param_str -I "$INCLUDE_PATH" "/. /libxml2 /hawknl") \
	$(build_param_str -L "$LIB_PATH") \
	$($sdlconfig --cflags) \
	$($sdlconfig --libs) \
	$($xmlconfig --cflags) \
	$($xmlconfig --libs) \
	src/breakpad/ExtractInfo.cpp \
	$( [ "$BREAKPAD" = "1" ] && \
		echo "-I src/breakpad/external/src " && \

		echo "src/breakpad/DumpSyms.cpp" && \
		echo "src/breakpad/BreakPad.cpp" && \
		echo "src/breakpad/on_demand_symbol_supplier.cpp" && \

		echo "src/breakpad/external/src/client/minidump_file_writer.cc" && \
		echo "src/breakpad/external/src/client/linux/handler/exception_handler.cc" && \
		echo "src/breakpad/external/src/client/linux/handler/linux_thread.cc" && \
		echo "src/breakpad/external/src/client/linux/minidump_writer/linux_dumper.cc" && \
		echo "src/breakpad/external/src/client/linux/minidump_writer/minidump_writer.cc" && \

		echo "src/breakpad/external/src/common/convert_UTF.c" && \
		echo "src/breakpad/external/src/common/md5.c" && \
		echo "src/breakpad/external/src/common/string_conversion.cc" && \
		echo "src/breakpad/external/src/common/linux/file_id.cc" && \
		echo "src/breakpad/external/src/common/linux/guid_creator.cc" && \
		echo "src/breakpad/external/src/common/linux/dump_symbols.cc" && \
		echo "src/breakpad/external/src/common/linux/module.cc" && \
		echo "src/breakpad/external/src/common/linux/stabs_reader.cc" && \
		echo "src/breakpad/external/src/common/dwarf/bytereader.cc" && \
		echo "src/breakpad/external/src/common/dwarf/dwarf2diehandler.cc" && \
		echo "src/breakpad/external/src/common/dwarf/dwarf2reader.cc" && \
		echo "src/breakpad/external/src/common/linux/dump_dwarf.cc" && \

		echo "src/breakpad/external/src/processor/basic_code_modules.cc" && \
		echo "src/breakpad/external/src/processor/basic_source_line_resolver.cc" && \
		echo "src/breakpad/external/src/processor/call_stack.cc" && \
		echo "src/breakpad/external/src/processor/logging.cc" && \
		echo "src/breakpad/external/src/processor/minidump.cc" && \
		echo "src/breakpad/external/src/processor/minidump_processor.cc" && \
		echo "src/breakpad/external/src/processor/pathname_stripper.cc" && \
		echo "src/breakpad/external/src/processor/process_state.cc" && \
		echo "src/breakpad/external/src/processor/simple_symbol_supplier.cc" && \
		echo "src/breakpad/external/src/processor/stackwalker_amd64.cc" && \
		echo "src/breakpad/external/src/processor/stackwalker_ppc.cc" && \
		echo "src/breakpad/external/src/processor/stackwalker_sparc.cc" && \
		echo "src/breakpad/external/src/processor/stackwalker_x86.cc" && \
		echo "src/breakpad/external/src/processor/stackwalker.cc"
	) \
	$( [ "$BREAKPAD" != "1" ] && echo "-DNBREAKPAD" ) \
	-lSDL_image -lSDL_mixer -lgd -pthread -lz -lcurl \
	-DSYSTEM_DATA_DIR="\"$SYSTEM_DATA_DIR\"" \
	$( [ "$DEBUG" = "1" ] && echo "-DDEBUG" ) \
	$( [ "$VERSION" != "" ] && echo -DLX_VERSION="\"$VERSION\"" ) \
	$( [ "$DEBUGINFO" = "1" ] && echo "-g" ) \
	$( [ "$X11" = "1" ] && echo "-DX11 -lX11" ) \
	$( [ "$G15" = "1" ] && echo "-DWITH_G15 -lg15daemon_client -lg15render" ) \
	$( [ "$GCOREDUMPER" = "1" ] && echo "-DGCOREDUMPER -Ilibs/coredumper/src" ) \
	$CXXFLAGS \
	$LDFLAGS \
	-o bin/openlierox
then
	echo ">>> success"
	exit 0
else
	echo ">>> error(s) reported, check the output above"
	exit 1
fi

