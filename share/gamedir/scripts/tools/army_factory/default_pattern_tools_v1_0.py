#!/usr/bin/python3 -u
#  _____        __            _ _     _____       _   _                     _______          _     
# |  __ \      / _|          | | |   |  __ \     | | | |                   |__   __|        | |    
# | |  | | ___| |_ __ _ _   _| | |_  | |__) |__ _| |_| |_  ___ _ __ _ __      | | ___   ___ | |___ 
# | |  | |/ _ \  _/ _` | | | | | __| |  ___// _` | __| __|/ _ \ '__| '_ \     | |/ _ \ / _ \| / __|
# | |__| |  __/ || (_| | |_| | | |_  | |   | (_| | |_| |_|  __/ |  | | | |    | | (_) | (_) | \__ \
# |_____/ \___|_| \__,_|\__,_|_|\__| |_|    \__,_|\__|\__|\___|_|  |_| |_|    |_|\___/ \___/|_|___/
#                                                                                                  
# This is the default tools used to create an army pattern.
# The main idea is that an enemy's "challange amount" is calculated by a function, and that this function is used to balance gameplay.
# Most important is the class Bot_blueprint.

#========= Imports ========================================
from .. import commands as cmd

class Bot_blueprint:
	"""The Bot_blueprint class describes a bot. It will be used to create the bot."""
	name = "Untitled" # This will be the name of the bot like for example "[CPU] Dummi"
	skin = "Reaperman.png" # The file ending should be included.
	color_r = 128 # [0 , 255]
	color_g = 128 # [0 , 255]
	color_b = 128 # [0 , 255]
	team = 0 # 0 = blue, 1 = red, 2 = green, 3 = yellow
	lives = 0 # Zero lives means that the bot is out after first death. (No backup lives)
	ai_diff = 1 # 0 = Easy, 1 = Medium, 2 = Hard, 3 = Xtreme
	ai_diff_dynamic = True # Scale this when scaling characters overall challange amount?
	can_use_ninja = 1 # 0 = No, 1 = Yes
	can_use_ninja_dynamic = False # Scale this when scaling characters overall challange amount?
	shield_factor = 1
	shield_factor_dynamic = True # Scale this when scaling characters overall challange amount?
	damage_factor = 1
	damage_factor_dynamic = False # Scale this when scaling characters overall challange amount?
	speed_factor = 1
	speed_factor_dynamic = False # Scale this when scaling characters overall challange amount?
	
	def __init__(self, lives = 0, team = 0):
		self.lives = lives
		self.team = team
		self.default_challenge_amount = self.get_current_challenge_amount() #Usefull if you would like to reset the blueprint values
	
	def get_current_challenge_amount(self):
		return calculate_challenge_amount(self.ai_diff, self.can_use_ninja, self.shield_factor, self.damage_factor, self.speed_factor)
	
	def scale_challenge_amount_to(self, new_challange_amount):
		"""This method scales the blueprint to a certain challange amount."""
		# Create scaled new_values
		original_values = [ai_diff_to_factor(self.ai_diff), can_use_ninja_to_factor(self.can_use_ninja), self.shield_factor, self.damage_factor, self.speed_factor]
		is_dynamic = [self.ai_diff_dynamic, self.can_use_ninja_dynamic, self.shield_factor_dynamic, self.damage_factor_dynamic, self.speed_factor_dynamic]
		new_values = scale_vector_parts_to_certain_product(original_values, is_dynamic, new_challange_amount)
		
		#ai_diff and can_use_ninja must be rounded correctly. Do that and then lock them.
		new_values[0] = ai_diff_to_factor(factor_to_ai_diff(new_values[0]))
		is_dynamic[0] = False
		new_values[1] = can_use_ninja_to_factor(factor_to_can_use_ninja(new_values[1]))
		is_dynamic[1] = False
		
		# Now scale again because we changed the challenge amount when we rounded ai_diff and can_use_ninja
		new_values = scale_vector_parts_to_certain_product(original_values, is_dynamic, new_challange_amount)
		
		# Apply the new scaled values
		self.ai_diff = factor_to_ai_diff(new_values[0])
		self.can_use_ninja = factor_to_can_use_ninja(new_values[1])
		self.shield_factor = new_values[2]
		self.damage_factor = new_values[3]
		self.speed_factor = new_values[4]
		
	def scale_challenge_amount_with(self, factor):
		"""This method scales the blueprint's challange amount with a factor."""
		self.scale_challenge_amount_to( self.get_current_challenge_amount() * float(factor) )
	
	def produce(self):
		"""This method creates a bot from the blueprint and throws it into the game. The id of the bot is returned. Note that a bot can not be successfully created in lobby."""
		cmd_return = cmd.addBots(1, self.ai_diff)
		if len(cmd_return) == 0:
			return None #Opps! We failed. Most truly we are adding to many bots!
		id = cmd_return[0]
		cmd.setWormName(id, self.name, False)
		cmd.setWormColor(id, self.color_r, self.color_g, self.color_b)
		cmd.setWormSkin(id, self.skin)
		cmd.setWormCanUseNinja(id, self.can_use_ninja)
		cmd.setWormDamageFactor(id, self.damage_factor)
		cmd.setWormShieldFactor(id, self.shield_factor)
		cmd.setWormSpeedFactor(id, self.speed_factor)
		cmd.setWormTeam(id, self.team)
		cmd.setWormLives(id, self.lives) # Note that this command will only work if we are in a game.
		
		return id
	
