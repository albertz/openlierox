#!/bin/bash

if [ "$1" == "" ]
then
	echo "please say me a version-string"
	exit -1
fi

VERSION=$1 ./create_archives.sh
