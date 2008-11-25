#!/usr/bin/python -u
# Dedicated Control handler script for OpenLieroX
# (http://openlierox.sourceforge.net)

# Needed for sleeping/pausing execution
import time
# Needed for directory access
import os
import sys
import threading
import traceback

import dedicated_config  # Per-host config like admin password
cfg = dedicated_config # shortcut

# TODO: is this up-to-date with console support on win?
# Print Python script errors to external file -
# on Windows it cannot print errors to console
#if sys.platform == "win32":
sys.stderr = open(cfg.ERROR_FILE, "w", 0)

from dedicated_control_io import *

import dedicated_control_ranking
ranking = dedicated_control_ranking

stdinInputThread = GetStdinThread()

# Uncomment following 3 lines to get lots of debug spam into dedicated_control_errors.log file
#def HeavyDebugTrace(frame,event,arg): 
#	sys.stderr.write( 'Trace: ' + str(event) + ': ' + str(arg) + ' at ' + str(frame.f_code) + "\n" )
#sys.settrace(HeavyDebugTrace)

## Global vars ##
curdir = os.getcwd()
curdir = os.path.join(curdir,"scripts")
presetDir = os.path.join(curdir,"presets")
# Should include several searchpaths, but now they contain just OLX root dir
levelDir = os.path.join(os.getcwd(),"levels")
modDir = os.getcwd()

# Preset stuffies
availiblePresets = list()
maxPresets = 0
curPreset = 0

availibleLevels = list()
availibleMods = list()

#worms = {} # Dictionary of all online worms - contains only worm name currently
worms = {} # List of all worms on the server
# Bots don't need to be itterated with the other ones.
bots = {}  # Dictionary of all possible bots
#admins = {} # Dictionary of all admin worms

# Function that controls ded server behavior
controlHandler = None

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
		self.Ping = [] # Contains 10 ping values, real ping = average of them
		self.Lives = -1 # -1 means Out
		self.Team = 0
		self.Alive = False

	def clear(self):
		self.__init__()

# Game states
GAME_QUIT = 0
GAME_LOBBY = 1
GAME_WEAPONS = 2
GAME_PLAYING = 3


gameState = GAME_QUIT

sentStartGame = False

scriptPaused = False

# Stolen from http://www.linuxjournal.com/article/5821
def formatExceptionInfo(maxTBlevel=5):
	cla, exc, trbk = sys.exc_info()
	excName = cla.__name__
	try:
		excArgs = exc.__dict__["args"]
	except KeyError:
		excArgs = "<no args>"
	excTb = traceback.format_tb(trbk, maxTBlevel)
	return (excName, excArgs, excTb)


def init():
	initPresets()
	initLevelList()
	initModList()

## High-level processing ##

def updateWorms(sig):
	if sig.find("newworm ") == 0 or sig.find("wormleft ") == 0:
		getWormList()

def getNumWorms():
	global worms
	i = 0
	for w in worms.values():
		if w.iID != -1:
			i += 1

	return i

def setWormTeam(iID, team):
	global worms
	if iID in worms.keys() and worms[iID].iID != -1:
		worms[iID].Team = team
		setWormTeam_io(iID, team)
	else:
		messageLog("Worm id %i invalid" % iID ,LOG_ADMIN)


adminCommandHelp_Preset = None
parseAdminCommand_Preset = None
userCommandHelp_Preset = None
parseUserCommand_Preset = None

def adminCommandHelp(wormid):
	privateMsg(wormid, "Admin help:")
	privateMsg(wormid, "%skick wormID [reason]" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%sban wormID [reason]" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%smute wormID" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%smod modName (or part)" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%smap mapName (or part)" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%spreset presetName (or part)" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%slt loadingTime" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%steam wormID teamID (0123 or brgy)" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%sstart - start game now" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%sstop - go to lobby" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%spause - pause ded script" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%sunpause - resume ded script" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%ssetvar varname value" % cfg.ADMIN_PREFIX)
	privateMsg(wormid, "%sauthorize wormID" % cfg.ADMIN_PREFIX)
	if adminCommandHelp_Preset:
		adminCommandHelp_Preset(wormid)

