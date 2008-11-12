#!/usr/bin/python -u

# The reason this script resides in cfg/ folder is that the OLX file downloader will 
# deny access to cfg/ folder, and we have admin password here
# .. Why don't we deny access to everything except levels and mods (by checking if a directory
# has got script.lgs in it). Current way sounds sucky to me.

ADMIN_PASSWORD = "secret" # Change it! should be single word without spaces, yet symbols ~!@#$%^&* are encouraged
ADMIN_PREFIX = "//" # What kind of prefix you want for admin commands. Example: !?-.@$ A.K.A you can use prettymuch everything.

# Where to log what is happening
LOG_FILE = "dedicated_control.log"
# If you modify dedicated_control Python will write errors here
ERROR_FILE = "dedicated_control_errors.log"

SERVER_NAME = "Dedicated server" # The name of your server, can be changed by preset
WELCOME_MESSAGE = "<player> from <country> connected - typing go/spamming is useless"

MIN_PLAYERS = 2
MIN_PLAYERS_TEAMS = 4 # Players will be split in two teams automatically if there is enough players
MAX_TEAMS = 2 # Only blue and red teams
MAX_PLAYERS = 8
TOO_FEW_PLAYERS_MESSAGE = "Game will start with minimum %i players. Team Deathmatch if there's %i or more players" % (MIN_PLAYERS, MIN_PLAYERS_TEAMS)
WAIT_BEFORE_SPAMMING_TOO_FEW_PLAYERS_MESSAGE = 30 # Seconds to wait before another "Game will start with %i players" message

# Seconds before rotating preset
PRESET_TIMEOUT = 300

WAIT_AFTER_GAME = 20 # Seconds to wait in lobby after round finished
WAIT_BEFORE_GAME = 15 # Seconds to wait in lobby before next round, will give some message
WAIT_BEFORE_GAME_MESSAGE = "Game will start in %i seconds" % WAIT_BEFORE_GAME

GAME_LIVES = 10
GAME_MAX_KILLS = 20
GAME_MAX_TIME = 10 # In minutes
WEAPON_SELECTION_TIME = 40 # In seconds

MAX_PING = 400 # Max ping to auto-kick player

# Users can enter some commands too
USER_PREFIX = ADMIN_PREFIX # Change to have custom user command prefix instead of "//"
ALLOW_TEAM_CHANGE = True # Player should type "//b", "//r", "//g", or "//y" to set it's own team
TEAM_CHANGE_MESSAGE = "Set your team with %sb %sr" % (USER_PREFIX, USER_PREFIX)
if MAX_TEAMS >= 3:
	TEAM_CHANGE_MESSAGE += " %sg" % (USER_PREFIX)
if MAX_TEAMS >= 4:
	TEAM_CHANGE_MESSAGE += " %sy" % (USER_PREFIX)

SERVER_PORT = 23400 # On which port to host a server - better if it's different from the port in your game settings

RANKING = 1 # Should we allow !rank user command
