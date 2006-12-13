#!/bin/bash

DEST=albertzeyer@shell.sourceforge.net:/home/groups/o/op/openlierox/htdocs/

rsync -avP --exclude .svn additions/* $DEST
rsync -avP --exclude .svn tarball/* $DEST
rsync -avP --exclude .svn ebuild/* $DEST
rsync -avP --exclude .svn deb/* $DEST