# Admin interface
def parseAdminCommand(wormid,message):
	global worms, curPreset, availiblePresets, availibleLevels, availibleMods, parseAdminCommand_Preset
	try: # Do not check on msg size or anything, exception handling is further down
		if (not message.startswith(cfg.ADMIN_PREFIX)):
			return False # normal chat

		cmd = message.split(" ")[0]
		cmd = cmd.replace(cfg.ADMIN_PREFIX,"",1) #Remove the prefix

		messageLog("%i:%s issued %s" % (wormid,worms[wormid].Name,cmd.replace(cfg.ADMIN_PREFIX,"",1)),LOG_ADMIN)

		# Unnecesary to split multiple times, this saves CPU.
		params = message.split(" ")[1:]

		if cmd == "help":
			adminCommandHelp(wormid)
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
			mod = ""
			for m in availibleMods:
				if m.lower().find(" ".join(params[0:]).lower()) != -1:
					mod = m
					break
			if mod == "":
				privateMsg(wormid,"Invalid mod name")
			else:
				setvar("GameOptions.GameInfo.ModName", mod) # In case mod name contains spaces
		elif cmd == "map":
			level = ""
			for l in availibleLevels:
				if l.lower().find(" ".join(params[0:]).lower()) != -1:
					level = l
					break
			if level == "":
				privateMsg(wormid,"Invalid map name")
			else:
				setvar("GameOptions.GameInfo.LevelName", level) # In case map name contains spaces
		elif cmd == "preset":
			preset = -1
			for p in range(len(availiblePresets)):
				if availiblePresets[p].lower().find(" ".join(params[0:]).lower()) != -1:
					preset = p
					break
			if preset == -1:
				privateMsg(wormid,"Invalid preset name")
			else:
				curPreset = preset
				selectNextPreset()
		elif cmd == "lt":
			setvar("GameOptions.GameInfo.LoadingTime", params[0])
		elif cmd == "start":
			startGame()
		elif cmd == "stop":
			gotoLobby()
		elif cmd == "pause":
			privateMsg(wormid,"Ded script paused")
			scriptPaused = True
		elif cmd == "unpause":
			privateMsg(wormid,"Ded script continues")
			scriptPaused = False
		elif cmd == "setvar":
			setvar(params[0], " ".join(params[1:])) # In case value contains spaces
		elif cmd == "authorize":
			try:
				wormID = int(params[0])
				if not worms[wormID].isAdmin:
					worms[wormID].isAdmin = True
					authorizeWorm(wormID)
					messageLog(("Worm %i (%s) added to admins by %i (%s)" % (wormID,worms[wormID].Name),wormid,worms[wormid].Name),LOG_INFO)
					privateMsg(wormID, "%s made you admin! Type %shelp for commands" % (worms[wormid].Name,cfg.ADMIN_PREFIX))
					privateMsg(wormid, "%s added to admins." % worms[wormID].Name)
			except KeyError:
				messageLog("parseAdminCommand: Our local copy of wormses doesn't match the real list.",LOG_ERROR)
		elif parseAdminCommand_Preset and parseAdminCommand_Preset(wormid, cmd, params):
			pass
		else:
			raise Exception, "Invalid admin command"

	except: # All python classes derive from main "Exception", but confused me, this has the same effect.
		privateMsg(wormid, "Invalid admin command")
		messageLog(formatExceptionInfo(),LOG_ERROR) #Helps to fix errors
		return False
	return True

# User interface

def userCommandHelp(wormid):
	if cfg.ALLOW_TEAM_CHANGE:
		msg = "%steam <blue/red" % (cfg.USER_PREFIX)
		if cfg.MAX_TEAMS >= 3:
			msg += "/green"
		if cfg.MAX_TEAMS >= 4:
			msg += "/yellow"
		msg += ">"
		privateMsg(wormid, msg + " - set your team")
	if cfg.RANKING:
		privateMsg(wormid, "%stoprank - display the best players" % cfg.USER_PREFIX )
		privateMsg(wormid, "%srank [name] - display your or other player rank" % cfg.USER_PREFIX )
		privateMsg(wormid, "%sranktotal - display the number of players in the ranking" % cfg.USER_PREFIX )
	if userCommandHelp_Preset:
		userCommandHelp_Preset(wormid)

