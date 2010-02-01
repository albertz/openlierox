#!/bin/sh

# This script will create symlinks to correct places in current OLX GIT directory structure,
# so you'll be able to start it with "./start.sh -dedicated" from root dir and get video recording out of the box.
# If you'll move git dir to other place just run the script again

ln -s -f `pwd`/YoutubeUpload.py `pwd`/atom `pwd`/gdata ../../share/gamedir/
ln -s -f `pwd`/dedicated-video-record.sh ../../share/gamedir/scripts/
cd ../..
ln -s -f tools/DedicatedServerVideo/start_dedicated.sh ./
