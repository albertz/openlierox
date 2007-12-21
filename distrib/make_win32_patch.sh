#!/bin/bash

# Usage: ./make_win32_patch.sh revision_number

REV=1000 # Beta3 revision number (+-20 actually, I don't know exact revision number)

if [ -n "$1" ] ; then REV="$1" ; fi

if [ -n "$2" ] ; then
	# Check revision of file specified in commandline
	FILEREV=`svn info "$2" | grep '^Last Changed Rev:' | sed 's/^[^:]*: *//'`
	if [ -n "$FILEREV" ] ; then
		if expr $REV '<' $FILEREV > /dev/null ; then
			FILEPATH=`echo $2 | sed "s@^$3/@@"`
			echo r$REV '<' r$FILEREV: "$FILEPATH": from "$3"
			CWD=`pwd`
			cd "$3"
			cp --parents "$FILEPATH" "$4"
			cd "$CWD"
		fi
	fi
else
	# Run "find" on game dirs and call this script for each found file
	CURREV=`svn info ./ | grep '^Revision:' | sed 's/^[^:]*: *//'`
	echo Creating Win32 patch from r$REV to r$CURREV
	OUTDIRNAME=win32_patch_r$CURREV
	OUTDIR="`pwd`/$OUTDIRNAME"
	if [ -e "$OUTDIR" ] ; then rm -r "$OUTDIR" ; fi
	mkdir "$OUTDIR"

	INDIR=win32
	find "$INDIR" -type f -wholename "*/.svn/*" -prune -o -type f -exec "$0" "$REV" "{}" "$INDIR" "$OUTDIR" \;

	INDIR=../share/gamedir
	find "$INDIR" -type f -wholename "*/.svn/*" -prune -o -type f -exec "$0" "$REV" "{}" "$INDIR" "$OUTDIR" \;

	cd "$OUTDIR"
	zip -r "../$OUTDIRNAME.zip" *
	cd ..
	rm -r "$OUTDIR"
fi
