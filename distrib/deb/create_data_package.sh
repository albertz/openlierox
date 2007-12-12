#!/bin/bash

echo Creating openlierox-data.deb
if [ -e pkg ] ; then rm -rf pkg ; fi
mkdir -p pkg/usr/share
if [ -e ../../share/gamedir/front.bmp ] ; then rm ../../share/gamedir/front.bmp ; fi
if [ -e ../../share/gamedir/back.bmp ] ; then rm ../../share/gamedir/back.bmp ; fi

cp -r ../../share/gamedir pkg/usr/share/OpenLieroX
find pkg/ -name ".svn" -exec rm -rf {} \; >/dev/null 2>&1

find pkg/ -type "f" -exec md5sum {} \; | sed 's@ pkg/@ @' > md5sums

mkdir pkg/DEBIAN
mv md5sums pkg/DEBIAN
find data -maxdepth 1 -type "f" -exec cp {} pkg/DEBIAN \;

Version=`cat ../../VERSION | sed 's/_/./g'`
Size=`du -k -s pkg | grep -o '[0-9]*'`
cat data/control | \
sed "s/^Version:.*/Version: $Version/" | \
sed "s/^Installed-Size:.*/Installed-Size: $Size/" \
> pkg/DEBIAN/control

dpkg -b pkg openlierox-data_$Version.deb

# rm -rf pkg
