#!/bin/bash

if [ ! -d include ] || [ ! -d src ]; then
	./download.sh || exit -1

	echo ">>> installing HawkNL source ..."
	mv hawknl1.68/src . && \
	mv hawknl1.68/include . && \
	rm -r hawknl1.68 && \
	exit 0

	echo ">>> problems detected, cleaning up ..."
	rm -r hawknl1.68 src include
	exit -1
fi
