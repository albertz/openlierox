#!/usr/bin/python3 -u
#   _____                                           _     
#  / ____|                                         | |    
# | |      ___  _ __ ___  _ __ ___   __ _ _ __   __| |___ 
# | |     / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` / __|
# | |____| (_) | | | | | | | | | | | (_| | | | | (_| \__ \
#  \_____|\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|___/
#                                                         
# This module provides a basic interface to execute OpenLieroX commands.
# The OpenLieroX program and your dedicated script communicates via stdin/stdout
# in a string format. This module transforms your input to the right format and
# Transforms the return values into a LIST of strings
#
# Example usage:
# import commands as cmd
# cmd.setWormCanAirJump(0, True) #Worm with ID 0 can air jump now :D
# current_game_state = cmd.getGameState()[0] # current_game_state could for example contain "S_SVRPLAYING" or "S_SVRWEAPONS" now.
# #note the [0] at the end. We take the first item from the returned list (which in this case contains only one item)

#========= Imports ========================================
import sys # used to read from stdin
import functools # functools.partial is used to "create" the majority of the commandfunctions. Please read more here: http://docs.python.org/py3k/library/functools.html
import os, inspect # used for writing a file

#========= Functions Definitions ==========================
# These functions are somewhat module internal and are not "meant" to be used from 
# outside the module. Please, scroll down a bit and check out the part where we "Load the Commands".

def read_stdin_line():
	"""Reads and returns one line from stdin."""
	ret = sys.stdin.readline().strip()
	if ret != "": 
		return ret
	else:
		sys.stderr.write("OpenLieroX Dedicated Server Script: OpenLieroX terminated, exiting\n")
		sys.exit(1)

def read_stdin_response_list():
	"""Returns a list with the response values OpenLieroX has written to us."""
	ret = []
	resp = read_stdin_line()
	while resp != ".":
		if not resp.startswith(':'):
			sys.stderr.write("OpenLieroX Dedicated Server Script: bad OLX dedicated response: " + resp + "\n")
		else:
			ret.append( resp[1:] )
		resp = read_stdin_line()
	return ret

def execute_command_string(command_string):
	"""Prints the command to stdout and returns the response."""
	print(command_string)
	return read_stdin_response_list()
	
	
def command_base_executor(*args, **kwargs):
	"""This function builds a command string and executes it as a command.
Example usage: command_base_executor(0, -2, name="setWormLives")
This would give the worm with id 0 unlimited lives (-2)
"""
	# Check for usage errors:
	if "name" not in kwargs:
		raise Exception('No command name was specified for command wrapper')
	
	# Now build the command string
	command_string = kwargs["name"]
	for parameter in args:
		command_string += (" \"%s\"" % parameter)
	
	# Execute the command and return the list of return values.
	return execute_command_string(command_string)

	
#========= "Load the Commands" ===========================
# Here we load the available commands and create corresponding functions.
# We also write the list of commands to the file available_commands.txt in the same folder as this file.
# Take a look there to see which commands are available for use.

longhelp_lines = execute_command_string("longhelp")
command_names = [line.partition(" ")[0] for line in longhelp_lines]

# Create available_commands.txt
custom_help_lines = []
for command_name in command_names:
	help_lines = execute_command_string("help "+command_name)
	custom_help_lines.append(help_lines[0] + " ===== " + help_lines[1])
custom_help_lines.append("nextSignal ===== This command will be very important for you when building your dedicated script. The signal OpenLieroX returns will be an ingame event, like 'backtolobby' or 'lobbystarted'. Read more here: http://www.openlierox.net/wiki/index.php/Dedicated_script")
file = open(os.path.dirname(inspect.getfile( inspect.currentframe() )) + '/available_commands.txt',"w")
file.writelines([line + "\n" for line in custom_help_lines])
file.close()

command_names.append("nextSignal")

# Now we "create" functions using functools.partial
for command_name in command_names:
	globals()[command_name] = functools.partial(command_base_executor, name=command_name)