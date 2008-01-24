#!/bin/bash

source ../../functions.sh

bin=Debug
[ "$1" != "" ] && bin="$1"
[ ! -d "build/$bin/OpenLieroX.app" ] && echo "build $bin does not exist" && exit -1

echo ">> fixing binaries ..."
./fix_binary.sh >/dev/null

cd ../.. # needed for get_olx_version
zipfile="OpenLieroX_$(get_olx_version)_mac.zip"
cd "build/Xcode/build/$bin"
echo ">> compressing ..."
zip -q -r -6  -D -X -y "../../$zipfile" OpenLieroX.app || { echo "error while zipping"; exit -1; }
cd ../..

echo "> file $zipfile created"
mv "$zipfile" "../../distrib/tmpbuild"
echo "> moved to distrib/tmpbuild"

