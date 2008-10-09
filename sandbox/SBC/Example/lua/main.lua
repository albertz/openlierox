-- Created by the LXE Script Basic Compiler
include("Common.lua")
function initialize(player)
	weapons.add("Missile", "w_missile.txt")
	weapons.add("Laser", "w_laser.txt")
	weapons.add("Test", "w_test.txt")
	rope.length = 300
	rope.restlength = 27
	rope.restlength = 4.5
	player.anglespeed = 150
	player.groundspeed = 8
	player.airspeed = 4
	player.gravity = 100
	player.jumpforce = -65
	player.airfriction = 0
	player.groundfriction = 0.4
end
