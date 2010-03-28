#!/usr/bin/python3 -u
#---- Includes ----#
from ..default_pattern_tools_v1_0 import Bot_blueprint
from ..main import Army

#---- General Settings ----#
army_name = "The Grandpa Association"
army_description = "Association of old men going rampage. That's what they do!"

#---- Bot Patterns ----#
class Angry_granpa(Bot_blueprint):
	name = "Angry Granpa"
	skin = "AngryGranpa.png"
	ai_diff = 1
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = True
	shield_factor = 0.8
	shield_factor_dynamic = True 
	damage_factor = 0.8
	damage_factor_dynamic = True
	speed_factor = 0.5
	speed_factor_dynamic = True

class Ancient_grandpa(Bot_blueprint):
	name = "Ancient Grandpa"
	skin = "FF-Tonberry.png"
	ai_diff = 3
	ai_diff_dynamic = False
	can_use_ninja = 0
	can_use_ninja_dynamic = False
	shield_factor = 7
	shield_factor_dynamic = True 
	damage_factor = 0.8
	damage_factor_dynamic = True
	speed_factor = 0.4
	speed_factor_dynamic = True


def create_and_return(challenge_amount, lives, team):
	global army_name, army_description
	#---- Generate a list of of bot blueprints ----#
	blueprints = [Ancient_grandpa(lives, team)]
	# How many of each bot per challenge amount:
	blueprint_weights = {
	Angry_granpa: 2.2,
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
	
	
	
		