def calculate_challenge_amount(ai_diff, can_use_ninja, shield_factor, damage_factor, speed_factor):
	return ai_diff_to_factor(ai_diff) * can_use_ninja_to_factor(can_use_ninja) * shield_factor * damage_factor * speed_factor

def ai_diff_to_factor(ai_diff):
	return 0.5 + float(ai_diff) / 2

def factor_to_ai_diff(factor):
	"""This function MUST be the inverse to ai_diff_to_factor"""
	ret_val = int(round((float(factor) - 0.5) * 2))
	if ret_val < 0:
		ret_val = 0
	if ret_val > 3:
		ret_val = 3	
	return ret_val

def can_use_ninja_to_factor(can_use_ninja):
	return 0.5 + float(can_use_ninja) / 2

def factor_to_can_use_ninja(factor):
	"""This function MUST be the inverse to can_use_ninja_to_factor"""
	ret_val = int(round((float(factor) - 0.5) * 2))
	if ret_val < 0:
		ret_val = 0
	if ret_val > 1:
		ret_val = 1	
	return ret_val
	
def scale_vector_parts_to_certain_product(values, dynamic, goal_result):
	"""Warning: This is pure epic logic/magic
Our Goal: Find a list, named return_list, such that return_list[0]*return_list[1]*return_list[2]*...*return_list[-1] = goal_result
That means the product of all entries in the list is our goal_result.
Other criterias are that if for any 'p', dynamic[p] == False, then return_list[p] = values[p].
And if for any 'p', dynamic[p] == True, then return_list[p] = k*values[p]
for a specific k. To find this k is sortof our mathematical problem.
"""
	if len(values) != len(dynamic):
		raise Exception("Values and Dynamic must be the same size!" )
	
	return_list = []
	
	num_dynamic_variables = dynamic.count(True)
	
	if num_dynamic_variables == 0:
		return values #We can not change anything, and by returning here we avid zero division later.
	
	
	values_product = values[0]
	for value in values[1:]:
		values_product *= value
	
	k = (float(goal_result) / values_product) ** (1.0 / float(num_dynamic_variables))
	
	
	
	for p in range(len(values)):
		if dynamic[p] == False:
			return_list.append(values[p])
		else:
			return_list.append(k * float(values[p]))
			
	return return_list