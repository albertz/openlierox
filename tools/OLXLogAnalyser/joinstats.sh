#!/bin/zsh

grepJoinedMsg() {
	egrep -E "Worm joined: .* \(.*\(.*\)\)"
}

grepVersion() {
	egrep -o -E "[A-Za-z]+/[0-9.]+(_[bB]eta[0-9]+)?"
}

handlepipe() {
	grepJoinedMsg | grepVersion
}

iteratefiles() {
	for f in olx.log.*.gz; do
		echo ">> $f:"
		gunzip -c "$f" | handlepipe
	done

	for f in olx.log.*.bz2; do
		echo ">> $f:"
		bunzip2 -c "$f" | handlepipe
	done

	for f in olx.log.*[^(gz,bz2)]; do
		echo ">> $f:"
		cat "$f" | handlepipe
	done
}

rm -f joinstats.log 2>/dev/null
iteratefiles | tee joinstats.log

versions=($(sort -u joinstats.log | grep -v ">>"))
for v in $versions; do echo -n "$v : "; grep "$v" joinstats.log | wc -l; done
