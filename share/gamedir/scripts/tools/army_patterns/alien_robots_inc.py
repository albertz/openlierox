#!/usr/bin/python3 -u
#---- Includes ----#
from ..army_architect import Bot_blueprint, Army_blueprint

#---- General Settings ----#
army_name = "Alien Robots Inc."
army_description = "It seems you will be participating in a test of new war robots..."

#---- Bot Patterns ----#
class Lightspeed_annihilator(Bot_blueprint):
	name = "Lightspeed Annihilator"
	skin = "drone.png"
	ai_diff = 2
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = False
	shield_factor = 0.6
	shield_factor_dynamic = True 
	damage_factor = 1
	damage_factor_dynamic = True
	speed_factor = 2
	speed_factor_dynamic = True

class Infantry_XZ32(Bot_blueprint):
	name = "Infantry XZ32"
	skin = "dynaA.png"
	ai_diff = 1
	ai_diff_dynamic = True
	can_use_ninja = 0
	can_use_ninja_dynamic = False
	shield_factor = 2
	shield_factor_dynamic = True 
	damage_factor = 0.8
	damage_factor_dynamic = True
	speed_factor = 0.7
	speed_factor_dynamic = True

class Heavy_mech_walker(Bot_blueprint):
	name = "Heavy Mech Walker"
	skin = "Mech.png"
	ai_diff = 3
	ai_diff_dynamic = True
	can_use_ninja = 0
	can_use_ninja_dynamic = False
	shield_factor = 4
	shield_factor_dynamic = True 
	damage_factor = 1.4
	damage_factor_dynamic = True
	speed_factor = 0.5
	speed_factor_dynamic = True
	
class Shell_droid(Bot_blueprint):
	name = "Shell Droid"
	skin = "MM-Metool.png"
	ai_diff = 1
	ai_diff_dynamic = True
	can_use_ninja = 0
	can_use_ninja_dynamic = False
	shield_factor = 6
	shield_factor_dynamic = True 
	damage_factor = 1
	damage_factor_dynamic = True
	speed_factor = 1
	speed_factor_dynamic = True


def generate_blueprint(challenge_amount, lives, team):
	global army_name, army_description
	#---- Generate a list of of bot blueprints ----#
	blueprints = []
	# How many of each bot per challenge amount:
	blueprint_weights = {
	Lightspeed_annihilator: 0.5,
	Infantry_XZ32: 1,
	Heavy_mech_walker: 0.3,
	Shell_droid: 0.3,
	}
	
	for blueprint, wieght in blueprint_weights.items():
		num_bots = round(wieght * challenge_amount)
		for x in range(num_bots):
			blueprints.append(blueprint(lives, team))
	
	#---- Scale the blueprints ----#
	# Sum the current default_challenge_amount
	default_challenge_amount_sum = 0
	for blueprint in blueprints:
		default_challenge_amount_sum += blueprint.default_challenge_amount
	
	scale_factor = challenge_amount / default_challenge_amount_sum

	for blueprint in blueprints:
		blueprint.scale_challenge_amount_with(scale_factor)
	
	#---- Return the army ----#	
	return Army_blueprint(army_name, army_description, blueprints)
	
	
	
		


