#!/bin/bash

# The idea in bin and data packages is that data package is big and unchanging,
# when bin package may be changed frequently, so bin package version 0.57.beta4.r1148
# depends on data package 0.57.beta4 (without revision number),
# and you can update bin 0.57.beta4.r1148 with 0.57.beta4.r1149 without updating data package.
# When we will increment number in ../../VERSION file new data package will be built.

if [ $USER != root ] ; then echo This program should be run from root account - type \"sudo $0\" ; exit 1 ; fi
echo Creating openlierox.deb - Make sure your bin/openlierox is a release version
echo If you built openlierox not on i386 then edit "bin/control" and specify correct Architecture: field.
# Copying data
if [ -e pkg ] ; then rm -rf pkg ; fi
mkdir -p pkg/usr/games
cp -r ../../bin/openlierox pkg/usr/games
strip pkg/usr/games/openlierox
cp -r bin/share pkg/usr
cp ../../share/OpenLieroX.svg pkg/usr/share/pixmaps
find ../../doc -maxdepth 1 -type "f"  -exec cp {} pkg/usr/share/doc/openlierox \;
rm pkg/usr/share/doc/openlierox/ChangeLog
gzip -c -9 ../../doc/ChangeLog > pkg/usr/share/doc/openlierox/changelog.gz
find pkg/ -name ".svn" -exec rm -rf {} \; >/dev/null 2>&1
find pkg/ -name "*~" -exec rm -rf {} \; >/dev/null 2>&1

find pkg/ -type "f" -exec md5sum {} \; | sed 's@ pkg/@ @' > md5sums

# Copying / creating control files
mkdir pkg/DEBIAN > /dev/null 2>&1
mv md5sums pkg/DEBIAN
find bin -maxdepth 1 -type "f" -exec cp {} pkg/DEBIAN \;
find pkg/DEBIAN -name ".svn" -exec rm -rf {} \; >/dev/null 2>&1
find pkg/DEBIAN -name "*~" -exec rm -rf {} \; >/dev/null 2>&1

chgrp -R root pkg/
chown -R root pkg/

Version=`cat ../../VERSION | sed 's/_/./g'`
Revision=`svn info ../../ | grep '^Revision:' | sed 's/Revision: //'`
Size=`du -k -s pkg | grep -o '[0-9]*'`
cat bin/control | \
sed "s/^Version:.*/Version: $Version.r$Revision/" | \
sed "s/openlierox-data ([^)]*)/openlierox-data (>= $Version)/" | \
sed "s/^Installed-Size:.*/Installed-Size: $Size/" \
> pkg/DEBIAN/control

dpkg -b pkg openlierox_$Version.r$Revision.deb

rm -rf pkg
