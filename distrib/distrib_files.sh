#!/bin/bash

DEST="albertzeyer@shell.sourceforge.net:/home/groups/o/op/openlierox/htdocs/"
SYNC_CMD="rsync -avP --exclude .svn"

# first, upload all new files
$SYNC_CMD tmpbuild $DEST
$SYNC_CMD ebuild $DEST

# now, update the site itself
$SYNC_CMD web/* $DEST

# and lastly, clean up old files
$SYNC_CMD --delete-after ebuild $DEST
#$SYNC_CMD --delete-after tmpbuild $DEST
