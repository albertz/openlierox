#!/bin/bash

DEST="albertzeyer@shell.sourceforge.net:/home/groups/o/op/openlierox/htdocs/"
SYNC_CMD="rsync -avP --exclude .svn --delete-after"

$SYNC_CMD additions/* $DEST
$SYNC_CMD tarball/* $DEST
$SYNC_CMD ebuild/* $DEST
$SYNC_CMD deb/* $DEST
$SYNC_CMD web/* $DEST
