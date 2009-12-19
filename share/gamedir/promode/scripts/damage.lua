function damage.init()

	bSnd = load_particle("berserkerSound.obj")
	pSnd = load_particle("porSound.obj")
	local blood = load_particle("blood_cloud_particle.obj")
	
	local damages = {}
		damages[0] = {}
		damages[1] = {}
		
	damages[0]["Lightning Gun"] = 0.46
	damages[0]["Blowtorch"] = 0.85
	damages[0]["Throwing Knife"] = 15
	damages[0]["Chaingun"] = 3.125
	damages[0]["Rifle"] = 22
	damages[0]["Flare Gun"] = 0
	damages[0]["Winchester"] = 11
	damages[0]["Flak Cannon"] = 4.5
	damages[0]["Bazooka"] = 15
	damages[0]["BFG"] = 10
	damages[0]["Shotgun"] = 2
	damages[0]["Mine"] = 0
	damages[0]["Mortart particle close"] = 10
	damages[0]["Mortart particle medium"] = 7.5
	damages[0]["Mortart particle far"] = 5
	damages[0]["Launcher particle close"] = 12.5
	damages[0]["Launcher particle medium"] = 15
	damages[0]["Autocannon explosion"] = 10.5
	damages[0]["Bazooka explosion"] = 10
	damages[0]["Gauss explosion"] = 32.5
	damages[0]["Doomsday explosion"] = 4
	damages[0]["Special explosion mortar"] = 60
	damages[0]["Special explosion grnlauncher"] = 40
	damages[0]["Special explosion grenade"] = 45
	damages[0]["Special explosion bfg"] = 30
	
	damages[1]["Lightning Gun"] = 0.99
	damages[1]["Blowtorch"] = 1.7
	damages[1]["Throwing Knife"] = 38
	damages[1]["Chaingun"] = 8
	damages[1]["Rifle"] = 44
	damages[1]["Flare Gun"] = 0
	damages[1]["Winchester"] = 30
	damages[1]["Flak Cannon"] = 7.5
	damages[1]["Bazooka"] = 30
	damages[1]["BFG"] = 20
	damages[1]["Shotgun"] = 4
	damages[1]["Mine"] = 0
	damages[1]["Mortart particle close"] = 20
	damages[1]["Mortart particle medium"] = 15
	damages[1]["Mortart particle far"] = 10
	damages[1]["Launcher particle close"] = 25
	damages[1]["Launcher particle medium"] = 30
	damages[1]["Autocannon explosion"] = 24
	damages[1]["Bazooka explosion"] = 20
	damages[1]["Gauss explosion"] = 55
	damages[1]["Doomsday explosion"] = 11
	damages[1]["Special explosion mortar"] = 120
	damages[1]["Special explosion grnlauncher"] = 80
	damages[1]["Special explosion grenade"] = 80
	damages[1]["Special explosion bfg"] = 60
	
	sv_pmc = 1
	
	console_register_command("SV_PMC",function(i)
		if i == nil then
			return "SV_PMC IS: "..sv_pmc.." DEFAULT: 1"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 1 and i >= 0 and AUTH then
				sv_pmc = i
			else
				return "SV_PMC IS: "..sv_pmc.." DEFAULT: 1"
			end
		end
	end)
	
	local pmc = network_game_event("pmc", function(self, data)
		local sv_pmc_ = data:get_int()
		
		if sv_pmc_ ~= sv_pmc then
			sv_pmc = sv_pmc_
		end
	end)
    
	function damage.send(clients)
		if not clients then 
			clients = 0 
		end
	
		local data = new_bitstream()
		data:add_int(sv_pmc)
		pmc:send(data, clients, SendMode.ReliableOrdered, RepRule.Auth2All)
	end
	
	function bindings.playerNetworkInit(newb, connID)
		if AUTH then
			damage.send(connID)
		end
	end
	
	function damage.create_blood(object,worm,amount)
		local wx,wy = worm:pos()
		local ox,oy = object:pos()
		local angle = vector_direction(wx, wy, ox, oy)
		worm:shoot(blood,amount*3,1,1,0,0,60,180,0)
	end

	function bindings.playerInit(p)
		p:data().lastHurtName = p
		p:data().lastHurtWeapon = ""
		p:data().suicides = 0
		p:data().doSuicide = false
	end
		
	function damage.main(object, worm, amount, name, explosion)
		--if sv_pmc == 0 then
		--	amount = amount /2
		--end
		if isTeamPlay() then
			if object:player() ~= nil and worm:player() ~= nil then
				if object:player() ~= worm:player() then
					if object:player():team() ~= worm:player():team() then
						worm:damage(amount,object:player())
						damage.create_blood(object,worm,amount)
						if not explosion and object:player() ~= worm:player() then
							stats.add(object, "hits", name)
						end
					end
				else
					worm:damage(amount,object:player())
					damage.create_blood(object,worm,amount)
					if not explosion and object:player() ~= worm:player() then
						stats.add(object, "hits", name)
					end
				end
				else
					if worm == nil then
						print("WORM: NIL")
					end
					if object == nil then
						print("OBJECT: NIL")
					end
				end
			else
				worm:damage(amount,object:player())
				damage.create_blood(object,worm,amount)
				if not explosion and object:player() ~= worm:player() then
					stats.add(object, "hits", name)
				end
			end
			
			if worm:health() == 0 then
				if object:player() == worm:player() and not object:player():data().doSuicide then
					object:player():data().doSuicide = true
					object:player():data().suicides = object:player():data().suicides + 1
					--print(object:player():data().suicides)
				end
			end
			
			--[[worm:data().lastHurtName = object:player()
			if name ~= "null" then
				worm:data().lastHurtWeapon = name
			end]]
		--end
		--awards.berCheck(object, worm)
	end

	function bindings.wormDeath(w)
		--[[if w:player() == w:player():data().lastHurtName then
			addMessage(w:player():name().." blew himself up")
		else
			addMessage(w:player():name().." got killed by "..w:player():data().lastHurtName:name().." with a "..w:player():data().lastHurtWeapon)
		end
		w:player():data().lastHurtName = w:player()
		w:player():data().lastHurtWeapon = ""]]
	end
	
