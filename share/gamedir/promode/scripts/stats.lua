function stats.init()
	local statsVisible = false
	
	local su = network_player_event("su", function(self, p, data)
		local i = data:get_int()
		local d = p:data()
		local hit = data:get_int()
		
		--print("recieving...")
		
		d.hits[weaponsName[i]] = hit
	end)
		
	function stats.update(pl)
		--print("...trying to send")
		
		if AUTH then
			for p in game_players() do
				for i = 1, weaponsCount do
					local data = new_bitstream()
					data:add_int(i)
					data:add_int(p:data().hits[weaponsName[i]])
					su:send(p, data)
					--print("sending...")
				end
			end
		end
	end
	
	cg_logstats = 1

	console_register_command("CG_LOGSTATS",function(i)
		if i == nil then
			return "CG_LOGSTATS IS: "..cg_logstats.." DEFAULT: 1"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 1 and i >= 0 then
				cg_logstats = i
			else
				return "CG_LOGSTATS IS: "..ccg_logstats.." DEFAULT: 1"
			end
		end
	end)

	if gameMode ~= 5 then
		weaponsCount = 5
	else
		weaponsCount = 1
	end
	
	console_register_command("+STATS", function()
		statsVisible = true
	end)
			
	console_register_command("-STATS", function()
		statsVisible = false
	end)
    
	function stats.add(worm, type, tableItem)
		local p = worm:player()
		local d = p:data()
		
		if p ~= nil then
			if gameEnd == 0 then
				if tableItem ~= "null" then
					if type == "hits" then
						d.hits[tableItem] = d.hits[tableItem] + 1
						--[[for i = 1, weaponsCount do
							if d.weaponSelection.list[i]:name() == tableItem then
								d.hitz[tableItem] = d.hitz[tableItem] + 1	
								break
							end
						end]]
					elseif type == "atts" then
							d.atts[tableItem] = d.atts[tableItem] + 1
					elseif type == "awards" then
						d.awards[tableItem] = d.awards[tableItem] + 1
					else
						debug("Unknow statistic type")
					end
				end
			end
		end
	end
	
	function stats.lightning(object)
		stats.add(object, "atts", "Lightning Gun")
	end
	
	function stats.blowtorch(object)
		stats.add(object, "atts", "Blowtorch")
	end

	function stats.chaingun(object)
		stats.add(object, "atts", "Chaingun")
	end
	
	function stats.autocannon(object)
		stats.add(object, "atts", "Autocannon")
	end
	
	function stats.rifle(object)
		if gameMode == 5 then
			stats.add(object, "atts", "Rifle ")
		else
			stats.add(object, "atts", "Rifle")
		end
	end
	
	function stats.gauss(object)
		stats.add(object, "atts", "Gauss Gun")
	end
	
	function stats.winchester(object)
		stats.add(object, "atts", "Winchester")
	end
	
	function stats.flak(object)
		stats.add(object, "atts", "Flak Cannon")
	end
	
	function stats.knife(object)
		stats.add(object, "atts", "Throwing Knife")
	end
	
	function stats.bazooka(object)
		stats.add(object, "atts", "Bazooka")
	end
	
	function stats.bfg(object)
		stats.add(object, "atts", "BFG 9000")
	end
	
	function stats.shotgun(object)
		stats.add(object, "atts", "Shotgun")
	end

	function stats.grenade(object)
		stats.add(object, "atts", "Grenade")
	end

	function stats.grenade_launcher(object)
		stats.add(object, "atts", "Grenade Launcher")
	end	

	function stats.doomsday(object)
		stats.add(object, "atts", "Doomsday")
	end

	function stats.mortar(object)
		stats.add(object, "atts", "Mortar")
	end
	
	function stats.mine(object)
		stats.add(object, "atts", "Mine")
	end
	
	function stats.flaregun(object)
		stats.add(object, "atts", "Flare Gun")
	end
	
	function stats.drawBox(bitmap, filled, x, y, width, height)
		gfx_set_alpha(50)
			bitmap:draw_box(x, y, x+width, y+height, color(0,0,0))
			bitmap:draw_box(x+1, y+1, x+1+(width-2), y+height-1, color(200, 200, 200))
		gfx_reset_blending()
	end
	    
	function bindings.viewportRender(viewport, worm)
		local bitmap = viewport:bitmap()
		local p = worm:player()
		local d = p:data()
		local leftMarign = 0
		local topMargin = 0
				
		if (not DEDSERV and gameEnd > 0 and gameMode ~= 2) or statsVisible then
			gfx_set_alpha(255)
				stats.drawBox(bitmap, 0 , bitmap:w()/30, 150, 150, 80)
			gfx_reset_blending()
			
			fonts.minifont:render( bitmap, "05Score00" , bitmap:w()/30+21, 152, color(255, 255, 255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, "03Klls00" , bitmap:w()/30+46, 152, color(255, 255, 255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, "02Dths00" , bitmap:w()/30+71, 152, color(255, 255, 255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, "02Sui00" , bitmap:w()/30+96, 152, color(255, 255, 255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, "05Effcny00" , bitmap:w()/30+136, 152, color(255, 255, 255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, p:kills()-d.suicides, bitmap:w()/30+21, 158, color(255, 255, 255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, p:kills() , bitmap:w()/30+46, 158, color(255, 255, 255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, p:deaths(), bitmap:w()/30+71, 158, color(255, 255, 255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, d.suicides, bitmap:w()/30+95, 158, color(255, 255, 255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, floor(p:kills()/(p:kills()+p:deaths())*100) .. " %", bitmap:w()/30+136, 158, color(255, 255, 255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, "06Weapon00" , bitmap:w()/30+25, 170, color(255,255,255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, "05Accrcy00" , bitmap:w()/30+80, 170, color(255,255,255), Font.Formatting + Font.Right )
			fonts.minifont:render( bitmap, "Hits/Atts" , bitmap:w()/30+130, 170, color(255,255,255), Font.Formatting + Font.Right )
	
			for i = 1, weaponsCount do
				fonts.minifont:render(bitmap, d.weaponSelection.list[i]:name(), bitmap:w()/30 + 1, 176 + topMargin, color(255, 255, 255),  Font.Formatting)
				fonts.minifont:render(bitmap, d.hits[d.weaponSelection.list[i]:name()] .. "/", bitmap:w()/30 + 114, 176 + topMargin, color(255, 255, 255), Font.Formatting + Font.Right)
				fonts.minifont:render(bitmap, d.atts[d.weaponSelection.list[i]:name()], bitmap:w()/30 + 114, 176 + topMargin, color(255, 255, 255), Font.Left)
				if d.atts[d.weaponSelection.list[i]:name()] > 0 then
					fonts.minifont:render(bitmap, floor((d.hits[d.weaponSelection.list[i]:name()] / d.atts[p:data().weaponSelection.list[i]:name()]) * 100) .. " %", bitmap:w()/30 + 80, 176 + topMargin, color(255, 255, 255), Font.Formatting + Font.Right)
				end
				topMargin = topMargin + 6
			end
		end
	end
	
	function stats.save()
		if (cg_logstats == 1) then
			print("launcher addstat gametype "..modes_[gameMode])
			print("launcher addstat gamemap "..gameMap)

			for p in game_players() do
				if (p:team() ~= 0) then
					local d = p:data()
					print("launcher addstat name "..string.gsub(p:name(), "[%d%s%a%c][%d%s%a%c]", ""))
					print("launcher addstat team "..p:team())
					print("launcher addstat score "..p:kills()-d.suicides)
					print("launcher addstat kills "..p:kills())
					print("launcher addstat deaths "..p:deaths())
					print("launcher addstat suicides "..d.suicides)
					print("launcher addstat effnency "..floor(p:kills()/(p:kills()+p:deaths())*100))
					for i = 1, weaponsCount do
						print("launcher addstat weapon "..d.weaponSelection.list[i]:name())
						print("launcher addstat hits "..d.hits[d.weaponSelection.list[i]:name()])
						print("launcher addstat atts "..d.atts[d.weaponSelection.list[i]:name()])
						print("launcher addstat accuracy "..floor((d.hits[d.weaponSelection.list[i]:name()] / d.atts[p:data().weaponSelection.list[i]:name()]) * 100))
					end
				end
			end
			print("launcher savestat")
		end
	end
end
