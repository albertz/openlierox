#!/usr/bin/python3 -u
#---- Includes ----#
from ..default_pattern_tools_v1_0 import Bot_blueprint
from ..main import Army

#---- General Settings ----#
army_name = "The Seven Dwarfs"
army_description = "Apposed to what Snow White beleives, the seven dwarfs go mental every now and then."

#---- Bot Patterns ----#
# Doc: The leader of the seven dwarfs, Doc wears glasses and often mixes up his words.
class Doc(Bot_blueprint):
	name = "Doc"
	skin = "BarryBurton.png"
	ai_diff = 3
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = False
	shield_factor = 1
	shield_factor_dynamic = True
	damage_factor = 1
	damage_factor_dynamic = False
	speed_factor = 1
	speed_factor_dynamic = False

#Grumpy: Grumpy initially disapproves of Snow White's presence in the dwarfs' home, but later warns her of the threat posed by the Queen and rushes to her aid upon realizing that she is in danger, leading the charge himself. He has the biggest nose of the dwarfs, and is frequently seen with one eye shut.
class Grumpy(Bot_blueprint):
	name = "Grumpy"
	skin = "X-Mas_Zelda.png"
	ai_diff = 1
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = False
	shield_factor = 1.2
	shield_factor_dynamic = True 
	damage_factor = 1
	damage_factor_dynamic = False
	speed_factor = 0.7
	speed_factor_dynamic = False

#Happy: Happy is the joyous dwarf and is usually portrayed laughing.
class Happy(Bot_blueprint):
	name = "Happy"
	skin = "smith.png"
	ai_diff = 1
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = False
	shield_factor = 2
	shield_factor_dynamic = True
	damage_factor = 0.5
	damage_factor_dynamic = True
	speed_factor = 4
	speed_factor_dynamic = True

#Sleepy: Sleepy is always tired and appears laconic in most situations. Sterling Holloway was also considered for the role.[citation needed]
class Sleepy(Bot_blueprint):
	name = "Sleepy"
	skin = "Link.png"
	ai_diff = 0
	ai_diff_dynamic = True
	can_use_ninja = 0
	can_use_ninja_dynamic = False
	shield_factor = 1.5
	shield_factor_dynamic = True
	damage_factor = 1.0
	damage_factor_dynamic = False
	speed_factor = 0.2
	speed_factor_dynamic = True
	
#Bashful: Bashful is the shyest of the dwarfs, and is often embarrassed by the presence of any attention directed at him.
class Bashful(Bot_blueprint):
	name = "Bashful"
	skin = "Zelda.png"
	ai_diff = 1
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = False
	shield_factor = 0.3
	shield_factor_dynamic = True
	damage_factor = 1.0
	damage_factor_dynamic = False
	speed_factor = 2
	speed_factor_dynamic = True

#Sneezy:[12] Sneezy's name is earned by his extraordinarily powerful sneezes (caused by hay fever), which are seen blowing even the heaviest of objects across a room.
class Sneezy(Bot_blueprint):
	name = "Sneezy"
	skin = "Kidwithagun.png"
	ai_diff = 0
	ai_diff_dynamic = True
	can_use_ninja = 1
	can_use_ninja_dynamic = False
	shield_factor = 0.5
	shield_factor_dynamic = True
	damage_factor = 1.5
	damage_factor_dynamic = True
	speed_factor = 1
	speed_factor_dynamic = False
	
#Dopey:[12] Dopey is the only dwarf that does not have a beard. He is clumsy and mute, with Happy explaining that he has simply "never tried to speak". Mel Blanc was briefly considered for the role.[citation needed]
class Dopey(Bot_blueprint):
	name = "Dopey"
	skin = "FredFlintstone.png"
	ai_diff = 1
	ai_diff_dynamic = True
	can_use_ninja = 0
	can_use_ninja_dynamic = True
	shield_factor = 0.5
	shield_factor_dynamic = True
	damage_factor = 1.0
	damage_factor_dynamic = False
	speed_factor = 1.6
	speed_factor_dynamic = True

def create_and_return(challenge_amount, lives, team):
	global army_name, army_description
	#---- Generate a list of of bot blueprints ----#
	blueprints = [Doc(lives, team), Grumpy(lives, team), Happy(lives, team), Sleepy(lives, team), Bashful(lives, team), Sneezy(lives, team), Dopey(lives, team)]
	
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
	
	
	
		


