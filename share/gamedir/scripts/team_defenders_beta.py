#!/usr/bin/python3 -u
#  _______                     _____        __               _               
# |__   __|                   |  __ \      / _|             | |              
#    | | ___  __ _ _ __ ___   | |  | | ___| |_ ___ _ __   __| | ___ _ __ ___ 
#    | |/ _ \/ _` | '_ ` _ \  | |  | |/ _ \  _/ _ \ '_ \ / _` |/ _ \ '__/ __|
#    | |  __/ (_| | | | | | | | |__| |  __/ ||  __/ | | | (_| |  __/ |  \__ \
#    |_|\___|\__,_|_| |_| |_| |_____/ \___|_| \___|_| |_|\__,_|\___|_|  |___/
#                                                                                                                                                        
# This is a dedicated server script that uses the Army Factory tool.
# The human players fight in team against teams of bots.

#========= Imports ========================================
import sys
import os
sys.path.append( os.path.dirname(sys.argv[0]) ) # Add the the folder this file is in to the system path.
import time
import random
import re
import traceback
import tools.commands as cmd
import tools.army_factory.main as army_factory

#========= Configuration ==================================

GAME_OPTIONS = {	
	"GameOptions.GameInfo.GameType":                "Team Death Match",
	"GameOptions.GameInfo.FullAimAngle":            1,
	"GameOptions.GameInfo.LoadingTime":             0,
	"GameOptions.GameInfo.ModName":                 "Liero v2.0",
	"GameOptions.Network.ServerName":               "Experimental Server",
	"GameOptions.Server.MaxPlayers":                32,
	"GameOptions.Server.AllowConnectDuringGame":    1,
	"GameOptions.GameInfo.AllowEmptyGames":         1,
	"GameOptions.GameInfo.ImmediateStart":          1,
	"GameOptions.GameInfo.TimeLimit":               5,
	"GameOptions.GameInfo.SelfHit":                 0,
	"GameOptions.GameInfo.SelfInjure":              0,
	"GameOptions.GameInfo.TeamHit":                 0,
	"GameOptions.GameInfo.TeamInjure":              0,
	"GameOptions.Server.WeaponSelectionMaxTime":    60,
	"GameOptions.GameInfo.CTF_AllowRopeForCarrier": 0,
	"GameOptions.GameInfo.RelativeAirJump":         0, # SorZ said it's lame
	"GameOptions.GameInfo.InfiniteMap":             0, # Infinite map is still broken and the rope is warping
	# Various options that should be set, you don't need to touch them in most cases
	"GameOptions.Server.ServerSideHealth":          0, # Turn this on if ppl hack and don't die on your server
	"GameOptions.Server.AllowNickChange":           1,
	"GameOptions.Network.AllowRemoteBots":          1,
	"GameOptions.Network.AllowWantsJoinMsg":        1,
	"GameOptions.Network.WantsToJoinFromBanned":    0,
	"GameOptions.Network.UseIpToCountry":           1,
	"GameOptions.Network.RegisterServer":           1,
	"GameOptions.Network.Speed":                    2, # 2 = LAN, do not change
	"GameOptions.Advanced.MaxFPS":                  95, # Higher values will decrease netlag, also needed if ServerSideHealth = 1, 
	"GameOptions.Game.AntilagMovementPrediction":   1, # If ServerSideHealth = 1 this influences gameplay
	"GameOptions.Misc.LogConversations":            0,
	"GameOptions.Advanced.MatchLogging":            0, # Do not save game results screenshot
	"GameOptions.Misc.ScreenshotFormat":            1, # 0 = JPG, 1 = PNG
	"GameOptions.Network.EnableChat":               0, # No IRC chat needed for ded server
	"GameOptions.Network.AutoSetupHttpProxy":       0,
	"GameOptions.Network.HttpProxy":                "",
	"GameOptions.Advanced.MaxCachedEntries":        0, # Disable cache for ded server
}

