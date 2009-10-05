#!/bin/sh

# collection of helper-functions, mainly for compile.sh

# some simple existance-test-function
# searches for a header file in all include paths
#	$1 - header file
#	$INCLUDE_PATH - list of include paths
# returns 1 when not found
test_include_file() {
	for p in $INCLUDE_PATH; do
		[ -e "$p"/"$1" ] && return 0
	done
	return 1
}

# check if executable is there
#	$1 - executable
# return 1 if not available
test_exec() {
	which "$1" 1>/dev/null 2>/dev/null
	return $?
}

# grep specific parameter out of input-stream
#	$1 - parameter-name
# example: stdin=-Iinclude, $1=-I  =>  stdout=include
grep_param() {
	grepexpr=$(echo $1 | sed -e "s/-/[-]/")
	for p in $(xargs); do
		echo "$p" | grep "$grepexpr" | sed -e "s/$1//"
	done
}

# simple own sdl-config
# tries to behave like sdl-config
# replacement for sdl-config if not available
#	$1 = --libs => lib-flags for linker
#	$1 = --cflags => compiler-flags
#	$INCLUDE_PATH - used for include-paths for compiler flags
# prints out the flags on stdout
own_sdl_config() {
	[ "$1" = "--libs" ] && echo "-lSDL"
	if [ "$1" = "--cflags" ]; then
		for d in $INCLUDE_PATH; do
			[ -d "$d/SDL" ] && echo -n "-I$d/SDL "
		done
		echo ""
	fi
}

# simply own xml2-config
# tries to behave like xml2-config
# replacement for xml2-config if not available
#	$1 = --libs => lib-flags for linker
#	$1 = --cflags => compiler-flags
#	$INCLUDE_PATH - used for include-paths for compiler flags
# prints out the flags on stdout
own_xml2_config() {
	[ "$1" = "--libs" ] && echo "-lz -lxml2"
	if [ "$1" = "--cflags" ]; then
		for d in $INCLUDE_PATH; do
			[ -d "$d/libxml2" ] && echo -n "-I$d/libxml2 "
		done
		echo ""
	fi
}

# get somehow a version-string (like "0.57_beta4_r1090")
# reads the file VERSION if available or else reads out from Version.h
# if SVN-data is available, it adds the revision number to the string
# prints out the version on stdout
get_olx_version() {
	VERSION=""
	if [ -e VERSION ]; then
		VERSION=$(cat VERSION)
	else
		VERSION=$(grep LX_VERSION include/Version.h | \
			grep define | grep -o -e "\".*\"" | cut -d "\"" -f 2)
	fi
	if [ -d .svn ]; then
		if type svn >/dev/null 2>&1; then
			VERSION="${VERSION}_r$(svn info | grep "Revision:" | cut -d " " -f 2)"
		fi
	fi
	if [ -d .git ]; then
		if type git >/dev/null 2>&1; then
			VERSION="${VERSION}_r$(git describe | sed 's/.*-\(.*\)-.*/\1/')"
			# Warning: version = number of commits since last tagged object
			# and it may be different from SF.net git if you merged some commits
		fi
	fi
	echo "$VERSION"
}

# builds a parameter string for a list of paths
#	$1 - parameter prefix (like "-I ")
#	$2 - list of paths/files
#	$3 - list of path-suffixes which will be added to each entry of first list
#	"$3" = "" => only $2 is used
# example: $* = -L "lib1 lib2 lib3"  =>  stdout = "-L lib1 -L lib2 -L lib3"
# example: $* = -I "p1 p2" "/SDL /libxml2"  =>  stdout = "-I p1/SDL -I p2/libxml"
# prints out the string on stdout
build_param_str() {
	for p in $2; do
		if [ "$3" = "" ]; then
			echo -n "$1 ${p} "
		else
			for a in $3; do
				echo -n "$1 ${p}${a} "
			done
		fi
	done
	echo ""
}

