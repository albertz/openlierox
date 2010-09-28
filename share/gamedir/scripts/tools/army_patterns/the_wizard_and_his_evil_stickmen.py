#!/usr/bin/python3 -u
#---- Includes ----#
from ..army_architect import Bot_blueprint, Army_blueprint

#---- General Settings ----#
army_name = "The Wizard and his Evil Stickmen"
army_description = "Using awsome dark magic, The Wizard spawned an army of Evil Stickmen from one of Freud's childhood paintings."

#---- Bot Patterns ----#
class The_wizard(Bot_blueprint):
	name = "The Wizard"
	skin = "Wizard.png"
	ai_diff = 3
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = False
	shield_factor = 3
	shield_factor_dynamic = True 
	damage_factor = 1.5
	damage_factor_dynamic = True
	speed_factor = 1
	speed_factor_dynamic = True

class Fragile_stick_man(Bot_blueprint):
	name = "Fragile Stick Man"
	skin = "danceman.png"
	ai_diff = 1
	ai_diff_dynamic = True
	can_use_ninja = 0
	can_use_ninja_dynamic = False
	shield_factor = 0.2
	shield_factor_dynamic = True 
	damage_factor = 0.5
	damage_factor_dynamic = True
	speed_factor = 0.7
	speed_factor_dynamic = True

class Staring_stick_man(Bot_blueprint):
	name = "Staring Stick Man"
	skin = "Eyestabber.png"
	ai_diff = 1
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = False
	shield_factor = 0.6
	shield_factor_dynamic = True
	damage_factor = 0.8
	damage_factor_dynamic = True
	speed_factor = 0.8
	speed_factor_dynamic = True
	
class Massive_stick_man(Bot_blueprint):
	name = "Massive Stick Man"
	skin = "AimMan.png"
	ai_diff = 2
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = False
	shield_factor = 4
	shield_factor_dynamic = True
	damage_factor = 1
	damage_factor_dynamic = True
	speed_factor = 0.5
	speed_factor_dynamic = True


def generate_blueprint(challenge_amount, lives, team):
	global army_name, army_description
	#---- Generate a list of of bot blueprints ----#
	blueprints = [The_wizard(lives, team)]
	# How many of each bot per challenge amount:
	blueprint_weights = {
	Fragile_stick_man: 2,
	Staring_stick_man: 1,
	Massive_stick_man: 0.5,
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
	
	
	
		


