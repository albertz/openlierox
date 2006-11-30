#!/bin/bash

DEST=albertzeyer@shell.sourceforge.net:/home/groups/o/op/openlierox/

rsync -avP additions/* $DEST
rsync -avP tarball/* $DEST
rsync -avP ebuild/* $DEST
rsync -avP deb/* $DEST
