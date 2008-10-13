#!/bin/sh

get_backtrace() {
    local exe=$1
    local core=$2

	echo "ERROR detected, printing core-info ..."

    gdb ${exe} \
        --core ${core} \
        --batch \
        --quiet \
		-ex "set width 0" \
		-ex "set height 0" \
        -ex "thread apply all bt full" \
        -ex "quit"

	echo "HINT: Please send the above output to openlierox@az2000.de."
}

cd share/gamedir
ulimit -c unlimited		# activate core-files
rm core* 2>/dev/null	# remove old core-files

bin=bin/openlierox
[ -x ../../$bin ] || bin=build/Xcode/build/Debug/OpenLieroX.app/Contents/MacOS/OpenLieroX
[ -x ../../$bin ] || bin=build/Xcode/build/Release/OpenLieroX.app/Contents/MacOS/OpenLieroX
../../$bin "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9"

# game was exited, check for core-files (if crashed)
[ -e core* ] && get_backtrace ../../bin/openlierox core*
rm core* 2>/dev/null
