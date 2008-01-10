#!/bin/bash

get_backtrace() {
    local exe=$1
    local core=$2

	echo "ERROR detected, printing core-info ..."

    gdb ${exe} \
        --core ${core} \
        --batch \
        --quiet \
        -ex "thread apply all bt full" \
        -ex "quit"

	echo "HINT: Please send the above output to openlierox@az2000.de."
	echo "	Perhaps we could also need the file '$core' later on,"
	echo "	so it would be nice if you can keep it for now."
}

cd share/gamedir
ulimit -c unlimited
rm core* 2>/dev/null
../../bin/openlierox "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9"
[ -e core* ] && get_backtrace ../../bin/openlierox core* && mv core* ../../
