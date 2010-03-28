#!/usr/bin/python3 -u
#                                  ______         _                    
#     /\                          |  ____|       | |                   
#    /  \   _ __ _ __ ___  _   _  | |__ __ _  ___| |_  ___  _ __ _   _ 
#   / /\ \ | '__| '_ ` _ \| | | | |  __/ _` |/ __| __|/ _ \| '__| | | |
#  / ____ \| |  | | | | | | |_| | | | | (_| | (__| |_| (_) | |  | |_| |
# /_/    \_\_|  |_| |_| |_|\__, | |_|  \__,_|\___|\__|\___/|_|   \__, |
#                           __/ |                                 __/ |
#                          |___/                                 |___/ 
#
# Army Factory is a python module for generating armies of bots for you dedicated server.

#========= Imports ========================================
import tools.commands as cmd

def get_army_pattern_list():
	pass

def produce(army_pattern, challenge_amount, lives = 0, team = 0):
	"""This function produces an army from the requested army pattern and throws the bots into the game."""
	_temp = __import__('army_patterns.'+army_pattern, globals(), locals(), ['create_and_return'], -1)
	create_and_return = _temp.create_and_return
	return create_and_return(challenge_amount, lives, team)
	
class Army:
	"""The army class contains id's of the bots in the army. It also contains some useful methods to handle the army as a whole."""
	def __init__(self, bot_ids=[]):
		self.ids = bot_ids
	
	def kick(self):
		"""Kicks all the bots in the army out of the game"""
		for id in self.ids:
			cmd.kickWorm(id)