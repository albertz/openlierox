#!/usr/bin/python -u
# Dedicated Control handler script for OpenLieroX
# (http://openlierox.sourceforge.net)
# Upon editing - keep good names and excessive comments
# Written for easy editing by newcomers to python or programming in general

# Needed for sleeping/pausing execution
import time
# Needed for directory access
import os
# Needed for running in non-blocking mode ? 
#(Strike that, it's for some other reason ATM, as it ain't nonblocking, but i can't remember what.)
import sys

import string


curdir = os.getcwd()
# TODO: This is if it's started from start.sh - hack
# Traceback (most recent call last):
#  File "./scripts/dedicated_control", line 14, in ?
#    print os.listdir(presetDir)
# OSError: [Errno 2] No such file or directory: '/home_l/dos/OLX_Official_Testing/openlierox/share/gamedir/presets'
curdir = os.path.join(curdir,"scripts")
presetDir = os.path.join(curdir,"presets")
#print os.listdir(presetDir)
availiblePresets = list()
maxPresets = 0
curPreset = 0


## Utility functions ##
def chomp(line):
      if line.endswith('\r\n'):
          return line[:-2]
      elif line.endswith('\r') or line.endswith('\n'):
          return line[:-1]
      else:
          return line

        
## Sending functions ##

# Set a server variable
def setvar(what, data):
    print "setvar %s %s" % (what,data)
    
# Use this to make the server quit
def Quit():
    print "quit"

# Use this to force the server into lobby
def startLobby():
    print "startlobby"

# Force the server into starting the game (weapon selections screen)
def startGame():
    print "startgame"

# Use this to refresh lobby information - like level/mod et.c..
def sendLobbyUpdate():
    print "sendlobbyupdate"

# No one quite knows what this does.. mysterious.
def addWorm():
    print "addworm"

# Both kick and ban uses the ingame identification code
# for kicking/banning.
def kickWorm(iID):
    print "kickworm " + iID

def banWorm(iID):
    print "banworm " + iID

# Use this to get ID number + name of all worms.
def getWormList():
    print "getwormlist"

# Use this to get the list of all possible bots.
def getComputerWormList():
    print "getcomputerwormlist"

# Use this to write to stdout (standard output
def msg(string):
    print "msg " + string

# Send a chat message
def chatMsg(string):
    print "chatmsg " + string

# Start the game with a countdown
def startWithCountdown():
    count = 20
    while (count >= 1):
        if (count >= 10):
            delay = 2
        else:
            delay = 1

        msg(str(count) + "...")
        chatMsg(str(count) + "...")
        time.sleep(delay)
        count -= delay

    startGame()
    # No wait_for_gameend function, cause it's bad
    # - it ignores all other signals -
    # making everything else sent useless
    # so it practically blocks all communication

    return



## Recieving functions/handlers ##

# Call this and it will wait for input
def signalHandler():
    while True:
        msg("Looping for signals.. hating you as usual")
        raw_signal = sys.stdin.readline()
        #raw_signal = raw_input()
        #msg(raw_signal)
        #msg("gameloop: " + raw_signal)
        raw_signal = raw_signal.strip()
        signal = raw_signal.split(":")
        #print signal
        #print signal[0]
        msg("%s recieved!" % signal)
        if signal[0] == "gameloopstart":
            pass
        elif signal[0] == "gameloopend":
            pass
        elif signal[0] == "backtolobby":
            # For now, add functions if you wish!
            chatMsg("back in lobby, waiting ...")
            selectNextPreset()
            startWithCountdown()
        elif signal[0] == "errorstartlobby":
            msg("Got error lobby start signal - exiting")
            sys.exit()
        elif signal[0] == "lobbystarted":
            pass
        elif signal[0] == "weaponselections":
            pass
        elif signal[0] == "gamestarted":
            pass
        # Signalises that a new worm connected
        elif signal[0] == "newworm":
            pass
        elif signal[0] == "quit":
            exit()
        elif signal[0] == "wormlistinfo":
            pass
    return


def initPresets():
    global availiblePresets,maxPresets

    # IMPORTANT: Why is presetDir availible globaly by default
    # - when the others aren't?
    for f in os.listdir(presetDir):
        if os.path.isdir(f):
            msg("Ignoring \"%s\" - It's a directory" % f)
            continue

        availiblePresets.append(f)
        
        maxPresets +=1

    if (maxPresets == 0):
        msg("There are no presets availible - nothing to do. Exiting.")
        exit()

# initPresets must be called before this - or it will crash
# TODO: Try to make something nicer for the user which doesn't read this
def selectNextPreset():
    global curPreset,maxPresets,availiblePresets

    msg("Preset " + availiblePresets[curPreset])
    chatMsg("Preset " + availiblePresets[curPreset])
    
    sFile = os.path.join(presetDir,availiblePresets[curPreset])
    fPreset = file(sFile,"r")
    for line in fPreset.readlines():
        line = line.strip()
        line = line.replace('"','')
        print line
    fPreset.close()

    # curPreset + 1 to say "1 out of 2" "2 out of 2" instead of
    # "0 out of 2" "1 out of 2"    
    msg("Current preset is: %d out of max %d" % (curPreset+1,maxPresets))
    
    curPreset += 1
    if curPreset >= maxPresets:
        curPreset = 0

    sendLobbyUpdate()



#msg("getcomputerwormlist")

#setvar("GameServer.ServerName","Dedicated multimod")

startLobby()

initPresets()

# TODO: Threading - split sending and signal handler
# That way we can start signalhandler, send startlobby,
# and wait for "lobbystarted" - then go for the 2 next rows
selectNextPreset()
startWithCountdown()
while True:
      signalHandler()

    
