#!/bin/bash

echo Creating openlierox-data.deb
if [ -e pkg ] ; then rm -rf pkg ; fi
# svn export is faster than just copy, 'cause it won't copy .svn/ dirs
mkdir -p pkg/usr
svn export data/share pkg/usr/share
mkdir -p pkg/usr/share/games
gzip -c -9 ../../doc/ChangeLog > pkg/usr/share/doc/openlierox-data/changelog.gz

svn export ../../share/gamedir pkg/usr/share/games/OpenLieroX
#find pkg/ -name ".svn" -exec rm -rf {} \; >/dev/null 2>&1
#find pkg/ -name "*~" -exec rm -rf {} \; >/dev/null 2>&1

find pkg/ -type "f" -exec md5sum {} \; | sed 's@ pkg/@ @' > md5sums

mkdir pkg/DEBIAN
mv md5sums pkg/DEBIAN
find data -maxdepth 1 -type "f" -exec cp {} pkg/DEBIAN \;

# You should be member of "root" group to execute this, dunno how to fix that - dh_fixperms does exactly the same
# This is done later by ./fix_package_permissions.sh script
#chown -R root:root pkg/

Version=`cat ../../VERSION | sed 's/_/./g'`
Size=`du -k -s pkg | grep -o '[0-9]*'`
cat data/control | \
sed "s/^Version:.*/Version: $Version/" | \
sed "s/^Installed-Size:.*/Installed-Size: $Size/" \
> pkg/DEBIAN/control

dpkg -b pkg openlierox-data_$Version.deb

rm -rf pkg

./fix_package_permissions.sh "openlierox-data_$Version.deb"
