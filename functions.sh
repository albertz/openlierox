#!/bin/bash

# collection of helper-functions, mainly for compile.sh

# some simple existance-test-function
function test_include_file() {
	for p in $INCLUDE_PATH; do
		[ -e "$p"/"$1" ] && return 0
	done
	return -1
}

# check if executable is there
function test_exec() {
	which "$1" 1>/dev/null 2>/dev/null
	return $?
}

# grep parameter $1 out of input-stream
# example: in=-Iinclude $1=-I  =>  out=include
function grep_param() {
	for p in $(xargs); do
		echo "$p" | grep "${1/-/\-}" | sed -e "s/$1//"
	done
}

# simply own sdl-config
function own_sdl_config() {
	[ "$1" == "--libs" ] && echo "-lSDL"
	if [ "$1" == "--cflags" ]; then
		for d in $INCLUDE_PATH; do
			[ -d "$d/SDL" ] && echo -n "-I$d/SDL "
		done
		echo ""
	fi
}

# simply own xml2-config
function own_xml2_config() {
	[ "$1" == "--libs" ] && echo "-lz -lxml2"
	if [ "$1" == "--cflags" ]; then
		for d in $INCLUDE_PATH; do
			[ -d "$d/libxml2" ] && echo -n "-I$d/libxml2 "
		done
		echo ""
	fi
}

# get somehow a version-string (like "0.57_beta4_r1090")
function get_olx_version() {
	VERSION=""
	if [ -e VERSION ]; then
		VERSION=$(cat VERSION)
	else
		VERSION=$(grep LX_VERSION include/LieroX.h | \
			grep define | grep -o -e "\".*\"" | cut -d "\"" -f 2)
	fi
	if [ -e .svn/entries ]; then	
		VERSION="${VERSION}_r$(head -n 4 .svn/entries | tail -n 1)"
	fi
	echo "$VERSION"
}

