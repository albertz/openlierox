#!/usr/bin/python -u
# Dedicated Control handler script for OpenLieroX
# (http://openlierox.sourceforge.net)
# Upon editing - keep good names and excessive comments
# Written for easy editing by newcomers to python or programming in general
# OH AND I MEAN THIS ABOUT EXCESSIVE COMMENTS T.T
# As usual, try to hardcode as little as possible, it becomes magical numbers for everyone except you(!!)

# Needed for sleeping/pausing execution
import time
# Needed for directory access
import os
import sys
import threading

import dedicated_config  # Per-host config like admin password 
cfg = dedicated_config # shortcut

curdir = os.getcwd()
curdir = os.path.join(curdir,"scripts")
presetDir = os.path.join(curdir,"presets")

## Global vars ##

# Preset stuffies
availiblePresets = list()
maxPresets = 0
curPreset = 0


#worms = {} # Dictionary of all online worms - contains only worm name currently
worms = {} # List of all worms on the server
# Bots don't need to be itterated with the other ones.
bots = {}  # Dictionary of all possible bots
#admins = {} # Dictionary of all admin worms

# TODO: Expand this class, use it.
class Worm():
	def __init__(self):
		self.Name = ""
		self.Continent = ""
		self.CountryShortcut = ""
		self.Country = ""
		self.Ip = ""
		self.iID = -1
		self.isAdmin = False

# Game states
GAME_QUIT = 0
GAME_LOBBY = 1
GAME_WEAPONS = 2
GAME_PLAYING = 3

#Log Severity
LOG_INFO = 0
LOG_WARN = 1
LOG_ERROR = 2
LOG_CRITICAL = 3 # For things that you REALLY need to exit for.

gameState = GAME_QUIT

## Receiving functions ##

# Non-blocking IO class
# You should use getSignal() function to get another signal, not read from stdin.
# getSignal() will return empty string if there is nothing to read.
# use pushSignal() if you want to have some signal later read by getSignal()


# This is just technical mumbojumbo, not really interresting for the common user (you)
# You can ignore this part: What's important is that this part catches EVERYTHING sent by OLX, 
# puts it in it's buffer, and delivers it to anyone who asks by getResponse().

bufferedSignalsLock = threading.Lock()
bufferedSignals = []

