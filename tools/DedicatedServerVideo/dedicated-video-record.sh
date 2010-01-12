#!/bin/sh


CURDATE=`date "+%Y-%m-%d_%H:%M"`

jackd -d dummy &
JOBS=$!
sleep 1
#Xvfb somehow catches Ctrl-C bypassing our trap (reads TTY perhaps?), so run it in different session and kill with killall
setsid Xvfb :11 -screen 0 640x480x16 &
XVFB=$!
JOBS="$JOBS $XVFB"
sleep 1
env DISPLAY=:11.0 SDL_AUDIODRIVER=dsp jacklaunch openlierox \
-exec 'setVar GameOptions.Advanced.MaxFPS 10' \
-exec 'setVar GameOptions.Game.LastSelectedPlayer "[CPU] Dummi"' \
-exec 'setVar GameOptions.Audio.Enabled 1' \
-exec 'wait game chatMsg /spectate ; chatMsg /suicide ; setViewport actioncam' \
-connect 127.0.0.1 &
OLX=$!
sleep 5
# Warning: Debian includes recordmydesktop with no Jack support, you'll have to compile it yourself
# --on-the-fly-encoding eats FPS
env DISPLAY=:11.0 recordmydesktop -o video-$$.ogv --no-cursor --fps 10 --v_quality 30 --s_quality 3 \
	--use-jack `jack_lsp | grep openlierox | head -n 1` &
RECORD=$!

#TODO: youtube upload, use scripts at http://code.google.com/p/gdata-python-client/downloads/list
# and http://code.google.com/apis/youtube/1.0/developers_guide_python.html#DirectUpload

UPLOADCMD="python YoutubeUpload.py OpenLieroXVideoBot01 UploadVideo357236 video-$$.ogv $CURDATE"
# Send INT signal or Ctrl-C when you done
trap "kill -INT $RECORD ; kill -INT $OLX ; wait $OLX $RECORD ; kill -INT $JOBS ; \
sleep 1 ; wait ; $UPLOADCMD ; rm video-$$.ogv" INT QUIT

wait
