#!/usr/bin/python -u

# TODO: what is this script for? why not using cfg/options.cfg ?
# TODO: This script should only contain special settings which are not covered by options.cfg

# TODO: register commands in ChatCommand system
ADMIN_PREFIX = "!" # What kind of prefix you want for admin commands. Example: !?-.@$ A.K.A you can use prettymuch everything.

# Where to log what is happening
LOG_FILE = "dedicated_control.log"

MIN_PLAYERS = 2
MIN_PLAYERS_TEAMS = 40 # Players will be split in two teams automatically if there is enough players
MAX_TEAMS = 4 # Only blue and red teams
TOO_FEW_PLAYERS_MESSAGE = "Game will start with minimum %i players. Team Deathmatch if there's %i or more players" % (MIN_PLAYERS, MIN_PLAYERS_TEAMS)
WAIT_BEFORE_SPAMMING_TOO_FEW_PLAYERS_MESSAGE = 30 # Seconds to wait before another "Game will start with %i players" message

# Seconds before rotating preset
PRESET_TIMEOUT = 300

WAIT_AFTER_GAME = 0 # Seconds to wait in lobby after round finished
WAIT_BEFORE_GAME = 0 # Seconds to wait in lobby before next round, will give some message
WAIT_BEFORE_GAME_MESSAGE = "Game will start in %i seconds" % WAIT_BEFORE_GAME

import dedicated_control_io as io # control handler

GAME_LIVES = int(io.getVar("GameOptions.GameInfo.Lives"))
GAME_MAX_KILLS = int(io.getVar("GameOptions.GameInfo.KillLimit"))
GAME_MAX_TIME = float(io.getVar("GameOptions.GameInfo.TimeLimit"))
WEAPON_SELECTION_TIME = int(io.getVar("GameOptions.GameInfo.WeaponSelectionMaxTime"))

# Note: This is unfair and I don't thing it is such a good idea. (At least for the average player, only 
# pro-gamers perhaps want that.)
# A user with a high ping doesn't give any disadvantages to other players (or at least that should not be the case and I wonder if it is).
MAX_PING = 30000 # Max ping to auto-kick player

# TODO: We should use the OLX chatcommand system.
# TODO: Register dedscript commands in OLX chatcommand system.
# Users can enter some commands too
USER_PREFIX = ADMIN_PREFIX # Change to have custom user command prefix instead of "!"
ALLOW_TEAM_CHANGE = True # Player should type "!b", "!r", "!g", or "!y" to set it's own team
TEAM_CHANGE_MESSAGE = "Set your team with %steam b/r" % USER_PREFIX
if MAX_TEAMS >= 3:
	TEAM_CHANGE_MESSAGE += "/g"
if MAX_TEAMS >= 4:
	TEAM_CHANGE_MESSAGE += "/y"


RANKING = 1 # Should we allow !rank user command
RANKING_AUTHENTICATION = 0 # Should we authenticate worm by it's skin color (pretty weak, but !password cmd is kinda ugly)

VOTING = 1 # Should we allow voting for preset/map/mod/lt/kick/mute
VOTING_PERCENT = 51 # How much users in percent should vote yes for vote to pass
VOTING_TIME = 45 # For how much time we should wait votes from users
VOTING_COUNT_NEGATIVE = 1 # If we should count negative votes, so users can fail unpopular vote before timeout
VOTING_KICK_TIME = 5 # Time in minutes when user kicked by voting cannot re-join server (it auto-kicks user again with message)


# List of levels - preset chooses a random level from those
LEVELS = [		"CastleStrike.lxl",
			"FossilFacility.lxl",
			"LieroFactory(Revisited).lxl",
			"LieroFactory_Maintenance.lxl",
			"JailBreak.lxl",
			"JukkeDome.lxl",
			"Lake Liero.lxl",
			"Lamda_bunker_(Razvisited).lxl",
			"MsPaintPower.lxl",
			"MsPaintPower2.lxl",
			"Ore Quarry.lxl",
			"Poo Arena.lxl",
			"RIP_Home.lxl",
			"Sunrise_Mansion.lxl",
			"wormmountain.lxl",
			"Alien Hood.lxl",
#			"FightBox.lxl",
#			"Duel.lxl",
			"Tetrisv2.lxl",
#			"Dirt Level.lxl",
			"HW-house.lxl",
			"GammaComplex.lxl",
			"Kirby_ice_cream_island.lxl",
			"KitchenKombat.lxl",
			"Labrinth.lxl"
		]

# List of presets to cycle on server - you may specify some preset multiple times, then it will have higher chances of appearing
# If this list is empty all presets are used
#PRESETS = [ "Mortars", "MSF_II", "Classic", "Shock", "ModernWarfare", "HideAndSeek", "8Bit" ]
PRESETS = [ "Mortars" ]

# General options that should be set
GLOBAL_SETTINGS = {	

	# Various options that should be set, you don't need to touch them in most cases

	"GameOptions.GameInfo.ServerSideHealth":        0, # Turn this on if ppl hack and don't die on your server
	"GameOptions.GameInfo.AllowNickChange":         1,
	"GameOptions.GameInfo.AllowStrafing":           1,
	"GameOptions.Network.AllowRemoteBots":          1,
	"GameOptions.Network.AllowWantsJoinMsg":        1,
	"GameOptions.Network.WantsToJoinFromBanned":    0,
	"GameOptions.Network.UseIpToCountry":           1, # Do not change, needed for correct server messages ; TODO: fix that
	"GameOptions.Network.RegisterServer":           1,
	"GameOptions.Network.Speed":                    2, # 2 = LAN, do not change
	"GameOptions.Advanced.MaxFPS":                  95, # Higher values will decrease netlag, also needed if ServerSideHealth = 1, 
	"GameOptions.Game.AntilagMovementPrediction":   1, # If ServerSideHealth = 1 this influences gameplay
# TODO: OLX should not do that anyway in dedicated mode. if it does, please fix that in OLX
#	"GameOptions.Game.Blood":                       0, # Ded server does not need any eye candies
#	"GameOptions.Game.Particles":                   0, # Ded server does not need any eye candies
#	"GameOptions.Game.Shadows":                     0, # Ded server does not need any eye candies
	"GameOptions.Misc.LogConversations":            0,
	"GameOptions.Advanced.MatchLogging":            0, # Do not save game results screenshot
	"GameOptions.Misc.ScreenshotFormat":            1, # 0 = JPG, 1 = PNG
	"GameOptions.Network.EnableChat":               0, # No IRC chat needed for ded server
	"GameOptions.Network.AutoSetupHttpProxy":       0,
	"GameOptions.Network.HttpProxy":                "",
}