# The thread to read from stdin
class GetStdinThread(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
		self.start()

	def run(self):
		global bufferedSignals, bufferedSignalsLock
		try:
			emptyLines = 0 # Hack to stdin being silently closed 
			while True: # Killed by sys.exit(), that's hack but I'm too lazy to do correct thread exit
				line = sys.stdin.readline()
				if line.strip() != "":
					emptyLines = 0
					bufferedSignalsLock.acquire()
					bufferedSignals.append(line.strip())
					bufferedSignalsLock.release()
				else:
					emptyLines += 1
				if sys.stdin.closed or emptyLines > 100:
					raise Exception, "stdin closed"
		except Exception:
			messageLog("Broken Pipe -- exiting",LOG_CRITICAL)
			# OLX terminated, stdin = broken pipe, we should terminate also
			sys.exit()

stdinInputThread = GetStdinThread()

# Use this function to get signals, don't access bufferedSignals array directly
def getSignal():
	global bufferedSignals, bufferedSignalsLock
	ret = ""
	bufferedSignalsLock.acquire()
	if len(bufferedSignals) > 0:
		ret = bufferedSignals[0]
		bufferedSignals = bufferedSignals[1:]
	bufferedSignalsLock.release()
	#if ret != "": msg("Got signal \"%s\"" % ret)
	return ret

# Use this function to push signal into bufferedSignals array, don't access bufferedSignals array directly
def pushSignal(sig):
	global bufferedSignals, bufferedSignalsLock
	bufferedSignalsLock.acquire()
	bufferedSignals.append(sig)
	bufferedSignalsLock.release()

# Wait until specific signal, returns params of that signal (strips signal name), saves all extra signals.
def getResponse(cmd):
	ret = ""
	extraSignals = []
	resp = getSignal()
	while ret == "":
		if resp.find(cmd+" ") == 0:
			ret = resp[len(cmd)+1:] # Strip cmd string and push the rest to return-value
		else:
			if resp != "":
				extraSignals.append(resp)
			else:
				time.sleep(0.01)
		if ret == "":
			resp = getSignal()
	for s in extraSignals:
		pushSignal(s)
	return ret

# This function is indended to parse something like:
# "newworm 1 Player1" -> we call "getwormlist" and getResponseList("wormlistinfo")
# "gamestarted" is arrived when getResponseList("wormlistinfo") called, then worm list info arrives:
# "wormlistinfo 0 [CPU] Kamikazee!"
# "wormlistinfo 1 Player1"
# "endlist"
# It will return array [ "0 [CPU] Kamikazee!", "1 Player1" ] and push back "gamestarted" signal so we will parse it next.
def getResponseList(cmd):
	ret = []
	extraSignals = []
	resp = getSignal()
	while resp != "endlist":
		if resp.find(cmd+" ") == 0:
			ret.append( resp[len(cmd)+1:] ) # Strip cmd string and push the rest to return-value
		else:
			if resp != "":
				extraSignals.append(resp)
			else:
				time.sleep(0.01)
		resp = getSignal()
	for s in extraSignals:
		pushSignal(s)
	return ret

## Sending functions ##

# Set a server variable
def setvar(what, data):
	print "setvar %s %s" % (str(what), str(data))
	
# Use this to make the server quit
def Quit():
	print "quit"

# Use this to force the server into lobby - it will kick all connected worms and restart the server
# TODO: Why do we send this with startlobby? Server doesn't catch it, + it's bad style.
# Use addWorm() instead
def startLobby(localWorm = "[CPU] Kamikazee!"):
	print "startlobby " + localWorm

# Force the server into starting the game (weapon selections screen)
def startGame():
	print "startgame"
	
# Use this to force the server into lobby - it will abort current game but won't kick connected worms
def gotoLobby():
	print "gotolobby"

# Use this to refresh lobby information - like level/mod et.c..
def sendLobbyUpdate():
	print "sendlobbyupdate"

# Not implemented yet in OLX
def addBot(name):
	print "addbot " + name

# Suicides all local bots
def killBots():
	print "killbots"

# Both kick and ban uses the ingame identification code
# for kicking/banning.
def kickWorm(iID, reason = ""):
	if reason != "":
		print "kickworm " + str(iID) + " " + reason
	else:
		print "kickworm " + str(iID)

def banWorm(iID, reason = ""):
	if reason != "":
		print "banworm " + str(iID) + " " + reason
	else:
		print "banworm " + str(iID)

def muteWorm(iID):
	print "muteworm " + str(iID)

def setWormTeam(iID, team):
	print "setwormteam " + str(iID) + " " + str(team)

# Use this to get ID number + name of all worms - updates worms list.
# For some reason it entered resp for me once, but it didn't contain any data (or it had strange data) and int() returned a ValueError.
# Please report when this happens and how it is caused. I tried to recreate but to no avail.
# It starts happening after a couple of hours running, and creates ValueErrors on every call, to what i can establish.

# TODO: We should not use this. We should rely on newworm <id> and wormleft <id>. We should only request names after that.
def getWormList():
	print "getwormlist"
	resp = getResponseList("wormlistinfo")
	global worms
	worms = {}
	for r in resp:
		try:
			iID = int(r[:r.find(" ")])
			name = r[r.find(" ")+1:]
			#worms[iID] = name
			
			try:
				worm = worms[iID]
			except KeyError: #Worm doesn't exist.
				worm = Worm()
			worm.Name = name
			worm.iID = iID
			worms[iID] = worm
		except ValueError:
			mesg = "ValueError in getWormList. "
			mesg += "r = " + r + " resp = " + str(resp)
			messageLog(mesg,LOG_ERROR)
	


# Use this to get the list of all possible bots.
def getComputerWormList():
	print "getcomputerwormlist"
	resp = getResponseList("computerwormlistinfo")
	global bots
	bots = {}
	for r in resp:
		iID = int(r[:r.find(" ")])
		name = r[r.find(" ")+1:]
		bots[iID] = name

def getWormIP(iID):
	print "getwormip"
	return getResponse("wormip %i" % iID)

def getWormLocationInfo(iID):
	print "getwormlocationinfo"
	return getResponse("wormlocationinfo %i" % iID)

# Use this to write to stdout (standard output)
def msg(string):
	print "msg " + string

# Send a chat message
def chatMsg(string):
	print "chatmsg " + string

## High-level processing ##

def checkGameState(sig):
	global gameState
	if sig == "quit" or sig == "errorstartlobby":
		gameState = GAME_QUIT
	if sig == "backtolobby" or sig == "errorstartgame" or sig == "lobbystarted":
		gameState = GAME_LOBBY
	if sig == "weaponselections":
		gameState = GAME_WEAPONS
	if sig == "gamestarted":
		gameState = GAME_PLAYING

def updateWorms(sig):
	if sig.find("newworm ") == 0 or sig.find("wormleft ") == 0:
		getWormList()

# Admin interface
def updateAdminStuff(sig):
	if sig.find("wormleft ") == 0:
		wormidx = int( sig.split(" ")[1] )
		try:
			if worms[wormidx].isAdmin:
				worms[wormidx].isAdmin = False
				messageLog(("Worm %i (%s) removed from admins" % (wormidx,worms[wormidx].Name)),LOG_INFO)
		except KeyError:
			messageLog("AdminRemove: Our local copy of wormses doesn't match the real list.",LOG_ERROR)
	if sig.find("privatemessage ") == 0:
		wormidx = int( sig.split(" ")[1] )
		if sig.split(" ")[3] == cfg.ADMIN_PASSWORD:
			#admins.append(wormidx)
			try:
				worms[wormidx].isAdmin = True
				messageLog(("Worm %i (%s) added to admins" % (wormidx,worms[wormidx].Name)),LOG_INFO)
				chatMsg("%s will banhaxkick everyone now! Type //help for commands info" % worms[wormidx].Name)
			except KeyError:
				messageLog("AdminAdd: Our local copy of wormses doesn't match the real list.",LOG_ERROR)
	try: # Do not check on msg size or anything
		if sig.find("chatmessage ") == 0:
			wormidx = int( sig.split(" ")[1] )
			msg( "Chat msg from worm %i: %s" % (wormidx, " ".join(sig.split(" ")[2:])) )
			if worms[wormidx].isAdmin:
				cmd = sig.split(" ")[2]
				if cmd == "//help":
					chatMsg("Admin help:")
					chatMsg("//kick wormID [reason]")
					chatMsg("//ban wormID [reason]")
					chatMsg("//mute wormID")
					chatMsg("//mod modname")
					chatMsg("//map mapname")
					chatMsg("//lt loadingTime")
					chatMsg("//start - start game now")
					chatMsg("//stop - go to lobby")
					chatMsg("//setvar varname value")
					
				elif cmd == "//kick":
					if len(sig.split(" ")) > 4: # Given some reason
						kickWorm( int( sig.split(" ")[3] ), " ".join(sig.split(" ")[4:]) )
					else:
						kickWorm( int( sig.split(" ")[3] ) )
				elif cmd == "//ban":
					if len(sig.split(" ")) > 4: # Given some reason
						banWorm( int( sig.split(" ")[3] ), " ".join(sig.split(" ")[4:]) )
					else:
						banWorm( int( sig.split(" ")[3] ) )
				elif cmd == "//mute":
					muteWorm( int( sig.split(" ")[3] ) )
				elif cmd == "//mod":
					setvar("GameServer.GameInfo.sModDir", " ".join(sig.split(" ")[3:])) # In case mod name contains spaces
					setvar("GameServer.GameInfo.sModName", " ".join(sig.split(" ")[3:]))
					sendLobbyUpdate()
				elif cmd == "//map":
					setvar("GameServer.GameInfo.sMapFile", " ".join(sig.split(" ")[3:]) + ".lxl") # In case map name contains spaces
					setvar("GameServer.GameInfo.sMapName", " ".join(sig.split(" ")[3:]))
					sendLobbyUpdate()
				elif cmd == "//lt":
					setvar("GameServer.GameInfo.iLoadingTimes", sig.split(" ")[3])
					sendLobbyUpdate()
				elif cmd == "//start":
					startGame()
				elif cmd == "//stop":
					gotoLobby()
				elif cmd == "//setvar":
					setvar(sig.split(" ")[3], " ".sig.split(" ")[4:]) # In case value contains spaces
	except Exception:
		chatMsg("Invalid admin command")
	except KeyError:
		errorLog("AdminCommands: Our local copy of wormses doesn't match the real list.",LOG_ERROR)
		
# Updates all global vars
# But it SHOULD handle all signals coming in, and route to the right functions. 
# (Just like when we parse networking in OLX, it's a good aproach imo)
def signalHandler(sig):
	global gameState
	updateAdminStuff(sig)
	checkGameState(sig)
	updateWorms(sig)

## Preset loading functions ##

def initPresets():
	global availiblePresets,maxPresets,presetDir
	
	# Reset - incase we get called a second time
	del availiblePresets[:]
	maxPresets = 0

	for f in os.listdir(presetDir):
		if os.path.isdir(f):
			messageLog(("initPresets: Ignoring \"%s\" - It's a directory" % f),LOG_INFO)
			continue

		availiblePresets.append(f)

		maxPresets +=1

	if (maxPresets == 0):
		messageLog("There are no presets availible - nothing to do. Exiting.",LOG_CRITICAL)
		exit()

# initPresets must be called before this - or it will crash
# TODO: Try to make something nicer for the user which doesn't read this
def selectNextPreset():
	global curPreset,maxPresets,availiblePresets

	msg("Preset " + availiblePresets[curPreset])
	chatMsg("Preset " + availiblePresets[curPreset])

	sFile = os.path.join(presetDir,availiblePresets[curPreset])
	try:
		fPreset = file(sFile,"r")
		for line in fPreset.readlines():
			line = line.strip()
			line = line.replace('"','')
			print line
			fPreset.close()
	except IOError:
		# File does not exist, perhaps it was removed.
		messageLog(("Unable to load %s, forcing rehash of all presets" % sFile),LOG_WARN)
		initPresets()
		# Try to step back so we don't force people to replay old rotations.
		while (curPreset >= maxPresets):
			curPreset -= 1
		# Re-call ourselves so that we do update the preset.
		fPreset.close() # Just incase
		selectNextPreset()
		return # So that we don't double the messages

	# curPreset + 1 to say "1 out of 2" "2 out of 2" instead of
	# "0 out of 2" "1 out of 2"
	msg("Current preset is: %d out of max %d" % (curPreset+1,maxPresets))

	curPreset += 1
	if curPreset >= maxPresets:
		curPreset = 0

	sendLobbyUpdate()

def waitLobbyStarted():
	while True:
		sig = getSignal()
		signalHandler(sig)
		if sig == "lobbystarted":
			return
		time.sleep(1)


def messageLog(message,severity):
	outline = time.strftime("%Y-%m-%d %H:%M:%S")
	# Don't clutter the strftime call
	outline += " -- "
	if severity == LOG_INFO:
		outline += "INFO"
	elif severity == LOG_WARN:
		outline += "WARN"
	elif severity == LOG_ERROR:
		outline += "ERROR"
	elif severity == LOG_CRITICAL:
		outline += "CRITICAL"
	outline += " -- "
	outline += message
	try:
		f = open(cfg.LOG_FILE,"a")
		f.write((outline + "\n"))
	except IOError:
		msg("ERROR: Unable to open logfile.")
	finally:
		f.close()
	try:
		msg(outline)
	except:
		return #This happen when we've got a broken pipe, and we should exit cleanly.