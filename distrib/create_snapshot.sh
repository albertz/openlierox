#!/bin/bash

if [ "$1" == "" ]
then
	echo "please say me a version-string"
	exit -1
fi

echo "copying tarballs ..."
cd tarball
for f in *_cur.*
do
	echo "$f -> ${f/_cur/_$1}"
	cp $f ${f/_cur/_$1}
done
cd ..