def parseUserCommand(wormid,message):
	global worms, curPreset, availiblePresets, availibleLevels, availibleMods, gameState, parseUserCommand_Preset, rank
	try: # Do not check on msg size or anything, exception handling is further down
		if (not message.startswith(cfg.USER_PREFIX)):
			return False # normal chat

		cmd = message.split(" ")[0]
		cmd = cmd.replace(cfg.USER_PREFIX,"",1) #Remove the prefix

		messageLog("%i:%s user cmd %s" % (wormid,worms[wormid].Name,cmd.replace(cfg.USER_PREFIX,"",1)),LOG_ADMIN)

		# Unnecesary to split multiple times, this saves CPU.
		params = message.split(" ")[1:]

		if cmd == "help":
			userCommandHelp(wormid)
		elif cmd == "team" and cfg.ALLOW_TEAM_CHANGE and gameState != GAME_PLAYING:
			if not params:
				userCommandHelp(wormid)
				raise Exception, "You need to specify a team"
			if params[0].lower() == "blue" or params[0].lower() == "b":
				setWormTeam(wormid, 0)
			elif params[0].lower() == "red" or params[0].lower() == "r":
				setWormTeam(wormid, 1)
			elif ( params[0].lower() == "green" or params[0].lower() == "g" ) and cfg.MAX_TEAMS >= 3:
				setWormTeam(wormid, 2)
			elif ( params[0].lower() == "yellow" or params[0].lower() == "y" ) and cfg.MAX_TEAMS >= 4:
				setWormTeam(wormid, 3)
		elif cmd == "toprank" and cfg.RANKING:
			ranking.firstRank(wormid)
		elif cmd == "rank" and cfg.RANKING:
			if wormid in worms:
				wormName = worms[wormid].Name
				if params:
					wormName = " ".join(params)
				ranking.myRank(wormName, wormid)
		elif cmd == "ranktotal" and cfg.RANKING:
			privateMsg(wormid, "There are " + str(len(ranking.rank)) + " players in the ranking.")
		elif parseUserCommand_Preset and parseUserCommand_Preset(wormid, cmd, params):
			pass
		else:
			raise Exception, "Invalid user command"

	except: # All python classes derive from main "Exception", but confused me, this has the same effect.
			# TODO, send what's passed in the exception to the user?
		privateMsg(wormid, "Invalid user command")
		messageLog(formatExceptionInfo(),LOG_ERROR) #Helps to fix errors
		return False
	return True



# Parses all signals that are not 2 way (like getip info -> olx returns info)
# Returns False if there's nothing to read
def signalHandler(sig):
	global gameState
	global sentStartGame
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
		parseWormDied(sig)
	elif header == "wormspawned":
		parseWormSpawned(sig)

	## Check GameState ##
	elif header == "quit":
		gameState = GAME_QUIT
	elif header == "errorstartlobby":
		gameState = GAME_QUIT
		messageLog("errorstartlobby",LOG_ERROR)

	elif header == "backtolobby" or header == "lobbystarted":
		if cfg.RANKING:
			ranking.refreshRank()
		gameState = GAME_LOBBY
		sentStartGame = False
	elif header == "errorstartgame":
		gameState = GAME_LOBBY
		messageLog("errorstartgame",LOG_ERROR)
		sentStartGame = False

	elif header == "weaponselections":
		gameState = GAME_WEAPONS
		sentStartGame = False
	elif header == "gamestarted":
		gameState = GAME_PLAYING
		sentStartGame = False

	#if sig != "":
		#msg(sig)
	return True

def parseNewWorm(sig):
	global worms
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
	
	# Balance teams
	teams = [0,0,0,0]
	for w in worms.keys():
		if worms[w].iID == -1:
			continue
		teams[worms[w].Team] += 1
	minTeam = 0
	minTeamCount = teams[0]
	for f in range(cfg.MAX_TEAMS):
		if minTeamCount > teams[f]:
			minTeamCount = teams[f]
			minTeam = f

	setWormTeam(wormID, minTeam)
	messageLog("New worm " + str(wormID) + " set team " + str(minTeam) + " teamcount " + str(teams), LOG_INFO)

def parseWormLeft(sig):
	global worms
	wormID = int(sig.split(" ")[1])
	name = " ".join(sig.split(" ")[2:])

	try:
		if worms[wormID].isAdmin:
			messageLog(("Worm %i (%s) removed from admins" % (wormID,name)),LOG_ADMIN)
	except KeyError:
		messageLog("AdminRemove: Our local copy of wormses doesn't match the real list.",LOG_ERROR)

	# Call last, that way we still have the data active.
	worms[wormID].clear()


