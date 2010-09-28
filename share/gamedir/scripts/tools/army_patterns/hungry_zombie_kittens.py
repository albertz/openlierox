#!/usr/bin/python3 -u
#---- Includes ----#
from ..army_architect import Bot_blueprint, Army_blueprint

#---- General Settings ----#
army_name = "The Hungry Zombie Kittens"
army_description = "Previously these kittens ate cat food. But now they wan't to eat your freakin' soul! (And your body to of course, after they ripped it asunder ;,,,;)"

#---- Bot Patterns ----#
class Zombie_kitten(Bot_blueprint):
	name = "Zombie Kitten"
	skin = "HK-KittyZombie.png"
	ai_diff = 0
	ai_diff_dynamic = True
	can_use_ninja = 0
	can_use_ninja_dynamic = False
	shield_factor = 0.6
	shield_factor_dynamic = True 
	damage_factor = 1
	damage_factor_dynamic = False
	speed_factor = 0.6
	speed_factor_dynamic = False

class Franken_kitten(Bot_blueprint):
	name = "Franken Kitten"
	skin = "HK-FrankenKitty.png"
	ai_diff = 0
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = True
	shield_factor = 0.6
	shield_factor_dynamic = False 
	damage_factor = 1
	damage_factor_dynamic = False
	speed_factor = 0.8
	speed_factor_dynamic = True

class Ghoul_kitten(Bot_blueprint):
	name = "Ghoul Kitten"
	skin = "HK-KittyMime.png"
	ai_diff = 2
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = True
	shield_factor = 0.5
	shield_factor_dynamic = True
	damage_factor = 1.0
	damage_factor_dynamic = False
	speed_factor = 1.6
	speed_factor_dynamic = True


def generate_blueprint(challenge_amount, lives, team):
	global army_name, army_description
	#---- Generate a list of of bot blueprints ----#
	blueprints = []
	# How many of each bot per challenge amount:
	blueprint_weights = {
	Zombie_kitten: 2,
	Franken_kitten: 1,
	Ghoul_kitten: 0.5,
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
	
	
	
		


