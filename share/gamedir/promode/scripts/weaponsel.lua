function weaponsel.init()
	
	function bindings.playerInit(p)
		p:data().init = false
	end
	
	weaponsList = {"Bazooka", "Chaingun", "Doomsday", "Flak Cannon", "Grenade Launcher", "Gauss Gun", "Grenade", "Lightning Gun", "Mortar", "Shotgun", "Throwing Knife", "Winchester", "Autocannon"}

	local cl_weapons = {1, 1, 1, 1, 1}

	console_register_command("CL_WEAPONS", function(i, j, k, l, m)
		local cl_weapons_ = {i, j, k, l, m}

		for v = 1,5 do
			if cl_weapons_[v] == nil then
				cl_weapons_[v] = 1
			elseif cl_weapons_[v] ~= nil then
				if tonumber(cl_weapons_[v]) and tonumber(cl_weapons_[v]) <= table.getn(weaponsList) and tonumber(cl_weapons_[v]) >= 1 then
					cl_weapons_[v] = cl_weapons_[v] * 1
					cl_weapons[v] = cl_weapons_[v]
				else
					cl_weapons[v] = 1
				end
			end
		end
	end)
	
	local maxSelectableWeapons
	if gameMode ~= 5 then
		maxSelectableWeapons = 5
	else
		maxSelectableWeapons = 1
	end
	
	local selectWeaponStart = maxSelectableWeapons+1
	local selectWeaponCount = selectWeaponStart+1
	local weapons = {}

	if not DEDSERV then
		function bindings.viewportRender(viewport, worm)
			local bitmap = viewport:bitmap()
			local player = worm:player()
			if player and player:data().weaponSelection and player:data().weaponSelection.finished == 0 then
				local o = player:data().weaponSelection
				local y = (bitmap:h() - selectWeaponCount*10) / 2
				local x = (bitmap:w() - 60) / 2
				local i = 0
				
				local function drawRow(name)
				if i == o.cur then
						--hudtools.drawSelected(bitmap, x-2, y, (string.len(name)*5)+1, 20)
						fonts.liero:render(
							bitmap,
							name,
							x, y,
							color(255, 255, 255), Font.Shadow)
						
				else
					if name == "Randomize" then
						fonts.liero:render(
							bitmap,
							name,
							x, y,
							color(196,168,124), Font.Shadow)
					elseif name == "DONE!" then
						fonts.liero:render(
							bitmap,
							name,
							x, y,
							color(84, 216, 84), Font.Shadow)
					else
						fonts.liero:render(
							bitmap,
							name,
							x, y,
							color(168, 168, 248), Font.Shadow)
					end
				end
				y = y + 10
				i = i + 1
			end
				
				drawRow("Randomize")

				while i <= maxSelectableWeapons do
					drawRow(o.list[i]:name())
				end
				
				drawRow("DONE!")
			end
		end
	end
		
		function weaponsel.randomWeapons()
			local list = {}
			
			for i = 1, maxSelectableWeapons do
				local ok
				if gameMode ~= 5 then
					repeat
						list[i] = weapon_random()
					
						if weapon_count() < maxSelectableWeapons then
							break
						end
						
						ok = true
						for j = 1, i-1 do
							if list[j] == list[i] then
								ok = false
							end
						end
					until ok and list[i]:name() ~= "Rifle "
				else
					repeat
						list[i] = weapon_random()
						
						if weapon_count() < maxSelectableWeapons then
							break
						end
						
						ok = true
						for j = 1, i-1 do
							if list[j] == list[i] then
								ok = false
							end
						end
					until ok
				end
			end
				
			return list
			
		end
	
		function bindings.localplayerEvent(player, event, state)
			local o = player:data().weaponSelection

			if o and o.finished == 0 then			
				if state then
					if event == Player.Down then
						o.cur = o.cur + 1
						if o.cur >= selectWeaponCount then
							o.cur = 0
						end
						elseif event == Player.Up then
						o.cur = o.cur - 1
						if o.cur < 0 then
							o.cur = selectWeaponCount - 1
						end
					elseif event == Player.Fire then
						o.pressed = true
					elseif o.list[o.cur] then
						if event == Player.Left and gameMode ~= 5 then
							o.list[o.cur] = o.list[o.cur]:prev()
							if o.list[o.cur]:name() == "Rifle " then
								o.list[o.cur] = o.list[o.cur]:prev()
							end
							for wpn = 1,5 do
								if o.list[o.cur]:name() == o.list[wpn]:name() and o.cur ~= wpn or o.list[o.cur]:name() == "Rifle " then
									o.list[o.cur] = o.list[o.cur]:prev()
								end
							end
						elseif event == Player.Right and gameMode ~= 5 then
							o.list[o.cur] = o.list[o.cur]:next()
							if o.list[o.cur]:name() == "Rifle " then
								o.list[o.cur] = o.list[o.cur]:next()
							end
							for wpn = 1,5 do
								if o.list[o.cur]:name() == o.list[wpn]:name() and o.cur ~= wpn or o.list[o.cur]:name() == "Rifle " then
									o.list[o.cur] = o.list[o.cur]:next()
								end
							end
						end
					end
				else
					if event == Player.Fire then
						o.pressed = false
					end
				end
			end
		end
	
		function bindings.playerUpdate(player)
			local o = player:data().weaponSelection
			
			if not player:data().weaponSelection then
				player:data().weaponSelection =
				{
					list = weaponsel.randomWeapons(),
					cur = 1,
					finished = 0
				}
			end
	
			if gameMode ~= 5 then
				if not player:data().init and player:data().weaponSelection.list ~= nil then
					for wpn = 1,5 do
						repeat
							player:data().weaponSelection.list[wpn] = player:data().weaponSelection.list[wpn]:prev()
						until player:data().weaponSelection.list[wpn]:name() == weaponsList[cl_weapons[wpn]]
						--[[for i = 1,5 do
							if player:data().weaponSelection.list[i]:name() == weaponsList[cl_weapons[wpn]] --[[and i ~= wpn then
								player:data().weaponSelection.list[wpn] = player:data().weaponSelection.list[wpn]:prev()
							end
						end]]
						
						local nob = false
						
						for x = 1,5 do
							for y = 1,5 do
								if cl_weapons[x] == cl_weapons[y] and x ~= y then
									nob = true
									break
								end
							end
							if nob then
								break
							end
						end
						
						if nob then
							player:data().weaponSelection.list = weaponsel.randomWeapons()
						end
					end
	
					player:data().init = true
				end
			else
				if not player:data().init and player:data().weaponSelection.list ~= nil then
					repeat
						player:data().weaponSelection.list[1] = player:data().weaponSelection.list[1]:prev()
					until player:data().weaponSelection.list[1]:name() == "Rifle "
					
					player:data().init = true
				end
			end
			if o and o.finished == 0 and o.pressed then
				if o.cur == 0 and gameMode ~= 5 then
					o.list = weaponsel.randomWeapons()
				elseif o.cur == selectWeaponStart then
					player:select_weapons(o.list)
					if gameMode ~= 5 then
						for i = 1, 5 do
							weaponsNumber[o.list[i]:name()] = i
							table.insert(weaponsName, o.list[i]:name())
						end
					else
						weaponsNumber[o.list[1]:name()] = 1
						table.insert(weaponsName, o.list[1]:name())
					end
					playerWeaponStart(player)
				end
			end
		end
		
		function playerWeaponStart(player)
			player:data().weaponSelection.finished = 1
			-- this immediate start was added for OLX:
			if isTeamPlay() then
				console.p0_team = 1
				console.p1_team = 1
			else
				console.p0_team = 1
				console.p1_team = 1
			end
		end

		function bindings.playerInit(player)
			player:data().weaponSelection =
			{
				list = weaponsel.randomWeapons(),
				cur = 1,
				finished = 0
			}
		end

		function bindings.localplayerInit(player)
			player:data().weaponSelection =
			{
				list = weaponsel.randomWeapons(),
				cur = 1,
				finished = 0
			}
		end

		function weaponsel.start(player)
			player:data().weaponSelection =
			{
				list = player:data().weaponSelection.list,
				cur = 1,
				finished = 0
			}
		end
		
end
