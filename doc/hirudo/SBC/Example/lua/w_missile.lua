-- Created by the LXE Script Basic Compiler
function initialize()
	me.class = weapons:missile
	me.recoil = 10
	me.rounds = 1
	me.recharge = 1
	me.rof = 400
	callbacks.realoding = "reloading"
	callbacks.onfire = "onfire"
	callbacks.oninput = "oninput"
end
function reloading()
	reloadtime = reloadtime + lx.deltatime
	if reloadtime > 1 then
		sample("Missile Reload.ogg"):play()
		me.ammo = me.ammo + 1
		reloadtime = 0
	end
end
function onfire()
	if me.ammo > 0 then
		sample("missile.wav"):play()
		me:spawn("p_missile.lua", 1, 0, 0, me.owner.angle, 250, 0)
		me.ammo = me.ammo - 1
		if me.inputfocus == false then
			me:focusinput()
		end
	end
end
function oninput(input)
	if input.movement.x != 0 | input.movement.y != 0 then
		for i = 0, i > me.projectiles.count, i++
			me.projectiles.get(i):move(input.movement)
		end
	end
end
