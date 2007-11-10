-- Created by the LXE Script Basic Compiler
function initialize()
	me.style = projectiles.pixel
	me.pixel.colour1 = colour(123,123,123)
	me.trail = trails:smoke
	-- me.gravity(100)
	-- me.dampening(1)
	callbacks.onhit = "onhit"
	callbacks.onplayerhit = "onplayerhit"
end
function onhit()
	me:explode(4, 2)
	sample("exp2.ogg"):play()
	me:spawn("p_greybits.lua", 10, 0, 0, 0, 50, 360)
	me:spawn("p_greybits.txt", 10, 0, 0, 0, 100, 360)
end
function onplayerhit(player)
	player:injure(4)
	me:spawn("p_greybits.txt", 10, 0, 0, 0, 150, 360)
	me:spawn("p_greybits.txt", 10, 0, 0, 0, 200, 360)
end