def parsePrivateMessage(sig):
	global worms
	wormID = int(sig.split(" ")[1])
	# [2] is the ID which it is being sent to. Eavesdrop anyone :>?
	if sig.split(" ")[3] == cfg.ADMIN_PASSWORD:
		try:
			if not worms[wormID].isAdmin:
				worms[wormID].isAdmin = True
				messageLog(("Worm %i (%s) added to admins" % (wormID,worms[wormID].Name)),LOG_ADMIN)
				# TODO: Send the last part in a PM to the admin. (Needs new backend for private messaging. Add teamchat too!)
				authorizeWorm(wormID)
				privateMsg(wormID, "%s authenticated for admin! Type %shelp for command info" % (worms[wormID].Name,cfg.ADMIN_PREFIX))
		except KeyError:
			messageLog("AdminAdd: Our local copy of wormses doesn't match the real list.",LOG_ERROR)

def parseChatMessage(sig):
	global worms
	wormID = int(sig.split(" ")[1])
	message = " ".join(sig.split(" ")[2:])
	msg("Chat msg from worm %i: %s" % (wormID, message))
	if worms[wormID].isAdmin:
		if not parseAdminCommand(wormID,message):
			parseUserCommand(wormID,message)
	else:
		parseUserCommand(wormID,message)

def parseWormDied(sig):
	global worms
	deaderID = int(sig.split(" ")[1])
	killerID = int(sig.split(" ")[2])
	worms[deaderID].Lives -= 1
	worms[deaderID].Alive = False

	if not cfg.RANKING:
		return

	try:
		f = open("pwn0meter.txt","a")
		f.write( time.strftime("%Y-%m-%d %H:%M:%S") + "\t" + worms[deaderID].Name + "\t" + worms[killerID].Name + "\n" )
		f.close()
	except IOError:
		msg("ERROR: Unable to open pwn0meter.txt")

	if(deaderID == killerID):
		try:
			ranking.rank[worms[killerID].Name][2] += 1
		except KeyError:
			ranking.rank[worms[killerID].Name] = [0,0,1,len(ranking.rank)+1]
	else:
		try:
			ranking.rank[worms[killerID].Name][0] += 1
		except KeyError:
			ranking.rank[worms[killerID].Name] = [1,0,0,len(ranking.rank)+1]
	try:
		ranking.rank[worms[deaderID].Name][1] += 1
	except KeyError:
		ranking.rank[worms[deaderID].Name] = [0,1,0,len(ranking.rank)+1]

def parseWormSpawned(sig):
	global worms
	wormID = int(sig.split(" ")[1])
	worms[deaderID].Alive = True

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

		if f.lower() != "defaults":
			availiblePresets.append(f)
			maxPresets +=1

	if (maxPresets == 0):
		messageLog("There are no presets availible - nothing to do. Exiting.",LOG_CRITICAL)
		exit()

# initPresets must be called before this - or it will crash
# TODO: Try to make something nicer for the user which doesn't read this
def selectNextPreset():
	global curPreset, maxPresets, availiblePresets, controlHandler

	msg("Preset " + availiblePresets[curPreset])
	chatMsg("Preset " + availiblePresets[curPreset])

	sFile = os.path.join(presetDir,availiblePresets[curPreset])
	sDefaults = os.path.join(presetDir,"Defaults")
	try:
		execfile(sDefaults)
		fPreset = file(sFile,"r")
		line = fPreset.readline()
		if line.find("python") != -1:
			fPreset.close()
			execfile(sFile)
		else:
			print line.strip().replace('"','')
			for line in fPreset.readlines():
				print line.strip().replace('"','')
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


def waitLobbyStarted():
	while True:
		sig = getSignal()
		signalHandler(sig)
		if sig == "lobbystarted":
			return
		time.sleep(1)


def initLevelList():
	global levelDir, availibleLevels
	for f in os.listdir(levelDir):
		if os.path.isdir(f):
			#messageLog("initLevelList: Ignoring \"%s\" - It's a directory" % f, LOG_INFO)
			continue
		#messageLog("Adding level " + f, LOG_INFO)
		availibleLevels.append(f)

