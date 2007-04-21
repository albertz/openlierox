#!/bin/bash

SRC="albertzeyer@shell.sourceforge.net:/home/groups/o/op/openlierox/htdocs/"
SYNC_CMD="rsync -avP --exclude .svn"

# download all missing files
$SYNC_CMD $SRC/additions .
$SYNC_CMD $SRC/tarball .
$SYNC_CMD $SRC/deb .
