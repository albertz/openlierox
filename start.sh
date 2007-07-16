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

	echo "HINT: please send the file '$core' along with the above output to openlierox@az2000.de"
}

cd share/gamedir
ulimit -c unlimited
rm core* 2>/dev/null
../../bin/openlierox
[ -e core* ] && get_backtrace ../../bin/openlierox core* && mv core* ../../
