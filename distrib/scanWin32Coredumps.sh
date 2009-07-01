#!/bin/bash

cd ..

for dump in coredumps/*.dmp; do

	rev=`Win32MinidumpAnalyzer.exe "$dump" 0 | grep -o '_r[0-9]*' | sed 's/_r//'`
	echo Coredump $dump rev $rev > "$dump.txt"
	Win32MinidumpAnalyzer.exe "$dump" 1 >> "$dump.txt"
	Win32MinidumpAnalyzer.exe "$dump" 2 >> "$dump.txt"
	if [ \! -e "debuginfo/r$rev" ] ; then
		echo No debuginfo for rev $rev >> "$dump.txt"
		continue
	fi
	
	cdb -y "debuginfo/r$rev" -i "debuginfo/r$rev" -z "$dump" \
		-srcpath "debuginfo/r$rev/sources" -lines \
		-c ".kframes 100; .ecxr; kp; ~*kp; q" >> "$dump.txt"
done
