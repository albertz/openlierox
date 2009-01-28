#!/usr/bin/python -u

# The reason this script resides in cfg/ folder is that the OLX file downloader will 
# deny access to cfg/ folder, and we have admin password here
# .. Why don't we deny access to everything except levels and mods (by checking if a directory
# has got script.lgs in it). Current way sounds sucky to me.

ADMIN_PASSWORD = "secret" # Change it! should be single word without spaces, yet symbols ~!@#$%^&* are encouraged
ADMIN_PREFIX = "!" # What kind of prefix you want for admin commands. Example: !?-.@$ A.K.A you can use prettymuch everything.

# Where to log what is happening
LOG_FILE = "dedicated_control.log"
# If you modify dedicated_control Python will write errors here
ERROR_FILE = "dedicated_control_errors.log"

SERVER_NAME = "Don't join" # The name of your server, can be changed by preset

MIN_PLAYERS = 2
MIN_PLAYERS_TEAMS = 4 # Players will be split in two teams automatically if there is enough players
MAX_TEAMS = 2 # Only blue and red teams
TOO_FEW_PLAYERS_MESSAGE = "Game will start with minimum %i players. Team Deathmatch if there's %i or more players" % (MIN_PLAYERS, MIN_PLAYERS_TEAMS)
WAIT_BEFORE_SPAMMING_TOO_FEW_PLAYERS_MESSAGE = 30 # Seconds to wait before another "Game will start with %i players" message

# Seconds before rotating preset
PRESET_TIMEOUT = 300

WAIT_AFTER_GAME = 10 # Seconds to wait in lobby after round finished
WAIT_BEFORE_GAME = 15 # Seconds to wait in lobby before next round, will give some message
WAIT_BEFORE_GAME_MESSAGE = "Game will start in %i seconds" % WAIT_BEFORE_GAME

GAME_LIVES = -2 # -2 = unlimited, can be changed by TeamArena preset
GAME_MAX_KILLS = 15
GAME_MAX_TIME = 7 # In minutes
WEAPON_SELECTION_TIME = 40 # In seconds

MAX_PING = 400 # Max ping to auto-kick player

# Users can enter some commands too
USER_PREFIX = ADMIN_PREFIX # Change to have custom user command prefix instead of "//"
ALLOW_TEAM_CHANGE = True # Player should type "!b", "!r", "!g", or "!y" to set it's own team
TEAM_CHANGE_MESSAGE = "Set your team with %steam b/r" % USER_PREFIX
if MAX_TEAMS >= 3:
	TEAM_CHANGE_MESSAGE += "/g"
if MAX_TEAMS >= 4:
	TEAM_CHANGE_MESSAGE += "/y"

SERVER_PORT = 23400 # On which port to host a server - better if it's different from the port in your game settings
SERVER_MAX_UPLOAD_LIMIT = 40000 # 40 kilobytes should be enough for 8 players

RANKING = 1 # Should we allow !rank user command
RANKING_AUTHENTICATION = 0 # Should we authenticate worm by it's skin color (pretty weak, but !password cmd is kinda ugly)

# List of levels - preset chooses a random level from those
LEVELS = [	"FossilFacility.lxl",
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
		]

# List of presets to cycle on server - you may specify some preset multiple times, then it will have higher chances of appearing
# If this list is empty all presets are used
PRESETS = [ "Classic", "Mortars", "MSF_II", ]

# General options that should be set
GLOBAL_SETTINGS = {	

	"GameOptions.Network.WelcomeMessage":           "<player> from <country> connected - typing go/spamming is useless",
	"GameOptions.GameInfo.MaxPlayers":              8,
	"GameOptions.GameInfo.WeaponSelectionMaxTime":  40,
	"GameOptions.Network.MaxUploadBandwidth":       40000, # Should be enough for 8-player game, raise this if your network is fast

	# Various options that should be set, you don't need to touch them in most cases

	"GameOptions.GameInfo.ServerSideHealth":        0, # Turn this on if ppl hack and don't die on your server
	"GameOptions.GameInfo.AllowNickChange":         1,
	"GameOptions.GameInfo.AllowStrafing":           1, 
	"GameOptions.Network.AllowRemoteBots":          1,
	"GameOptions.Network.AllowWantsJoinMsg":        1,
	"GameOptions.Network.WantsToJoinFromBanned":    0,
	"GameOptions.Network.UseIpToCountry":           1, # Do not change, needed for correct server messages
	"GameOptions.Network.RegisterServer":           1,
	"GameOptions.Network.Speed":                    2,
	"GameOptions.Advanced.MaxFPS":                  95, # Higher values will decrease netlag, also needed if ServerSideHealth = 1, 
	"GameOptions.Game.AntilagMovementPrediction":   1, # If ServerSideHealth = 1 this influences gameplay
	"GameOptions.Game.Blood":                       0, # Ded server does not need any eye candies
	"GameOptions.Game.Particles":                   0, # Ded server does not need any eye candies
	"GameOptions.Game.Shadows":                     0, # Ded server does not need any eye candies
	"GameOptions.Misc.LogConversations":            0,
	"GameOptions.Advanced.MatchLogging":            0, # Do not save game results screenshot
	"GameOptions.Misc.ScreenshotFormat":            1, # 0 = JPG, 1 = PNG
	"GameOptions.Network.AutoSetupHttpProxy":       1,
	"GameOptions.Network.EnableChat":               0, # No IRC chat needed for ded server
	"GameOptions.Network.HttpProxy":                "",
	"GameServer.bStartDedicated":                   0, # Do not change this
}

