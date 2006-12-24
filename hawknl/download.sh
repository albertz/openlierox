#!/bin/bash

FILE=HawkNL168src.tar.gz

FILE_EXISTS=0
if [ ! -e $FILE ]; then
	echo ">>> downloading..."
	wget http://www.sonic.net/~philf/download/$FILE && FILE_EXISTS=1
else
	FILE_EXISTS=1
fi

if [ FILE_EXISTS == 0 ]; then
	echo "ERROR: cannot get $FILE" >&2
	exit
fi

if [ ! -d hawknl1.68 ]; then
	echo ">>> extracting..."
	tar -xzf $FILE
fi
