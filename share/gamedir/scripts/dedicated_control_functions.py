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
class Worm:
	def __init__(self):
		self.Name = ""
		self.Continent = ""
		self.CountryShortcut = ""
		self.Country = ""
		self.Ip = ""
		self.iID = -1
		self.isAdmin = False
		
	def clear(self):
		self.__init__()

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


## Misc functions ##
def getNumWorms():
	global worms
	i = 0
	for w in worms.values():
		if w.iID != -1:
			i += 1
			
	return i
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

def updateWorms(sig):
	if sig.find("newworm ") == 0 or sig.find("wormleft ") == 0:
		getWormList()

# Admin interface
def parseAdminCommand(wormid,message):
	try: # Do not check on msg size or anything	
		cmd = message.split(" ")[0]
		if cmd.find(cfg.ADMIN_PREFIX) != -1:
			cmd = cmd.replace(cfg.ADMIN_PREFIX,"",1) #Remove the prefix
			
		# Unnecesary to split multiple times, this saves CPU.
		params = message.split(" ")[1:]
		
		if cmd == "help":
			chatMsg("Admin help:")
			chatMsg("%skick wormID [reason]" % cfg.ADMIN_PREFIX) 
			chatMsg("%sban wormID [reason]" % cfg.ADMIN_PREFIX)
			chatMsg("%smute wormID" % cfg.ADMIN_PREFIX)
			chatMsg("%smod modname" % cfg.ADMIN_PREFIX)
			chatMsg("%smap mapname" % cfg.ADMIN_PREFIX)
			chatMsg("%slt loadingTime" % cfg.ADMIN_PREFIX)
			chatMsg("%sstart - start game now" % cfg.ADMIN_PREFIX)
			chatMsg("%sstop - go to lobby" % cfg.ADMIN_PREFIX)
			chatMsg("%ssetvar varname value" % cfg.ADMIN_PREFIX)
			
		# TODO: put adminhelp in it's own function. Check for if we don't get enough params for stuff, and send it.
		elif cmd == "kick":
			if len(params) > 1: # Given some reason
				kickWorm( int( params[0] ), " ".join(params[1:]) )
			else:
				kickWorm( int( params[0] ) )
		elif cmd == "ban":
			if len(params) > 1: # Given some reason
				banWorm( int( params[0] ), " ".join(params[1:]) )
			else:
				banWorm( int( params[0] ) )
		elif cmd == "mute":
			muteWorm( int( params[0] ) )
		elif cmd == "mod":
			setvar("GameServer.GameInfo.sModDir", " ".join(params[0:])) # In case mod name contains spaces
			setvar("GameServer.GameInfo.sModName", " ".join(params[0:]))
			sendLobbyUpdate()
		elif cmd == "map":
			setvar("GameServer.GameInfo.sMapFile", " ".join(params[0:]) + ".lxl") # In case map name contains spaces
			setvar("GameServer.GameInfo.sMapName", " ".join(params[0:]))
			sendLobbyUpdate()
		elif cmd == "lt":
			setvar("GameServer.GameInfo.iLoadingTimes", params[0])
			sendLobbyUpdate()
		elif cmd == "start":
			startGame()
		elif cmd == "stop":
			gotoLobby()
		elif cmd == "setvar":
			setvar(params[0], " ".join(params[1:])) # In case value contains spaces
			
				
	except Exception:
		chatMsg("Invalid admin command")
	except KeyError:
		messageLog("AdminCommands: Our local copy of wormses doesn't match the real list.",LOG_ERROR)
		

# Parses all signals that are not 2 way (like getip info -> olx returns info)
# Returns False if there's nothing to read
def signalHandler(sig):
	global gameState
	if sig == "":
		return False #Didn't get anything
	header = sig.split(" ")[0] # This makes sure we only get the first word, no matter if there's anything after it or not.
	if header == "newworm":
		parseNewWorm(sig)
	elif header == "wormleft":
		parseWormLeft(sig)
	elif header == "privatemessage":
		parsePrivateMessage(sig)
	elif header == "chatmessage":
		parseChatMessage(sig)
	elif header == "wormdied":
		pass
	
	## Check GameState ##
	elif header == "quit" or header == "errorstartlobby":
		gameState = GAME_QUIT
	elif header == "backtolobby" or header == "errorstartgame" or header == "lobbystarted":
		gameState = GAME_LOBBY		
	elif header == "weaponselections":
		gameState = GAME_WEAPONS
	elif header == "gamestarted":
		gameState = GAME_PLAYING
		
	#if sig != "":
		#msg(sig)
	return True

def parseNewWorm(sig):
	wormID = int(sig.split(" ")[1])
	name = " ".join(sig.split(" ")[2:])
	exists = False
	try:
		worm = worms[wormID]
		exists = True
	except KeyError: #Worm doesn't exist.
		worm = Worm()
	worm.Name = name
	worm.iID = wormID
	
	if not exists:
		worms[wormID] = worm

def parseWormLeft(sig):
	wormID = int(sig.split(" ")[1])
	name = " ".join(sig.split(" ")[2:])
	
	try:
		if worms[wormID].isAdmin:
			messageLog(("Worm %i (%s) removed from admins" % (wormID,name)),LOG_INFO)
	except KeyError:
		messageLog("AdminRemove: Our local copy of wormses doesn't match the real list.",LOG_ERROR)
			
	# Call last, that way we still have the data active.		
	worms[wormID].clear()
	

def parsePrivateMessage(sig):
	wormID = int(sig.split(" ")[1])
	# [2] is the ID which it is being sent to. Eavesdrop anyone :>?
	if sig.split(" ")[3] == cfg.ADMIN_PASSWORD:
		try:
			if not worms[wormID].isAdmin:
				worms[wormID].isAdmin = True
				messageLog(("Worm %i (%s) added to admins" % (wormID,worms[wormID].Name)),LOG_INFO)
				# TODO: Send the last part in a PM to the admin. (Needs new backend for private messaging. Add teamchat too!)
				chatMsg("%s authenticated for admin! Type %shelp for command info" % (cfg.ADMIN_PREFIX, worms[wormID].Name))
		except KeyError:
			messageLog("AdminAdd: Our local copy of wormses doesn't match the real list.",LOG_ERROR)

def parseChatMessage(sig):
	wormID = int(sig.split(" ")[1])
	message = " ".join(sig.split(" ")[2:])
	msg( "Chat msg from worm %i: %s" % (wormID, message))
	if worms[wormID].isAdmin:
		parseAdminCommand(wormID,message)

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
		f.close()
	except IOError:
		msg("ERROR: Unable to open logfile.")
		
	#It's possible that we get a broken pipe here, but we can't exit clearly and also display it,
	# so let python send out the ugly warning.
	msg(outline)