--PROJECTILES DAMAGE
	function damage.lightning(object,worm)
		local dmg = damages[sv_pmc]["Lightning Gun"]
		damage.main(object,worm,dmg,"Lightning Gun", false)
	end
	
	function damage.autocannon(object,worm)
		--damage.main(object,worm,0, "null", false)
	end
	
	function damage.blowtorch(object,worm)
		local dmg = damages[sv_pmc]["Blowtorch"]
		damage.main(object,worm,dmg,"Blowtorch", false)
	end
	
	function damage.knife(object,worm)
		local dmg = damages[sv_pmc]["Throwing Knife"]
		damage.main(object,worm,dmg,"Throwing Knife", false)
	end

	function damage.chaingun(object,worm)
		local dmg = damages[sv_pmc]["Chaingun"]
		damage.main(object,worm,dmg,"Chaingun", false)
	end
	
	function damage.rifle(object,worm)
		if gameMode == 5 then
			damage.main(object,worm,400,"Rifle ", false)
		else
			local dmg = damages[sv_pmc]["Rifle"]
			damage.main(object,worm,dmg,"Rifle", false)
		end
	end
	
	function damage.flaregun(object,worm)
		local dmg = damages[sv_pmc]["Flare Gun"]
		damage.main(object,worm,dmg,"Flare Gun", false)
	end
	
	function damage.fire(object,worm)
		worm:damage(0.1,object:player())
	end
	
	function damage.gauss(object,worm)
		--damage.main(object,worm,0,"Gauss Gun", false)
	end
	
	function damage.winchester(object,worm)
		local dmg = damages[sv_pmc]["Winchester"]
		damage.main(object,worm,dmg,"Winchester", false)
	end
	
	function damage.flak(object,worm)
		local dmg = damages[sv_pmc]["Flak Cannon"]
		damage.main(object,worm,dmg,"Flak Cannon", false)
	end
	
	function damage.bazooka(object,worm)
		local dmg = damages[sv_pmc]["Bazooka"]
		damage.main(object,worm,dmg,"null", true)
	end
	
	function damage.bfg(object,worm)
		local dmg = damages[sv_pmc]["BFG"]
		damage.main(object,worm,dmg,"null", true)
	end
	
	function damage.shotgun(object,worm)
		local dmg = damages[sv_pmc]["Shotgun"]
		damage.main(object,worm,dmg,"Shotgun", false)
	end
	
	function damage.mine(object,worm)
		local dmg = damages[sv_pmc]["Mine"]
		damage.main(object,worm,dmg,"Mine", false)
	end


