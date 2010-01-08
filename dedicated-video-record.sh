#!/bin/sh

jackd -d dummy &
JOBS=$!
sleep 1
#Xvfb somehow catches Ctrl-C bypassing our trap (reads TTY perhaps?), so run it in different session and kill with killall
setsid Xvfb :10 -screen 0 640x480x16
#JOBS="$JOBS $!"
sleep 1
env DISPLAY=:10.0 SDL_AUDIODRIVER=dsp jacklaunch ./start.sh -connect 127.0.0.1 &
OLX=$!
sleep 5
# Warning: Debian includes recordmydesktop with no Jack support, you'll have to compile it yourself
# --on-the-fly-encoding eats FPS
rm video.ogv
env DISPLAY=:10.0 recordmydesktop -o video.ogv --no-cursor --fps 10 --v_quality 40 --s_quality 7 \
	--use-jack `jack_lsp | grep openlierox | head -n 1` &
RECORD=$!

#TODO: youtube upload, use scripts at http://code.google.com/p/gdata-python-client/downloads/list
# and http://code.google.com/apis/youtube/1.0/developers_guide_python.html#DirectUpload

# Send INT signal or Ctrl-C when you done
trap "kill -INT $RECORD ; kill -INT $OLX ; sleep 10 ; kill -INT $JOBS ; killall -INT Xvfb ; sleep 1 ; wait" INT QUIT

wait
