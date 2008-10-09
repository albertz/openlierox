-- Created by the LXE Script Basic Compiler
function initialize()
	me.style = projectiles.pixel
	me.pixel.colour1 = colour(176,176,176)
	me.trail = trails:none
	-- me.gravity(100)
	-- me.dampening(1)
	callbacks.onhit = "onhit"
	callbacks.onplayerhit = "onplayerhit"
end
function onhit()
	me:explode(2, 1)
	sample("exp3a.wav"):play()
end
function onplayerhit(player)
	player:injure(2)
end
