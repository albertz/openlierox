-- Created by the LXE Script Basic Compiler
function initialize()
	me.class = weapons:missile
	me.recoil = 1
	me.rounds = 10
	me.recharge = 6
	me.rof = 400
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
		sample("throw.wav"):play()
		me:spawn("p_test.lua", 1, 0, 0, me.owner.angle, 250, 0)
		me.ammo = me.ammo - 1
	end
end
