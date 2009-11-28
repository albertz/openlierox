#!/bin/bash

dpkg-buildpackage -rfakeroot -sa -S \
	-I.svn -I.git -Idistrib -Ibuild -I*stamp -IMakefile \
	-i\
"(?:^|/).*~$|"\
"(?:^|/)\.#.*$|"\
"(?:^|/)\..*\.swp$|"\
"(?:^|/),,.*(?:$|/.*$)|"\
"(?:^|/)(?:DEADJOE|\.cvsignore|\.arch-inventory|\.bzrignore|\.gitignore)$|"\
"(?:^|/)(?:CVS|RCS|\.deps|\{arch\}|\.arch-ids|\.svn|_darcs|\.git|\.bzr(?:\.backup|tags)?)(?:$|/.*$)$|"\
"(?:^|/)(?:CMakeFiles|build|distrib|tools|Makefile|CMakeCache.txt|cmake)|"\
"(?:^|/)bin/.*$"

sudo pbuilder build ../openlierox-dedicated_`head -n 1 debian/changelog | grep -o "[(].*[)]" | sed 's/[()]//g'`.dsc

# result is: /var/cache/pbuilder/result/openlierox_$ver_$arch.deb
