#!/bin/bash

if [ ! -d include ] || [ ! -d src ]; then
	./download.sh || exit -1

	echo ">>> installing..."
	mv hawknl1.68/src . && \
	mkdir -p include/nl && \
	mv hawknl1.68/include/* include/nl && \
	rm -r hawknl1.68 && \
	exit 0

	echo ">>> problems detected, cleaning up..."
	rm -r hawknl1.68 src include
	exit -1
fi
