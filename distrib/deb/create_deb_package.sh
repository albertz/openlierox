#!/bin/bash

cd ../.. # go into root dir
dpkg-buildpackage -rfakeroot -sa \
	-I.svn -Idistrib -ICMakeFiles -Ibuild -I*stamp -IMakefile \
	-ICMakeCache.txt -Icmake* -Itools \
	-i\
"(?:^|/).*~$|"\
"(?:^|/)\.#.*$|"\
"(?:^|/)\..*\.swp$|"\
"(?:^|/),,.*(?:$|/.*$)|"\
"(?:^|/)(?:DEADJOE|\.cvsignore|\.arch-inventory|\.bzrignore)$|"\
"(?:^|/)(?:CVS|RCS|\.deps|\{arch\}|\.arch-ids|\.svn|_darcs|\.git|\.bzr(?:\.backup|tags)?)(?:$|/.*$)$|"\
"(?:^|/)(?:CMakeFiles|build|distrib|tools|Makefile|CMakeCache.txt|cmake)|"\
"(?:^|/)bin/.*$"
