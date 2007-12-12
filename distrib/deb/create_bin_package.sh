#!/bin/bash

echo Creating openlierox.deb - Make sure your bin/openlierox is a release version
echo If you built openlierox not on i386 then edit "bin/control" and specify correct Architecture: field.
if [ -e pkg ] ; then rm -rf pkg ; fi
mkdir -p pkg/usr/games
cp -r ../../bin/openlierox pkg/usr/games
strip pkg/usr/games/openlierox
cp -r bin/share pkg/usr
cp ../../share/OpenLieroX.svg pkg/usr/share/pixmaps
find ../../doc -maxdepth 1 -type "f"  -exec cp {} pkg/usr/share/doc/openlierox \;
find pkg/ -name ".svn" -exec rm -rf {} \; >/dev/null 2>&1

find pkg/ -type "f" -exec md5sum {} \; | sed 's@ pkg/@ @' > md5sums

mkdir pkg/DEBIAN > /dev/null 2>&1
mv md5sums pkg/DEBIAN
find bin -maxdepth 1 -type "f" -exec cp {} pkg/DEBIAN \;

Version=`cat ../../VERSION | sed 's/_/./g'`
Size=`du -k -s pkg | grep -o '[0-9]*'`
cat bin/control | \
sed "s/^Version:.*/Version: $Version/" | \
sed "s/^Installed-Size:.*/Installed-Size: $Size/" \
> pkg/DEBIAN/control

dpkg -b pkg openlierox_$Version.deb

# rm -rf pkg
