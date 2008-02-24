#!/bin/bash

cd ../.. # go into root dir
dpkg-buildpackage -rfakeroot \
	-I.svn -Idistrib -ICMakeFiles -Ibuild -I*stamp -IMakefile \
	-ICMakeCache.txt -Icmake* -Itools
