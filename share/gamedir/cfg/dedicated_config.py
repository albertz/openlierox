#!/usr/bin/python -u

# The reason this script resides in cfg/ folder is that the OLX file downloader will 
# deny access to cfg/ folder, and we have admin password here

ADMIN_PASSWORD = "secret" # Change it!

SERVER_NAME = "Dedicated server" # The name of your server, can be changed by preset
WELCOME_MESSAGE = "<player> from <country> connected - typing go/spamming is useless"
LOCAL_BOT_NAME = "[CPU] Kamikazee!" # The name of a bot in server, make sure such bot exist in your OLX
KILL_LOCAL_BOT = True # Kill local bot when game starts

MIN_PLAYERS = 3 # including bot worm
MAX_PLAYERS = 8 # Including bot worm
MIN_PLAYERS_TEAMS = 5 # Players will be split in two teams automatically if there is enough players (including bot worm)
TOO_FEW_PLAYERS_MESSAGE = "Game will start with %i players"

WAIT_AFTER_GAME = 20 # Seconds to wait in lobby after round finished
WAIT_BEFORE_GAME = 15 # Seconds to wait in lobby before next round, will give some message
WAIT_BEFORE_GAME_MESSAGE = "Game will start in %i seconds"

GAME_LIVES = 10
GAME_MAX_KILLS = 20
GAME_MAX_TIME = 10 # In minutes
WEAPON_SELECTION_TIME = 40 # In seconds

