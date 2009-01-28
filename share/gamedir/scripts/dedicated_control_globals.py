#!/usr/bin/python -u
# Dedicated Control handler script for OpenLieroX
# (http://openlierox.sourceforge.net)

global availablePresets, nextPresets, availableLevels, availableMods, worms, bots, scriptPaused, controlHandler
global adminCommandHelp_Preset, parseAdminCommand_Preset, userCommandHelp_Preset, parseUserCommand_Preset
# Global functions
global selectNextPreset

# Preset stuffies
availablePresets = []
nextPresets = []

availableLevels = list()
availableMods = list()

worms = {} # List of all worms on the server
# Bots don't need to be itterated with the other ones.
bots = {}  # Dictionary of all possible bots

# Function that controls ded server behavior
controlHandler = None

scriptPaused = False

adminCommandHelp_Preset = None
parseAdminCommand_Preset = None
userCommandHelp_Preset = None
parseUserCommand_Preset = None

selectNextPreset = None

