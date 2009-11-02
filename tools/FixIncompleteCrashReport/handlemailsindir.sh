#!/bin/bash

if [ "$1" == "" ]; then
	echo "usage: $0 <dir>"
	exit 1
fi

mydir="$(dirname "$0")"

for f in "$1"/*; do
	if "$mydir/fixer.py" "$f" > "$f.tmp"; then
		mv "$f.tmp" "$f"
		echo "$(basename "$f"): ok"
	else
		rm "$f.tmp" 2>/dev/null
		echo "$(basename "$f"): error"
	fi
done

