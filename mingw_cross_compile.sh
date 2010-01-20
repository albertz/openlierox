#!/bin/sh

env CC="i586-mingw32msvc-cc" \
	CFLAGS="-I`pwd`/build/mingw/include" \
	CXX="i586-mingw32msvc-c++" \
	CXXFLAGS="-I`pwd`/build/mingw/include" \
	cmake -D MIKNGW_CROSS_COMPILE=1 -D PCH=1 -D BREAKPAD=0 -D CMAKE_EXE_LINKER_FLAGS="-L`pwd`/build/mingw/lib" .
