function effects.init()
	local gibs = {}
		gib = load_particle("gibs/gib.obj")
		brain = load_particle("gibs/brain.obj")
		rib = load_particle("gibs/rib.obj")
		gib2 = load_particle("gibs/gib2.obj")
		skull = load_particle("gibs/skull.obj")
		
	local blood = load_particle("blood.obj")
	local healthPack = load_particle("health/ahealth.obj")
	local bloodScreenTimer = 0
	ch_bloodscreen = 1
	cg_gibs = 10
	
	sv_healthpacks = 0
	sv_healthpacks_delay = 60
	healthDelay = 0
	nextHealthSpawn = randomint(3000, sv_healthpacks_delay*100)

	console_register_command("CH_BLOODSCREEN",function(i)
		if i == nil then
			return "CH_BLOODSCREEN IS: "..ch_bloodscreen.." DEFAULT: 1"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 1 and i >= 0 then
				ch_bloodscreen = i
			else
				return "CH_BLOODSCREEN IS: "..ch_bloodscreen.." DEFAULT: 1"
			end
		end
	end)
	
	console_register_command("SV_HEALTHPACKS",function(i)
		if i == nil then
			return "SV_HEALTHPACKS: "..sv_healthpacks.." DEFAULT: 0"
		elseif i ~= nil then
			local i = i *1
			
			if tonumber(i) and i <= 1 and i >= 0 and AUTH then
				sv_healthpacks = i
			else
				return "SV_HEALTHPACKS IS: "..sv_healthpacks.." DEFAULT: 0"
			end
		end
	end)
	
	console_register_command("SV_HEALTHPACKS_DELAY",function(i)
		if i == nil then
			return "SV_HEALTHPACKS_DELAY IS: "..sv_healthpacks_delay.." DEFAULT: 60"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i >= 0 and AUTH then
				sv_healthpacks_delay = i
			else
				return "SV_HEALTHPACKS_DELAY IS: "..sv_healthpacks_delay.." DEFAULT: 60"
			end
		end
	end)
	
	console_register_command("CG_GIBS",function(i)
		if i == nil then
			return "CG_GIBS IS: "..cg_gibs.." DEFAULT: 6"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i >= 0 then
				cg_gibs = i
			else
				return "CG_GIBS IS: "..ch_gibs.." DEFAULT: 6"
			end
		end
	end)

	function bindings.viewportRender(viewport, worm)
		local bitmap = viewport:bitmap()
		local p = worm:player()

		if ch_bloodscreen == 1 and cg_draw2d == 1 then
			if worm:health() <= 0 and not isSpectator(worm:player())  and p:deaths() > 0 then
				if bloodScreenTimer < 100 then
					bloodScreenTimer = bloodScreenTimer + 0.5
				end
				gfx_set_alphach(bloodScreenTimer)
					bitmap:draw_box(0, 0, 320, 240, color(215,0,0))
				gfx_reset_blending()
			else
				bloodScreenTimer = 0
			end
		end
	end

	function bindings.playerInit(p)
		local d = p:data()
		
		d.respawn = false
	end

	function bindings.playerUpdate(player)
		local w = player:worm()
		local d = player:data()

		if (w:health() - randomint(1,350)) < -307 and w:health() > 0 then
			w:shoot(blood,1,0.1,0.1,1,0,360,0,0)
		end
	
		if d.respawn == false then
			if w then
				if w:health() > 0 then
					d.respawn = true
					effects.spawn(player)
				end
			end
		else
			if w then
				if w:health() <= 0 then
					d.respawn = false
				end
			else
				d.respawn = false
			end
		end
		
		if (sv_healthpacks == 1) then
			if (AUTH or DEDSERV) then
				healthDelay = healthDelay + 1
			end
			
			if (AUTH or DEDSERV) and healthDelay == nextHealthSpawn and gameMode ~= 5 then
				local hX, hY
				repeat
					hX = randomint(1, 2000)
					hY = randomint(1, 2000)
				until map_is_particle_pass(hX, hY)
				healthPack:put(hX, hY)
				healthDelay = 0
				nextHealthSpawn = randomint(3000, sv_healthpacks_delay*100)
			end
		end
	end

	local spawnPlayer = load_particle("respawn.obj")

	function effects.spawn(player)
		player:worm():shoot(spawnPlayer,1,0,0,0,0,0,0,0)
		player:data().doSuicide = false
	end

	function effects.follow(object)
		object:set_pos(object:player():worm():pos())
		object:set_spd(object:player():worm():spd())
	end
	
	function effects.healthCheck(object, worm)
	
	end
	
	function effects.healthRun(object, worm)
		if worm:health() <= 95 then
			worm:damage(-30, worm:player())
		else 
			worm:damage(-(125-worm:health()), worm:player())
		end
	end
	
	function effects.mega_health(object, worm)
		worm:damage(-(100-worm:health()), worm:player())
	end
	
	local megaHealth = load_particle("health/megahealth.obj")
	
	function put_health(x, y)
		--megaHealth:put(x, y)
	end
	
	--[[function effects.createGibs(object)
		for i = 0, cg_gibs do
			local _gib = randomint(1, 5)
			
			if _gib == 1 then
				object:shoot(gib,1,1.2,0.3,0.5,0,90,0,0)
			end
			if _gib == 2 then
				object:shoot(brain,1,1.2,0.3,0.5,0,90,0,0)
			end
			if _gib == 3 then
				object:shoot(rib,1,1.2,0.3,0.5,0,90,0,0)
			end
			if _gib == 4 then
				object:shoot(gib2,1,1.2,0.3,0.5,0,90,0,0)
			end
			if _gib == 5 then
				object:shoot(skull,1,1.2,0.3,0.5,0,90,0,0)
			end
		end
	end]]--
	
	function effects.createGibs(object)
		object:shoot(gib,math.ceil(cg_gibs/5),0.45,0.9,0.5,0,360,0,0)
		object:shoot(brain,math.ceil(cg_gibs/5),0.45,0.9,0.5,0,360,0,0)
		object:shoot(rib,math.ceil(cg_gibs/5),0.45,0.9,0.5,0,360,0,0)
		object:shoot(gib2,math.ceil(cg_gibs/5),0.45,0.9,0.5,0,360,0,0)
		object:shoot(skull,math.ceil(cg_gibs/5),0.45,0.9,0.5,0,360,0,0)
	end

end
