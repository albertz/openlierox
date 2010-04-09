#!/usr/bin/python3 -u
#---- Includes ----#
from ..default_pattern_tools_v1_0 import Bot_blueprint
from ..main import Army
from ... import commands as cmd
import random

#---- General Settings ----#
army_name = "Horde of Raging Bunnies"
army_description = "This horde of raging bunnies looks somewhat dangerous."

bunny_names = ["Adrian", "Elaine", "Gwendolen", "Jeremy", "Lionel", "Nicholas", "Alf", "Virginia", "Daphne", "Arthur", "John", "Andrew", "Bernard", "Timothy", "Donald", "George", "Beryl", "Gerald", "Brenda", "Reginald", "Sandra", "Trevor","Norman", "Jenny", "Charles", "Matthew", "Penny", "Maria", "Roger", "Judith", "Colin", "Bruce", "Jim", "Rosemary", "Kay", "Gregory", "David", "Evelyn", "Bruce", "Arthur", "Diana", "Wendy", "Denis", "Freda", "Ian","Barry", "Derek", "Frederick", "Dennis", "Mark", "Eileen", "Mary", "Jean", "Eve", "Dorothy", "Simon", "Neville", "Linda", "Margaret", "Sidney", "Eric", "Ernest", "Norman", "Valerie", "Diana", "Gay", "Frances", "Joan","Elizabeth", "Nick", "Steve", "Alan", "Giles", "Marjorie", "Daphne", "Paul", "Bronwen", "Rosamund", "Gillian", "Audrey", "William", "Carol", "Beryl", "Godfrey", "Grace", "Doreen", "Ivor", "Judith", "Owen", "Roderick","Graham", "Millicent", "Natalie", "Sheila", "Brian", "Taffy", "Heather", "Jock", "Lucy", "Noel", "Robert", "Stewart", "Irene", "Thomas", "Ben", "Dolores", "Geoffrey", "Iris", "Ivor", "Geraladine", "Jack", "Henry","Bert", "Rodney", "Joyce", "Bill", "Marian", "Kay", "Rupert", "Maureen", "Jock", "Irene", "Donald", "Jacqueline", "Pat", "Evan", "Julia", "Victor", "Marius", "Janice", "Pamela", "Julian", "Laurence", "Philip", "Nancy","Gilbert", "Shirley", "Nicholas", "Louise", "Rita", "Stephen", "Alexander", "Clive", "Ellen", "Malcolm", "Norman", "Rodney", "David", "Evelyn", "Frank", "Marilyn", "Keith", "Percy", "Ronald", "Roger", "Brian", "Mark","Edmund", "Karen", "James", "Deborah", "George", "Nigel", "Richard", "Stella", "Albert", "Clifford", "Elizabeth", "Oliver", "Pamela", "Veronica", "Pauline", "Gerry", "Brenda", "Pat", "June", "Mavis", "Kevin", "Grace","Douglas", "Peter", "Sally", "Leonard", "Janice", "Ralph", "Kim", "Raymond", "Lillian", "Jean", "Bill", "Tracey", "Phyllis", "Randolph", "Doreen", "Angus", "Monica", "Samuel", "Leslie", "Rex", "Allen", "Clare", "Eleanor","Harold", "Jill", "Robin", "Joe", "Terry", "Vera", "Lana", "Sean", "Robin", "Sylvia", "Angela", "Ernest", "Herbert", "Joseph", "Rod", "Stan", "Roy", "Sylvia", "Malcolm", "Thelma", "Roy", "Duncan", "June", "Cliff","Martin", "Yvonne", "Ruth", "Ronald", "Mandy", "Edna", "Cecil", "Edward", "Sarah", "Miriam", "Neil", "Douglas", "Olive", "Joyce", "Trevor", "Hilary", "Barbara", "Guy", "Vivien", "Ann", "Virginia", "Josephone", "Mary","Edwin", "Wendy", "Kenneth", "Winifred", "Janet", "Jane", "Margaret", "Olive", "Kathleen", "Winifred", "Stan", "Richard", "Wilfred", "Hugh", "Edith"]

#---- Bot Patterns ----#
class Bunny(Bot_blueprint):
	name = "Bunny default"
	skin = "Bunny.png"
	ai_diff = 0
	ai_diff_dynamic = False
	can_use_ninja = 0
	can_use_ninja_dynamic = False
	shield_factor = 0.3
	shield_factor_dynamic = False 
	damage_factor = 0.5
	damage_factor_dynamic = False
	speed_factor = 1
	speed_factor_dynamic = False
	
	def __init__(self, lives = 0, team = 0):
		super( Bunny, self ).__init__(lives, team)
		self.name = "Bunny ''" + random.choice(bunny_names) + "''"

def create_and_return(challenge_amount, lives, team):
	global army_name, army_description
	#---- Generate a list of of bot blueprints ----#
	blueprints = []
	# How many of each bot per challenge amount:
	blueprint_weights = {
	Bunny: 4,
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
	
	
	
		