def initModList():
	global modDir, availibleMods
	for f in os.listdir(modDir):
		if not os.path.isdir(f):
			continue
		for ff in os.listdir(os.path.join(modDir,f)):
			if ff.lower() == "script.lgs":
				#messageLog("Adding mod " + f, LOG_INFO)
				availibleMods.append(f)

## Control functions

def average(a):
	r = 0
	for i in a:
		r += i
	return r / len(a)

def checkMaxPing():
	global worms
	for f in worms.keys():
		if worms[f].iID == -1 or not worms[f].Alive:
			continue
		ping = int(getWormPing(worms[f].iID))
		if ping > 0:
			worms[f].Ping.insert( 0, ping )
			if len(worms[f].Ping) > 25:
				worms[f].Ping.pop()
				if average(worms[f].Ping) > cfg.MAX_PING:
					kickWorm( worms[f].iID, "Your ping is " + str(average(worms[f].Ping)) + " allowed is " + str(cfg.MAX_PING) )

lobbyChangePresetTimeout = cfg.WAIT_BEFORE_GAME*10
lobbyWaitBeforeGame = cfg.WAIT_BEFORE_GAME
lobbyWaitAfterGame = 0
lobbyWaitGeneral = cfg.WAIT_BEFORE_SPAMMING_TOO_FEW_PLAYERS_MESSAGE
lobbyEnoughPlayers = False
oldGameState = GAME_LOBBY

def controlHandlerDefault():

	global worms, gameState, lobbyChangePresetTimeout, lobbyWaitBeforeGame, lobbyWaitAfterGame, lobbyWaitGeneral, lobbyEnoughPlayers, oldGameState

	if gameState == GAME_LOBBY:

		# Do not check ping in lobby - it's wrong

		lobbyChangePresetTimeout -= 1

		if oldGameState != GAME_LOBBY or lobbyChangePresetTimeout <= 0:
			lobbyChangePresetTimeout = cfg.PRESET_TIMEOUT
			selectNextPreset()
			lobbyEnoughPlayers = False # reset the state
			lobbyWaitGeneral = cfg.WAIT_BEFORE_SPAMMING_TOO_FEW_PLAYERS_MESSAGE
			lobbyWaitAfterGame = 0
			if oldGameState == GAME_PLAYING:
				lobbyWaitAfterGame = cfg.WAIT_AFTER_GAME

		lobbyWaitAfterGame -= 1

		if lobbyWaitAfterGame <= 0:

			lobbyWaitGeneral -= 1

			if not lobbyEnoughPlayers and lobbyWaitGeneral <= 0:
				lobbyWaitGeneral = cfg.WAIT_BEFORE_SPAMMING_TOO_FEW_PLAYERS_MESSAGE
				chatMsg(cfg.TOO_FEW_PLAYERS_MESSAGE)

			if not lobbyEnoughPlayers and getNumWorms() >= cfg.MIN_PLAYERS: # Enough players already - start game
				lobbyEnoughPlayers = True
				chatMsg(cfg.WAIT_BEFORE_GAME_MESSAGE)
				lobbyWaitBeforeGame = cfg.WAIT_BEFORE_GAME

			if lobbyEnoughPlayers and getNumWorms() < cfg.MIN_PLAYERS: # Some players left when game not yet started
				lobbyEnoughPlayers = False
				chatMsg(cfg.TOO_FEW_PLAYERS_MESSAGE)

			if lobbyEnoughPlayers and not sentStartGame:
				lobbyWaitBeforeGame -= 1
				if lobbyWaitBeforeGame <= 0: # Start the game

					if getNumWorms() >= cfg.MIN_PLAYERS_TEAMS: # Split in teams
						setvar("GameOptions.GameInfo.GameType", "1")
						if not cfg.ALLOW_TEAM_CHANGE:
							counter = 0
							for w in worms.values():
								if w.iID != -1:
									setWormTeam( w.iID, counter % cfg.MAX_TEAMS )
									counter += 1
					else:
						setvar("GameOptions.GameInfo.GameType", "0")

					startGame()
					if cfg.ALLOW_TEAM_CHANGE:
						chatMsg(cfg.TEAM_CHANGE_MESSAGE)

	if gameState == GAME_WEAPONS:

		#checkMaxPing()

		if getNumWorms() < cfg.MIN_PLAYERS: # Some players left when game not yet started
			gotoLobby()

	if gameState == GAME_PLAYING:

		checkMaxPing()


controlHandler = controlHandlerDefault