--PARTICLES DAMAGE
	function damage.mortar_particle_close(object,worm)
		local dmg = damages[sv_pmc]["Mortart particle close"]
		damage.main(object,worm,dmg,"null", true)
	end
	
	function damage.mortar_particle_medium(object,worm)
		local dmg = damages[sv_pmc]["Mortart particle medium"]
		damage.main(object,worm,dmg,"null", true)
	end
	
	function damage.mortar_particle_far(object,worm)
		local dmg = damages[sv_pmc]["Mortart particle far"]
		damage.main(object,worm,dmg,"null", true)
	end
	
	function damage.launcher_particle_close(object,worm)
		local dmg = damages[sv_pmc]["Launcher particle close"]
		damage.main(object,worm,dmg,"null", true)
	end
	
	function damage.launcher_particle_medium(object,worm)
		local dmg = damages[sv_pmc]["Launcher particle medium"]
		damage.main(object,worm,dmg,"null", true)
	end

--EXPLOSIONS DAMAGE
  function damage.special_explosion_autocannon(object,worm)
		local ox,oy = object:pos()
		local wx,wy = worm:pos()
		
	--DAMAGE SETUP
		local range = 30	--MAX RANGE
		local power = damages[sv_pmc]["Autocannon explosion"]	--POWER IN FIRST STEP
		local steps = 1	--AMOUNT OF STEPS
		
		local tempsteps = steps
		
		if not (map_is_blocked(ox, oy, wx, wy)) then
			while tempsteps > 0 do
				if vector_distance(ox, oy, wx, wy) < (range / tempsteps) then
					damage.main(object,worm,floor(power/steps),"null", true)
				end
				tempsteps = tempsteps - 1
			end
			damage.main(object,worm,0,"Autocannon", false)
		end
	end
	
	function damage.special_explosion_bazooka(object,worm)
		local ox,oy = object:pos()
		local wx,wy = worm:pos()
		
	--DAMAGE SETUP
		local range = 30	--MAX RANGE
		local power = damages[sv_pmc]["Bazooka explosion"]	--POWER IN FIRST STEP
		local steps = 1	--AMOUNT OF STEPS
		
		local tempsteps = steps
		
		if not (map_is_blocked(ox, oy, wx, wy)) then
			while tempsteps > 0 do
				if vector_distance(ox, oy, wx, wy) < (range / tempsteps) then
					damage.main(object,worm,floor(power/steps),"null", true)
				end
				tempsteps = tempsteps - 1
			end
			damage.main(object,worm,0,"Bazooka", false)
		end
	end
	
	function damage.special_explosion_gauss(object,worm)
		local ox,oy = object:pos()
		local wx,wy = worm:pos()
		
	--DAMAGE SETUP
		local range = 20	--MAX RANGE
		local power = damages[sv_pmc]["Gauss explosion"]	--POWER IN FIRST STEP
		local steps = 1	--AMOUNT OF STEPS
		
		local tempsteps = steps
		
		if not (map_is_blocked(ox, oy, wx, wy)) then
			while tempsteps > 0 do
				if vector_distance(ox, oy, wx, wy) < (range / tempsteps) then
					damage.main(object,worm,floor(power/steps),"null", true)
				end
				tempsteps = tempsteps - 1
			end
			damage.main(object,worm,0,"Gauss Gun", false)
		end
	end
	
	function damage.special_explosion_doomsday(object,worm)
		local ox,oy = object:pos()
		local wx,wy = worm:pos()
		
	--DAMAGE SETUP
		local range = 15	--MAX RANGE
		local power = damages[sv_pmc]["Doomsday explosion"]	--POWER IN FIRST STEP
		local steps = 1	--AMOUNT OF STEPS
		
		local tempsteps = steps
		
		if not (map_is_blocked(ox, oy, wx, wy)) then
			while tempsteps > 0 do
				if vector_distance(ox, oy, wx, wy) < (range / tempsteps) then
					damage.main(object,worm,floor(power/steps),"null", true)
				end
				tempsteps = tempsteps - 1
			end
			damage.main(object,worm,0,"Doomsday", false)
		end
	end

	function damage.special_explosion_mortar(object,worm)
		local ox,oy = object:pos()
		local wx,wy = worm:pos()
		
	--DAMAGE SETUP
		local range = 45	--MAX RANGE
		local power = damages[sv_pmc]["Special explosion mortar"]	--POWER IN FIRST STEP
		local steps = 5		--AMOUNT OF STEPS
		
		local tempsteps = steps
		
		if not (map_is_blocked(ox, oy, wx, wy)) then
			while tempsteps > 0 do
				if vector_distance(ox, oy, wx, wy) < (range / tempsteps) then
					damage.main(object,worm,floor(power/steps),"null", true)
				end
				tempsteps = tempsteps - 1
			end
			damage.main(object,worm,0,"Mortar", false)
		end
	end
	
	function damage.special_explosion_grnlauncher(object,worm)
		local ox,oy = object:pos()
		local wx,wy = worm:pos()
		
	--DAMAGE SETUP
		local range = 90	--MAX RANGE
		local power = damages[sv_pmc]["Special explosion grnlauncher"]	--POWER IN FIRST STEP
		local steps = 4	--AMOUNT OF STEPS
		
		local tempsteps = steps
		
		if not (map_is_blocked(ox, oy, wx, wy)) then
			while tempsteps > 0 do
				if vector_distance(ox, oy, wx, wy) < (range / tempsteps) then
					damage.main(object,worm,floor(power/steps),"null", true)
				end
				tempsteps = tempsteps - 1
			end
			damage.main(object,worm,0,"Grenade Launcher", false)
		end
	end
	
	function damage.special_explosion_grenade(object,worm)
		local ox,oy = object:pos()
		local wx,wy = worm:pos()
		
	--DAMAGE SETUP
		local range = 30	--MAX RANGE
		local power = damages[sv_pmc]["Special explosion grenade"]	--POWER IN FIRST STEP
		local steps = 1	--AMOUNT OF STEPS
		
		local tempsteps = steps
		
		if not (map_is_blocked(ox, oy, wx, wy)) then
			while tempsteps > 0 do
				if vector_distance(ox, oy, wx, wy) < (range / tempsteps) then
					damage.main(object,worm,floor(power/steps),"null", true)
				end
				tempsteps = tempsteps - 1
			end
			damage.main(object,worm,0,"Grenade", false)
		end
	end
	
	function damage.special_explosion_bfg(object,worm)
		local ox,oy = object:pos()
		local wx,wy = worm:pos()
		
	--DAMAGE SETUP
		local range = 30	--MAX RANGE
		local power = damages[sv_pmc]["Special explosion bfg"]	--POWER IN FIRST STEP
		local steps = 3	--AMOUNT OF STEPS
		
		local tempsteps = steps
		
		if not (map_is_blocked(ox, oy, wx, wy)) then
			while tempsteps > 0 do
				if vector_distance(ox, oy, wx, wy) < (range / tempsteps) then
					damage.main(object,worm,floor(power/steps),"null", true)
				end
				tempsteps = tempsteps - 1
			end
			damage.main(object,worm,0,"BFG 9000", false)
		end
	end
end
