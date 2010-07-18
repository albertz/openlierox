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

cd "$(dirname "$0")"
cd share/gamedir
ulimit -c unlimited		# activate core-files
rm core* 2>/dev/null	# remove old core-files

bin="/dev/null"
[ -x ../../$bin ] || bin=build/Xcode/build/Debug/OpenLieroX.app/Contents/MacOS/OpenLieroX
[ -x ../../$bin ] || bin=build/Xcode/build/Release/OpenLieroX.app/Contents/MacOS/OpenLieroX
[ -x ../../$bin ] || bin=bin/openlierox
../../$bin "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9"

# game was exited, check for core-files (if crashed)
[ -e core* ] && get_backtrace ../../bin/openlierox core*
mv core* ../.. 2>/dev/null

if [ -e /proc/sys/kernel/core_pattern ] && [ "$(cat /proc/sys/kernel/core_pattern)" != "" ]; then
	corefile="$(sh -c "echo $(cat /proc/sys/kernel/core_pattern | sed -e "s/%e/openlierox/g" -e "s/%p/*/g" -e "s/%u/$(id -u)/g" -e "s/%t/*/g")")"
	if [ -e "$corefile" ]; then
		echo "found corefile $corefile"
		get_backtrace ../../bin/openlierox "$corefile"
		mv "$corefile" ../..
	fi
fi
