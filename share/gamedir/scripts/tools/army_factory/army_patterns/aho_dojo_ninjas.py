#!/usr/bin/python3 -u
#---- Includes ----#
from ..default_pattern_tools_v1_0 import Bot_blueprint
from ..main import Army

#---- General Settings ----#
army_name = "A-Ho Dojo Ninjas"
army_description = "A-Ho is japanese for ''idiot''. Sensei A-Ho tried to tie his own shoes but failed and hit is face 73 times by mistake instead."

#---- Bot Patterns ----#
class Sensei_aho(Bot_blueprint):
	name = "Sensei A-Ho"
	skin = "yodatarodarka.png"
	ai_diff = 3
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = False
	shield_factor = 3
	shield_factor_dynamic = True 
	damage_factor = 1
	damage_factor_dynamic = False
	speed_factor = 1.5
	speed_factor_dynamic = True

class Heavy_retard_ninja(Bot_blueprint):
	name = "Heavy Retard Ninja"
	skin = "NinjaTaroDarkB.png"
	ai_diff = 0
	ai_diff_dynamic = True
	can_use_ninja = 0
	can_use_ninja_dynamic = False
	shield_factor = 1
	shield_factor_dynamic = True 
	damage_factor = 1
	damage_factor_dynamic = True
	speed_factor = 0.2
	speed_factor_dynamic = False

class Tiny_worthles_ninja(Bot_blueprint):
	name = "Tiny Worthles Ninja"
	skin = "NinjaTaroDarkB.png"
	ai_diff = 1
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = False
	shield_factor = 0.15
	shield_factor_dynamic = True
	damage_factor = 0.3
	damage_factor_dynamic = True
	speed_factor = 1
	speed_factor_dynamic = False


def create_and_return(challenge_amount, lives, team):
	global army_name, army_description
	#---- Generate a list of of bot blueprints ----#
	blueprints = [Sensei_aho(lives, team)]
	# How many of each bot per challenge amount:
	blueprint_weights = {
	Heavy_retard_ninja: 0.5,
	Tiny_worthles_ninja: 3,
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
	
	#---- Produce the army----#
	army = Army()
	army.name = army_name
	army.description = army_description
	for blueprint in blueprints:
		army.ids.append(blueprint.produce())
		
	return army
	
	
	
		


