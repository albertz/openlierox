#!/usr/bin/python -u
# Dedicated Control handler script for OpenLieroX
# (http://openlierox.sourceforge.net)

import time
import os
import sys
import threading
import traceback

## Global vars (across all modules)
import dedicated_control_globals
g = dedicated_control_globals

import dedicated_config
cfg = dedicated_config

import dedicated_control_io
io = dedicated_control_io
setvar = io.setvar
formatExceptionInfo = io.formatExceptionInfo

import dedicated_control_ranking
ranking = dedicated_control_ranking


def adminCommandHelp(wormid):
	io.privateMsg(wormid, "Admin help:")
	io.privateMsg(wormid, "%skick wormID [reason]" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%sban wormID [reason]" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%smute wormID" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%smod modName (or part of name)" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%smap mapName" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%spreset presetName [repeatCount]" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%slt loadingTime" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%steam wormID teamID (0123 or brgy)" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%sstart - start game now" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%sstop - go to lobby" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%spause - pause ded script" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%sunpause - resume ded script" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%ssetvar varname value" % cfg.ADMIN_PREFIX)
	io.privateMsg(wormid, "%sauthorize wormID" % cfg.ADMIN_PREFIX)
	if g.adminCommandHelp_Preset:
		g.adminCommandHelp_Preset(wormid)

# Admin interface
def parseAdminCommand(wormid,message):
	try: # Do not check on msg size or anything, exception handling is further down
		if (not message.startswith(cfg.ADMIN_PREFIX)):
			return False # normal chat

		cmd = message.split(" ")[0]
		cmd = cmd.replace(cfg.ADMIN_PREFIX,"",1) #Remove the prefix

		io.messageLog("%i:%s issued %s" % (wormid,g.worms[wormid].Name,cmd.replace(cfg.ADMIN_PREFIX,"",1)),io.LOG_ADMIN)

		# Unnecesary to split multiple times, this saves CPU.
		params = message.split(" ")[1:]

		if cmd == "help":
			adminCommandHelp(wormid)
		elif cmd == "kick":
			if len(params) > 1: # Given some reason
				io.kickWorm( int( params[0] ), " ".join(params[1:]) )
			else:
				io.kickWorm( int( params[0] ) )
		elif cmd == "ban":
			if len(params) > 1: # Given some reason
				io.banWorm( int( params[0] ), " ".join(params[1:]) )
			else:
				io.banWorm( int( params[0] ) )
		elif cmd == "mute":
			io.muteWorm( int( params[0] ) )
		elif cmd == "mod":
			mod = ""
			for m in g.availableMods:
				if m.lower().find(" ".join(params[0:]).lower()) != -1:
					mod = m
					break
			if mod == "":
				io.privateMsg(wormid,"Invalid mod name")
			else:
				io.setvar("GameOptions.GameInfo.ModName", mod) # In case mod name contains spaces
		elif cmd == "map":
			level = ""
			for l in g.availableLevels:
				if l.lower().find(" ".join(params[0:]).lower()) != -1:
					level = l
					break
			if level == "":
				io.privateMsg(wormid,"Invalid map name")
			else:
				io.setvar("GameOptions.GameInfo.LevelName", level) # In case map name contains spaces
		elif cmd == "preset":
			preset = -1
			presetCount = 1
			if len(params) > 1:
				presetCount = int(params[1])
			for p in range(len(g.availablePresets)):
				if g.availablePresets[p].lower().find(params[0].lower()) != -1:
					preset = p
					break
			if preset == -1:
				io.privateMsg(wormid,"Invalid preset name")
			else:
				g.nextPresets = []
				for f in range(presetCount):
					g.nextPresets.append(g.availablePresets[preset])
				dedicated_control_globals.selectNextPreset()
		elif cmd == "lt":
			io.setvar("GameOptions.GameInfo.LoadingTime", params[0])
		elif cmd == "start":
			io.startGame()
		elif cmd == "stop":
			io.gotoLobby()
		elif cmd == "pause":
			io.privateMsg(wormid,"Ded script paused")
			g.scriptPaused = True
		elif cmd == "unpause":
			io.privateMsg(wormid,"Ded script continues")
			g.scriptPaused = False
		elif cmd == "setvar":
			io.setvar(params[0], " ".join(params[1:])) # In case value contains spaces
		elif cmd == "authorize":
			try:
				wormID = int(params[0])
				if not g.worms[wormID].isAdmin:
					g.worms[wormID].isAdmin = True
					io.authorizeWorm(wormID)
					io.messageLog( "Worm %i (%s) added to admins by %i (%s)" % (wormID,g.worms[wormID].Name,wormid,g.worms[wormid].Name),io.LOG_INFO)
					io.privateMsg(wormID, "%s made you admin! Type %shelp for commands" % (g.worms[wormid].Name,cfg.ADMIN_PREFIX))
					io.privateMsg(wormid, "%s added to admins." % g.worms[wormID].Name)
			except KeyError:
				io.messageLog("parseAdminCommand: Our local copy of wormses doesn't match the real list.",io.LOG_ERROR)
		elif g.parseAdminCommand_Preset and g.parseAdminCommand_Preset(wormid, cmd, params):
			pass
		else:
			raise Exception, "Invalid admin command"

	except: # All python classes derive from main "Exception", but confused me, this has the same effect.
		io.privateMsg(wormid, "Invalid admin command")
		io.messageLog(formatExceptionInfo(),io.LOG_ERROR) #Helps to fix errors
		return False
	return True

