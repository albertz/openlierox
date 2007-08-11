#!/bin/bash

if [ "$1" == "" ]
then
	echo "please say me a version-string (like 0.57_beta3)"
	exit -1
fi

VERSION=$1 ./create_archives.sh
