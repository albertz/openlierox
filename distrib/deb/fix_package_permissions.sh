#!/bin/bash

if [ -z "$1" ] ; then echo Specify .deb package as parameter ; exit 1 ; fi

if [ -e pkg ] ; then rm -rf pkg ; fi

echo Repacking the package "$1" with correct permissions

if [ -e control.tar.gz ] ; then rm control.tar.gz ; fi
ar x "$1" control.tar.gz
mkdir pkg
tar -x -C pkg -f control.tar.gz
rm control.tar.gz
tar -c -C pkg ./ --owner root --group root | gzip -9 > control.tar.gz
ar r "$1" control.tar.gz
rm control.tar.gz
rm -rf pkg

if [ -e data.tar.gz ] ; then rm data.tar.gz ; fi
ar x "$1" data.tar.gz
mkdir pkg
tar -x -C pkg -f data.tar.gz
rm data.tar.gz
tar -c -C pkg ./ --owner root --group root | gzip -9 > data.tar.gz
ar r "$1" data.tar.gz
rm data.tar.gz
rm -rf pkg
