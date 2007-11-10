-- Created by the LXE Script Basic Compiler
function initialize()
	me.style = projectiles.image
	me.image.source = surface("missile.png", true)
	me.image.angles = -1
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
end
function onplayerhit(player)
	player:injure(8)
end
