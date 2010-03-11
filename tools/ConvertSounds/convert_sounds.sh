#!/bin/bash

if [ "$1" == "" ]; then
	echo "usage: $0 dirname"
	echo "That will search dirname and all subdirs and convert all mp3/wav to ogg."
	exit 1
fi

lenofstr() {
	# the most stupid way I can imagine of doing this
	expr "$1" : ".*"
}

convertfile() {
	f="$1"
	o="${f:0:$(expr $(lenofstr "$f") - 4)}.ogg"
	echo -n "$(basename "$f") ..."
	if ffmpeg -i "$f" -acodec vorbis -ac 2 "$o" >/dev/null 2>/dev/null; then
		echo " ok"
		rm "$f"
	else
		echo " failed"
	fi
}

for f in "$1"/*; do
	if [ -d "$f" ]; then
		"$0" "$f"
	elif echo "$f" | egrep '.*\.([wW][aA][vV]|[mM][pP]3)$' >/dev/null; then
		convertfile "$f"
	fi
done

