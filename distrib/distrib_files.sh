#!/bin/bash

SERVER="albertzeyer@shell.sourceforge.net"
REMOTEDIR="/home/groups/o/op/openlierox/htdocs/"
DEST="$SERVER:$REMOTEDIR"
SYNC_CMD="rsync -avP --exclude .svn"

[ -e tmpbuild/* ] 2>/dev/null && {
	f=$(echo tmpbuild/*)
	ssh $SERVER "cd $REMOTEDIR; [ -e tmpbuild/* ] 2>/dev/null && \
		[ tmpbuild/* != $f ] && \
		echo 'single tmpbuild-file, renaming' && \
		mv tmpbuild/* $f"
}

# first, upload all new files
$SYNC_CMD tmpbuild $DEST
$SYNC_CMD ebuild $DEST

# now, update the site itself
$SYNC_CMD --delete-after web/* $DEST

# and lastly, clean up old files
$SYNC_CMD --delete-after ebuild $DEST
$SYNC_CMD --delete-after tmpbuild $DEST