DEFENDER_TEAM = 0 # Blue
ENEMY_TEAM = 1 # Red

#========= Initializations ================================

# Find a list of all available maps.
all_available_maps = []
while len(all_available_maps) == 0:
	# We repeat this because right at OLX startup, it may return an empty list.
	all_available_maps = cmd.listMaps()
	time.sleep(1)

if float(re.match("([A-Za-z]+/)?(?P<num>[0-9]+\.[0-9]+).*", cmd.getVar("GameOptions.Network.ForceMinVersion")[0]).group("num")) + 0.001 < 0.59:
	cmd.msg("LX levels only")
	all_available_maps = [ x for x in all_available_maps if x.endswith(".lxl") ]
else:
	cmd.msg("all levels")

#========= Definitions ====================================
def init_server():
	"""This function sets the the default game options and starts the server."""
	global GAME_OPTIONS;
	
	#Set the game options
	for key, value in GAME_OPTIONS.items():
		cmd.setVar( key, value )
	
	cmd.startLobby() # This will start the server.

def signal_handler(signal):
	signal_name = signal[0]

	if signal_name == "newworm" and not worm_is_local(signal[1]):
		cmd.privateMsg(signal[1], "Welcome %s!<br> Your team's mission is to defend this land from the invading enemies." % get_command_safe_string(signal[2]))
		cmd.setWormTeam(signal[1], DEFENDER_TEAM) # Player from outside the dedicated server
		
	if signal_name == "quit":
		exit()
	
	if signal_name == "backtolobby" or signal_name == "lobbystarted":
		# Kick all bots
		cmd.kickBots()
		
		# select next map
		cmd.map(random.choice(all_available_maps)) 

	if signal_name in [ "backtolobby", "lobbystarted", "newworm" ] and not cmd.getGameState()[0] in [ "S_SVRPLAYING", "S_SVRWEAPONS" ]:
		start_game_ret = cmd.startGame()
		if len(start_game_ret) > 0: # this means an error
			cmd.chatMsg(start_game_ret[0])
		
		# Add new bots:
		the_army = army_factory.produce(random.choice(['alien_robots_inc', 'hungry_zombie_kittens', 'the_seven_dwarfs', 'aho_dojo_ninjas', 'bunny_invasion', 'the_grandpa_association', 'the_wizard_and_his_evil_stickmen']), get_num_foreign_worms()+0.5, 0, ENEMY_TEAM)
		#'alien_robots_inc', 'hungry_zombie_kittens', 'the_seven_dwarfs', 'aho_dojo_ninjas', 'bunny_invasion', 'the_grandpa_association', 'the_wizard_and_his_evil_stickmen'
		cmd.chatMsg("Now fighting...")
		cmd.chatMsg("<b>"+get_command_safe_string(the_army.name)+"</b>")
		cmd.chatMsg("<em>"+get_command_safe_string(the_army.description)+"</em>")

			
def get_command_safe_string(string):
	return string.replace('"', "''")

def worm_is_local(id):
	"""Uses whois to find out if the worm is a local one or not"""
	cmd_resp = parse_olx_dict_format(cmd.whoIs(id))
	if len(cmd_resp) == 0:
		return False #Ugly error handling
	if cmd_resp["NetSpeed"] == "local":
		return True
	return False

def get_num_foreign_worms():
	ret = 0
	ids = cmd.getWormList()
	if len(ids) < 1:
		return 0 #Ugly error handling
	for id in ids:
		if not worm_is_local(id):
			ret += 1	
	return ret
	
def parse_olx_dict_format(list):
	return dict([ re.match("([^:]+): (.*)", item).groups() for item in list ])
	

#========= Action! ========================================
init_server()
while True:
	try:
		signal_handler(cmd.nextSignal())
	except SystemExit:
		break
	except:
		cmd.msg( "Unexpected error in signal handler main loop:" )
		for line in traceback.format_exc().splitlines():
			cmd.msg( get_command_safe_string(line) )	