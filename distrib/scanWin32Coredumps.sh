#!/bin/bash

cd ..

for dump in coredumps/*.dmp; do

	rev=`Win32MinidumpAnalyzer.exe "$dump" 0 | grep -E -o '_r[0-9]+' | sed 's/_//'`
	if [ -z "$rev" ]; then
		rev=`Win32MinidumpAnalyzer.exe "$dump" 0 | grep -E -o '_beta[0-9]+' | sed 's/_//'`
	fi
	if [ -z "$rev" ]; then
		echo Coredump $dump rev `Win32MinidumpAnalyzer.exe "$dump" 0` - cannot determine revnumber > "$dump.txt"
		continue
	fi
	echo Coredump $dump rev $rev > "$dump.txt"
	Win32MinidumpAnalyzer.exe "$dump" 1 >> "$dump.txt"
	Win32MinidumpAnalyzer.exe "$dump" 2 >> "$dump.txt"
	if [ \! -e "debuginfo/$rev" ] ; then
		echo No debuginfo for rev $rev >> "$dump.txt"
		revnumActual=`echo $rev | grep -E -o '[0-9]+'`
		revnumMinDiff=100000000
		for f in `ls debuginfo` ; do
			revnumCheck=`echo $f | grep -E -o '[0-9]+'`
			revnumDiff=`expr $revnumCheck - $revnumActual | sed 's/-//'`
			if [ $revnumDiff -lt $revnumMinDiff ] ; then
				revnumMinDiff=$revnumDiff
				rev=$f
			fi
		done
		if [ \! -e "debuginfo/$rev" ] ; then
			echo Cannot find any debuginfo for rev $rev >> "$dump.txt"
			continue
		fi
		echo Found closest matching debuginfo - rev $rev >> "$dump.txt"
	fi
	
	cdb -y "debuginfo/$rev" -i "debuginfo/$rev" -z "$dump" \
		-srcpath "debuginfo/$rev/sources" -lines \
		-c ".kframes 100; .ecxr; kp; q" > except.txt
	fileline=`grep -o -m 1 '[^[]*@ [0-9]*' except.txt`
	rm except.txt

	if [ -z "$fileline" ]; then
		echo Cannot determine exception location in source files >> "$dump.txt"
		cdb -y "debuginfo/$rev" -i "debuginfo/$rev" -z "$dump" \
			-srcpath "debuginfo/$rev/sources" -lines \
			-c ".kframes 100; .ecxr; kp; ~*kp; q" >> "$dump.txt"
	else
		file=`echo $fileline | grep -o '[^@]*' | sed 's/ //'`
		line=`echo $fileline | grep -o '@ [0-9]*' | sed 's/@ //'`
		echo Exception at $file: $line >> "$dump.txt"
		if [ $line -gt 10 ]; then
			line=`expr $line - 10`
		else
			line=0
		fi
		cdb -y "debuginfo/$rev" -i "debuginfo/$rev" -z "$dump" \
			-srcpath "debuginfo/$rev/sources" -lines \
			-c ".kframes 100; lsp -a 14; lsf $file; ls $line; .ecxr; kp; ~*kp; q" >> "$dump.txt"
	fi
done
