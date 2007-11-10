-- Created by the LXE Script Basic Compiler
function initialize()
	me.class = weapons:laser
	me.recoil = 5
	me.rounds = 34
	me.recharge = 6
	me.rof = 1
	callbacks.realoding = "reloading"
	callbacks.onfire = "onfire"
	callbacks.oninput = "oninput"
end
function reloading()
	reloadtime = reloadtime + lx.deltatime
	if reloadtime > 1 then
		sample("Reload.ogg"):play()
		me.ammo = me.ammo + 1
		reloadtime = 0
	end
end
function onfire()
	if me.ammo > 0 then
		me:spawnbeam(0, 0, me.owner.angle, colour(255,0,0), 999999, 1)
		me.ammo = me.ammo - 1
	end
end