# User interface

def userCommandHelp(wormid):
	if cfg.ALLOW_TEAM_CHANGE:
		msg = "%steam [b/r" % (cfg.USER_PREFIX)
		if cfg.MAX_TEAMS >= 3:
			msg += "/g"
		if cfg.MAX_TEAMS >= 4:
			msg += "/y"
		msg += "] - set your team"
		io.privateMsg(wormid, msg + " - set your team")
	if cfg.RANKING:
		io.privateMsg(wormid, "%stoprank - display the best players" % cfg.USER_PREFIX )
		io.privateMsg(wormid, "%srank [name] - display your or other player rank" % cfg.USER_PREFIX )
		io.privateMsg(wormid, "%sranktotal - display the number of players in the ranking" % cfg.USER_PREFIX )
	if g.userCommandHelp_Preset:
		g.userCommandHelp_Preset(wormid)

def parseUserCommand(wormid,message):
	try: # Do not check on msg size or anything, exception handling is further down
		if (not message.startswith(cfg.USER_PREFIX)):
			return False # normal chat

		cmd = message.split(" ")[0]
		cmd = cmd.replace(cfg.USER_PREFIX,"",1) #Remove the prefix

		io.messageLog("%i:%s user cmd %s" % (wormid,g.worms[wormid].Name,cmd.replace(cfg.USER_PREFIX,"",1)),io.LOG_ADMIN)

		# Unnecesary to split multiple times, this saves CPU.
		params = message.split(" ")[1:]

		if cmd == "help":
			userCommandHelp(wormid)
		elif cmd == "team" and cfg.ALLOW_TEAM_CHANGE and gameState != GAME_PLAYING:
			if not params:
				userCommandHelp(wormid)
				raise Exception, "You need to specify a team"
			if params[0].lower() == "blue" or params[0].lower() == "b":
				io.setWormTeam(wormid, 0)
			elif params[0].lower() == "red" or params[0].lower() == "r":
				io.setWormTeam(wormid, 1)
			elif ( params[0].lower() == "green" or params[0].lower() == "g" ) and cfg.MAX_TEAMS >= 3:
				io.setWormTeam(wormid, 2)
			elif ( params[0].lower() == "yellow" or params[0].lower() == "y" ) and cfg.MAX_TEAMS >= 4:
				io.setWormTeam(wormid, 3)
		elif cmd == "toprank" and cfg.RANKING:
			ranking.firstRank(wormid)
		elif cmd == "rank" and cfg.RANKING:
			if wormid in g.worms:
				wormName = g.worms[wormid].Name
				if params:
					wormName = " ".join(params)
				ranking.myRank(wormName, wormid)
		elif cmd == "ranktotal" and cfg.RANKING:
			io.privateMsg(wormid, "There are " + str(len(ranking.rank)) + " players in the ranking.")
		elif g.parseUserCommand_Preset and g.parseUserCommand_Preset(wormid, cmd, params):
			pass
		else:
			raise Exception, "Invalid user command"

	except: # All python classes derive from main "Exception", but confused me, this has the same effect.
			# TODO, send what's passed in the exception to the user?
		io.privateMsg(wormid, "Invalid user command")
		io.messageLog(formatExceptionInfo(),io.LOG_ERROR) #Helps to fix errors
		return False
	return True

