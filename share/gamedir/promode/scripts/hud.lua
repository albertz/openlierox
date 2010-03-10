function hud.init()

	icons = {}
	if not DEDSERV then
		icons["Autocannon"] = sprites_load("icons/small/autocannon.png")
		icons["Bazooka"] = sprites_load("icons/small/bazooka.png")
		icons["BFG 9000"] = sprites_load("icons/small/bfg.png")
		icons["Blowtorch"] = sprites_load("icons/small/blowtorch.png")
		icons["Chaingun"] = sprites_load("icons/small/chaingun.png")
		icons["Doomsday"] = sprites_load("icons/small/doomsday.png")
		icons["Flak Cannon"] = sprites_load("icons/small/flak.png")
		icons["Grenade Launcher"] = sprites_load("icons/small/grn_launcher.png")
		icons["Gauss Gun"] = sprites_load("icons/small/gauss.png")
		icons["Grenade"] = sprites_load("icons/small/grenade.png")
		icons["Throwing Knife"] = sprites_load("icons/small/knife.png")
		icons["Lightning Gun"] = sprites_load("icons/small/lightning.png")
		icons["Mine"] = sprites_load("icons/small/mine.png")
		icons["Mortar"] = sprites_load("icons/small/mortar.png")
		icons["Rifle"] = sprites_load("icons/small/rifle.png")
		icons["Rifle "] = sprites_load("icons/small/rifle.png")
		icons["Shotgun"] = sprites_load("icons/small/shotgun.png")
		icons["Winchester"] = sprites_load("icons/small/winchester.png")
	end

	function isVisible(worm, owner)
		local ownerWorm = owner:worm()
		local x1, y1 = ownerWorm:pos()
		local x2, y2 = worm:pos()
		local relAngle = vector_direction(x1, y1, x2, y2)
			
		if worm:health() > 0 then
			if darkMode then
				if abs(angle_diff(ownerWorm:angle(), relAngle)) < fov and not map_is_blocked(x1, y1, x2, y2) then
					return true
				else 
					return false
				end
			else
				return true
			end
		else
			return false
		end
	end

	if not DEDSERV then
		ch_playerhealth = 1
		ch_playerhealth_own = 0
		ch_playernames = 1
		ch_crosshairtype = 1
		ch_crosshairdist = 25
		ch_radar = 1
		ch_radartype = 0
		ch_statusbar = 1
		ch_weaponsinfo = 0
		ch_reloadtimer = 1
		crossh = {r = "0", g = "255", b = "0"}
		enemy = {r = "0", g = "255", b = "0"}
		friend = {r = "0", g = "0", b = "255"}
		box_red = {r = "255", g = "0", b = "0"}
		box_blue = {r = "90", g = "90", b = "255"}
		fov = 135
		cg_draw2d = 1
		
		console_register_command("CG_DRAW2D",function(i)
			if i == nil then
				return "CG_DRAW2D IS: "..cg_draw2d.." DEFAULT: 1"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 1 and i >= 0 then
					cg_draw2d = i
				else
					return "CG_DRAW2D IS: "..cg_draw2d.." DEFAULT: 1"
				end
			end
		end)

		console_register_command("CH_STATUSBAR",function(i)
			if i == nil then
				return "CH_STATUSBAR IS: "..ch_statusbar.." DEFAULT: 1"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 4 and i >= 0 then
					ch_statusbar = i
				else
					return "CH_STATUSBAR IS: "..ch_statusbar.." DEFAULT: 1"
				end
			end
		end)
		
		console_register_command("CH_WEAPONSINFO",function(i)
			if i == nil then
				return "CH_SWEAPONSINFO IS: "..ch_weaponsinfo.." DEFAULT: 0"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 3 and i >= 0 then
					ch_weaponsinfo = i
				else
					return "CH_WEAPONSINFO IS: "..ch_weaponsinfo.." DEFAULT: 0"
				end
			end
		end)

		console_register_command("CH_RELOADTIMER",function(i)
			if i == nil then
				return "CH_RELOADTIMER IS: "..ch_reloadtimer.." DEFAULT: 1"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 1 and i >= 0 then
					ch_reloadtimer = i
				else
					return "CH_RELOADTIMER IS: "..ch_reloadtimer.." DEFAULT: 1"
				end
			end
		end)

		console_register_command("CH_CROSSHAIRTYPE",function(i)
			if i == nil then
				return "CH_CROSSHAIRTYPE IS: "..ch_crosshairtype.." DEFAULT: 1"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 7 and i >= 0 then
					ch_crosshairtype = i
				else
					return "CH_CROSSHAIRTYPE IS: "..ch_crosshairtype.." DEFAULT: 1"
				end
			end
		end)

		console_register_command("CH_CROSSHAIRDIST",function(i)
			if i == nil then
				return "CH_CROSSHAIRDIST IS: "..ch_crosshairdist.." DEFAULT: 25"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 30 and i >= 0 then
					ch_crosshairdist = i
				else
					return "CH_CROSSHAIRDIST IS: "..ch_crosshairdist.." DEFAULT: 25"
				end
			end
		end)

		console_register_command("CH_CROSSHAIRCOLOR",function(i,j,k)
			if i == nil then
				return "CH_CROSSHAIRCOLOR IS: "..crossh.r.." "..crossh.g.." "..crossh.b.." DEFAULT: 0 255 0"
			elseif i ~= nil then
				local i = i *1
				local j = j *1
				local k = k *1
				if tonumber(i) and tonumber(j) and tonumber(k) then
					if i >= 0 and i <= 255 then
						if j >= 0 and j <= 255 then
							if k >= 0 and k <= 255 then
								crossh.r = i
								crossh.g = j
								crossh.b = k
							end
						end
					end
				else
					return "CH_CROSSHAIRCOLOR IS: "..crossh.r.." "..crossh.g.." "..crossh.b.." DEFAULT: 0 255 0"
				end
			end
		end)
	
		console_register_command("CG_ENEMYBOX",function(i)
			if i == nil then
				return "CG_ENEMYBOX IS: "..cg_enemybox.." DEFAULT: 0"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 1 and i >= 0 then
					cg_enemybox = i
				else
					return "CG_ENEMYBOX IS: "..cg_enemybox.." DEFAULT: 0"
				end
			end
		end)

		console_register_command("CG_ENEMYCOLOR",function(i,j,k)
			if i == nil then
				return "CG_ENEMYCOLOR IS: "..enemy.r.." "..enemy.g.." "..enemy.b.." DEFAULT: 0 255 0"
			elseif i ~= nil then
				local i = i *1
				local j = j *1
				local k = k *1
				if tonumber(i) and tonumber(j) and tonumber(k) then
					if i >= 0 and i <= 255 then
						if j >= 0 and j <= 255 then
							if k >= 0 and k <= 255 then
								enemy.r = i
								enemy.g = j
								enemy.b = k
							end
						end
					end
				else
					return "CG_ENEMYCOLOR IS: "..enemy.r.." "..enemy.g.." "..enemy.b.." DEFAULT: 0 255 0"
				end
			end
		end)

		console_register_command("CG_TEAMBOX",function(i)
			if i == nil then
				return "CG_TEAMBOX IS: "..cg_teambox.." DEFAULT: 0"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 1 and i >= 0 then
					cg_teambox = i
				else
					return "CG_TEAMBOX IS: "..cg_teambox.." DEFAULT: 0"
				end
			end
		end)

		console_register_command("CG_TEAMCOLOR",function(i,j,k)
			if i == nil then
				return "CG_TEAMCOLOR IS: "..friend.r.." "..friend.g.." "..friend.b.." DEFAULT: 0 0 255"
			elseif i ~= nil then
				local i = i *1
				local j = j *1
				local k = k *1
				if tonumber(i) and tonumber(j) and tonumber(k) then
					if i >= 0 and i <= 255 then
						if j >= 0 and j <= 255 then
							if k >= 0 and k <= 255 then
								friend.r = i
								friend.g = j
								friend.b = k
							end
						end
					end
				else
					return "CG_TEAMCOLOR IS: "..friend.r.." "..friend.g.." "..friend.b.." DEFAULT: 0 0 255"
				end
			end
		end)
		
		console_register_command("CH_PLAYERHEALTH_OWN",function(i)
			if i == nil then
				return "CH_PLAYERHEALTH_OWN IS: "..ch_playerhealth_own.." DEFAULT: 0"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 1 and i >= 0 then
					ch_playerhealth_own = i
				else
					return "CH_PLAYERHEALTH_OWN IS: "..ch_playerhealth_own.." DEFAULT: 0"
				end
			end
		end)

		console_register_command("CH_PLAYERHEALTH",function(i)
			if i == nil then
				return "CH_PLAYERHEALTH IS: "..ch_playerhealth.." DEFAULT: 1"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 1 and i >= 0 then
					ch_playerhealth = i
				else
					return "CH_PLAYERHEALTH IS: "..ch_playerhealth.." DEFAULT: 1"
				end
			end
		end)
		
		console_register_command("CH_PLAYERNAMES",function(i)
			if i == nil then
				return "CH_PLAYERNAMES IS: "..ch_playernames.." DEFAULT: 1"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 1 and i >= 0 then
					ch_playernames = i
				else
					return "CH_PLAYERNAMES IS: "..ch_playernames.." DEFAULT: 1"
				end
			end
		end)

		console_register_command("CH_RADAR",function(i)
			if i == nil then
				return "CH_RADAR IS: "..ch_radar.." DEFAULT: 1"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 1 and i >= 0 then
					ch_radar = i
				else
					return "CH_RADAR IS: "..ch_radar.." DEFAULT: 1"
				end
			end
		end)
		
		console_register_command("CH_RADARTYPE",function(i)
			if i == nil then
				return "CH_RADARTYPE IS: "..ch_radartype.." DEFAULT: 1"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 2 and i >= 0 then
					ch_radartype = i
				else
					return "CH_RADARTYPE IS: "..ch_radartype.." DEFAULT: 1"
				end
			end
		end)

		function drawWormBox(bitmap, x, y, worm, friend_)
			local colour = {}
			local team = worm:player():team()

			if isTeamPlay() then
				if team == 1 then
					if friend_ then 
						colour = box_red
					else
						colour = box_red
					end
				elseif team == 2 then
					if friend_ then 
						colour = box_blue
					else
						colour = box_blue
					end
				end
			else
				if friend_ then 
					colour = friend
				else
					colour = enemy
				end
			end

			gfx_set_alpha(30)
				bitmap:draw_box(x - 6, y - 7, x + 6, y + 7, color(colour.r,colour.g,colour.b))
			gfx_set_alpha(180)
				bitmap:line(x - 7, y - 7, x + 6, y - 7, color(colour.r,colour.g,colour.b))
				bitmap:line(x + 7, y + 7-1, x - 6, y + 7-1, color(colour.r,colour.g,colour.b))
				bitmap:line(x - 6, y - 8, x - 6, y + 4, color(colour.r,colour.g,colour.b))
				bitmap:line(x + 6, y + 8-1, x + 6, y - 5, color(colour.r,colour.g,colour.b))	
			gfx_reset_blending()		
		end
		
		function drawWormDot(bitmap, x, y, worm, friend_)
			local color = {}
			local team = worm:player():team()
			
			if isTeamPlay() then
				if team == 1 then
					if friend_ then 
						color = box_red
					else
						color = box_blue
					end
				elseif team == 2 then
					if friend_ then 
						color = box_blue
					else
						color = box_red
					end
				end
			else
				if friend_ then 
					color = friend
				else
					color = enemy
				end
			end

			gfx_set_alphach(255)
				bitmap:putpixel(x, y, color.r, color.g, color.b)
			gfx_reset_blending()
			if ch_radartype == 1 or ch_radartype == 2 then
				gfx_set_alphach(130)
					bitmap:putpixel(x-1, y, color.r, color.g, color.b)
					bitmap:putpixel(x+1, y, color.r, color.g, color.b)
					bitmap:putpixel(x, y+1, color.r, color.g, color.b)
					bitmap:putpixel(x, y-1, color.r, color.g, color.b)
				gfx_reset_blending()
			end
			if ch_radartype == 2 then
				gfx_set_alphach(30)
					bitmap:putpixel(x-1, y-1, color.r, color.g, color.b)
					bitmap:putpixel(x-1, y+1, color.r, color.g, color.b)
					bitmap:putpixel(x+1, y+1, color.r, color.g, color.b)
					bitmap:putpixel(x+1, y-1, color.r, color.g, color.b)
				gfx_reset_blending()
			end
		end

		local hudFont = fonts.liero
		local testCrosshair = sprites_load("crosshair")
		local chatBaloon = sprites_load("talk.png")
		local killsMeter = sprites_load("kills")
		local hudBar = sprites_load("hud_jrc_bar")
		local hudBarDown = sprites_load("hud_down")
		local reloadTimer = 0

		function bindings.viewportRender(viewport, worm)
			local bitmap = viewport:bitmap()
			local filled = (worm:health() / 100)
			local limit = 200-filled*2
				
			if worm:health() > 0 and cg_draw2d == 1 then	
			if gameMode ~= 5 then
			if ch_weaponsinfo == 1 then
				for i = 1,5 do
					local hx = 2
					local alpha1 = 25
					local alpha2 = 60
					local alpha3 = 0
					local color1 = 90
					local width = 10
					if worm:player():data().weaponSelection.list[i]:name() ==  worm:current_weapon():type():name() then
						hx = 7
						alpha1 = 100
						alpha2 = 255
						color1 = 255
						width = 25
						alpha3 = 255
					end
					gfx_set_alphach(alpha1)
					hudtools.simpleBox(bitmap, hx, 67 + (11 * i), width, 9)
					gfx_set_alphach(alpha2)
					icons[worm:player():data().weaponSelection.list[i]:name()]:render(bitmap, 0, hx + 5, 72 + (11 * i))
					gfx_set_alpha(alpha3)
					fonts.minifont:render(bitmap, worm:current_weapon():ammo(), hx + 23, 69 + (11 * i), color(color1, color1, color1), Font.Right)
				end
				gfx_reset_blending()
			elseif ch_weaponsinfo == 2 then
			elseif ch_weaponinfo == 3 then
			end
			end
			if ch_statusbar == 1 then
				hudBarDown:render(bitmap,0,bitmap:w()/2-55,240)
				hudBarDown:render(bitmap,0,bitmap:w()/2+55,240)
			
				if filled < 0.3 then
					fonts.minifont:render(bitmap,(math.ceil(filled*100)).."/100",bitmap:w()/2+54,233,color(255,180,0),Font.Formatting +Font.CenterH + Font.CenterV )
				else
					fonts.minifont:render(bitmap,(math.ceil(filled*100)).."/100",bitmap:w()/2+54,233,color(0,255,0),Font.Formatting +Font.CenterH + Font.CenterV )
				end

				if filled > 0 and filled <=1 then
					gfx_set_alpha(100)
						bitmap:draw_box(bitmap:w()/2+55, 237, (bitmap:w()/2+55)+(38-2)*filled, 239, hudtools.greenRedInterpol(1-filled))
						bitmap:draw_box((bitmap:w()/2+55)+(38-2)*(-filled), 237,bitmap:w()/2+55, 239, hudtools.greenRedInterpol(1-filled))
					gfx_reset_blending()

					bitmap:draw_box(bitmap:w()/2+55, 238, (bitmap:w()/2+55)+(37-2)*filled,238, hudtools.greenRedInterpol(1-filled))
					bitmap:draw_box((bitmap:w()/2+55)+(37-2)*(-filled), 238,bitmap:w()/2+55, 238, hudtools.greenRedInterpol(1-filled))
				elseif filled > 1 then
          gfx_set_alpha(100)
					bitmap:draw_box(bitmap:w()/2+55-(38-2), 237, (bitmap:w()/2+55)+(38-2), 239, color(150,150,255))
					gfx_reset_blending()
          bitmap:draw_box((bitmap:w()/2+55)-(37-2), 238,(bitmap:w()/2+55)+(37-2), 238, color(150,150,255))
				end
				
				local afilled = (worm:current_weapon():ammo())
				local w = worm:current_weapon()
				if w:is_reloading() then
					fonts.minifont:render(bitmap,100-floor(w:reload_time()/w:type():reload_time()*100).." %",bitmap:w()/2-55,233,color(255,0,0),Font.Formatting +Font.CenterH + Font.CenterV )
				else
					if w:type():name() == "Lightning Gun" or w:type():name() == "Blowtorch" then
						if w:ammo() < w:type():ammo() / 4 or (w:ammo() == 1 and w:type():ammo() > 1) then
							fonts.minifont:render(bitmap,floor(w:ammo()/10).." / "..floor(w:type():ammo()/10),bitmap:w()/2-56,233,color(255,180,0),Font.Formatting +Font.CenterH + Font.CenterV )
						else
							fonts.minifont:render(bitmap,floor(w:ammo()/10).." / "..floor(w:type():ammo()/10),bitmap:w()/2-56,233,color(0,255,0),Font.Formatting +Font.CenterH + Font.CenterV )
						end
					else
						if w:ammo() < w:type():ammo() / 4 or (w:ammo() == 1 and w:type():ammo() > 1) then
							fonts.minifont:render(bitmap,w:ammo().." / "..w:type():ammo(),bitmap:w()/2-56,233,color(255,180,0),Font.Formatting +Font.CenterH + Font.CenterV )
						else
							fonts.minifont:render(bitmap,w:ammo().." / "..w:type():ammo(),bitmap:w()/2-56,233,color(0,255,0),Font.Formatting +Font.CenterH + Font.CenterV )
						end
					end
				end
				local afilled = 0
				if w:is_reloading() then
					afilled = 1 - w:reload_time() / w:type():reload_time()
				else
					afilled = w:ammo() / w:type():ammo()
				end
				gfx_set_alpha(100)
					bitmap:draw_box(bitmap:w()/2-55, 237, (bitmap:w()/2-55)+(38-2)*afilled, 239, hudtools.greenRedInterpol(1-afilled))
					bitmap:draw_box((bitmap:w()/2-55)+(38-2)*(-afilled), 237,bitmap:w()/2-55, 239, hudtools.greenRedInterpol(1-afilled))
				gfx_reset_blending()

				bitmap:draw_box(bitmap:w()/2-55, 238, (bitmap:w()/2-55)+(37-2)*afilled,238, hudtools.greenRedInterpol(1-afilled))
				bitmap:draw_box((bitmap:w()/2-55)+(37-2)*(-afilled), 238,bitmap:w()/2-55, 238, hudtools.greenRedInterpol(1-afilled))
			elseif ch_statusbar == 2 then
				hudBar:render(bitmap,0,bitmap:w()/2,0)
				if filled > 0 and filled <= 1 then
					gfx_set_alpha(100)
						bitmap:draw_box(bitmap:w()/2, 1, (bitmap:w()/2)+(38-2)*filled, 3, hudtools.greenRedInterpol(1-filled))
						bitmap:draw_box((bitmap:w()/2)+(38-2)*(-filled), 1,bitmap:w()/2, 3, hudtools.greenRedInterpol(1-filled))
					gfx_reset_blending()

					bitmap:draw_box(bitmap:w()/2, 2, (bitmap:w()/2)+(37-2)*filled, 2, hudtools.greenRedInterpol(1-filled))
					bitmap:draw_box((bitmap:w()/2)+(37-2)*(-filled), 2,bitmap:w()/2, 2, hudtools.greenRedInterpol(1-filled))
				elseif filled > 1 then
          gfx_set_alpha(100)
					bitmap:draw_box((bitmap:w()/2)-(38-2), 1, (bitmap:w()/2)+(38-2), 3, color(150,150,255))
					gfx_reset_blending()
          bitmap:draw_box((bitmap:w()/2)-(37-2), 2,(bitmap:w()/2)+(37-2), 2, color(150,150,255))
				end
			
				local w = worm:current_weapon()
				if w:is_reloading() then
					fonts.minifont:render(bitmap,100-floor(w:reload_time()/w:type():reload_time()*100).." %",bitmap:w()/2,6,color(255,0,0),Font.Formatting +Font.CenterH + Font.CenterV )
				else
					if w:type():name() == "Lightning Gun" or w:type():name() == "Blowtorch" then
						if w:ammo() < w:type():ammo() / 4 or (w:ammo() == 1 and w:type():ammo() > 1) then
							fonts.minifont:render(bitmap,floor(w:ammo()/10).." / "..floor(w:type():ammo()/10),bitmap:w()/2-1,6,color(255,180,0),Font.Formatting +Font.CenterH + Font.CenterV )
						else
							fonts.minifont:render(bitmap,floor(w:ammo()/10).." / "..floor(w:type():ammo()/10),bitmap:w()/2-1,6,color(0,255,0),Font.Formatting +Font.CenterH + Font.CenterV )
						end
					else
						if w:ammo() < w:type():ammo() / 4 or (w:ammo() == 1 and w:type():ammo() > 1) then
							fonts.minifont:render(bitmap,w:ammo().." / "..w:type():ammo(),bitmap:w()/2-1,6,color(255,180,0),Font.Formatting +Font.CenterH + Font.CenterV )
						else
							fonts.minifont:render(bitmap,w:ammo().." / "..w:type():ammo(),bitmap:w()/2-1,6,color(0,255,0),Font.Formatting +Font.CenterH + Font.CenterV )
						end
					end
				end
			elseif ch_statusbar == 3 then
				killsMeter:render(bitmap, 0, bitmap:w()-140, 6)
				fonts.minifont:render(bitmap, worm:player():kills(), bitmap:w()-135, 4, color(255, 255, 255), 0)
	
				hudtools.drawHBar( bitmap, filled, bitmap:w()-54, 5, 50, 4 )	
				local w = worm:current_weapon()
	
				if w:is_reloading() then
					filled = 1 - w:reload_time() / w:type():reload_time()
				else
					filled = w:ammo() / w:type():ammo()
				end
	
				hudtools.drawHBar( bitmap, filled, bitmap:w()-109, 5, 50, 4 )
			elseif ch_statusbar == 4 then
				fonts.liero:render(bitmap, "Kills: ", 2, 206, color(84, 215, 84), Font.Shadow)
				fonts.liero:render(bitmap, "Deaths: ", 2, 214, color(252, 84, 84), Font.Shadow)
				fonts.liero:render(bitmap, worm:player():kills(), 21, 206, color(84, 215, 84), Font.Shadow)
				fonts.liero:render(bitmap, worm:player():deaths(), 31, 214, color(252, 84, 84), Font.Shadow)
				
				if filled > 0 then
					hudtools.drawLieroHBar(bitmap, filled,2, 194, 101, 3)
				else
					hudtools.drawLieroHBar(bitmap, 1, 2, 194, 101, 3)
				end

				local w = worm:current_weapon()
		
				if reloadTimer == 100 then
					reloadTimer = 0
				end

				reloadTimer = reloadTimer + 1
					
				if w:is_reloading() then
					aFilled = 1 - w:reload_time() / w:type():reload_time()
					aFilled2 = 0
				else
					aFilled = w:ammo() / w:type():ammo()
					aFilled2 = w:ammo()
				end

				hudtools.drawLieroABar(bitmap, aFilled, 2, 200, 101, 3)

				if w:is_reloading() then
					if reloadTimer <= 100 and reloadTimer >= 50 then
						fonts.liero:render(bitmap, "Reloading...", 6, 199, color(255, 255, 255), Font.Shadow)
					end
				end
			elseif ch_statusbar == 5 then
				if worm:is_changing() then
					fonts.minifont:render( bitmap, worm:current_weapon():type():name(), bitmap:w()-32, 200, color(150, 255, 255), Font.Formatting + Font.Right)
				else 
					fonts.minifont:render( bitmap, worm:current_weapon():type():name(), bitmap:w()-32, 200, color(255, 255, 255), Font.Formatting + Font.Right )
				end
				
				gfx_set_alpha(202)
					if worm:current_weapon():is_reloading() then
						fonts.minifont:render( bitmap, "02RELOADING00 " .. floor((worm:current_weapon():type():reload_time() - worm:current_weapon():reload_time()) / worm:current_weapon():type():reload_time() * 100) .. " %", bitmap:w()-32, 208, color(255, 255, 255), Font.Formatting + Font.Right )
					else
						fonts.minifont:render( bitmap, worm:current_weapon():ammo() .. "/" .. worm:current_weapon():type():ammo(), bitmap:w()-32, 208, color(255, 255, 255), Font.Formatting + Font.Right )
					end				
				gfx_reset_blending()

				hudtools.cogDrawHealth( bitmap, worm:health(), bitmap:w()-21, 200 )
	
				local w = worm:current_weapon()
				if w:is_reloading() then
					filled = 1 - w:reload_time() / w:type():reload_time()
				else
					filled = w:ammo() / w:type():ammo()
				end
				hudtools.cogDrawAmmo( bitmap, filled, bitmap:w()-8, 200 )
			end
			end
		end
	
		function bindings.wormRender(x, y, worm, viewport, ownerPlayer)
			local ownViewport = ( ownerPlayer == worm:player() )
			local bitmap = viewport:bitmap()
			local dx, dy = angle_vector(worm:angle(), 25)
			local dot_offset = 20
			local dot_offset2 = 60
			local player = worm:player()
			
			if worm:health() > 0 and cg_draw2d == 1 then	
			if not ownViewport then
				if ch_playerhealth == 1 then
					if isVisible(worm, ownerPlayer) then
						bitmap:draw_box(x - 5, y + 9, x + 5, y + 11, color(0, 0, 0))
						local filled = (worm:health() / 100)
						if filled <= 1 then
							bitmap:draw_box(x - 4, y + 10, x - 4 + filled * 8, y + 10, hudtools.greenRedInterpol(1-filled))
						else
							bitmap:draw_box(x - 4, y + 10, x - 4 + 8, y + 10, color(150,150,255))
						end
					end
				end
			
				local colour = {}

				if friend_ then 
					colour = friend
				else
					colour = enemy
				end

				local ownerWorm = ownerPlayer:worm()
				local x1, y1 = ownerWorm:pos()
				local x2, y2 = worm:pos()
				local relAngle = vector_direction(x1, y1, x2, y2)
				local isFriend = ( ownerPlayer:team() == worm:player():team() ) and ( ownerPlayer:team() ~= -1)
				
				if isVisible(worm, ownerPlayer) then
					if ch_playernames == 1 then
						if isFriend and isTeamPlay() then
							if cg_teambox == 1 or isTeamPlay() then
								fonts.minifont:render( bitmap, worm:player():name(), x, y - 14, color(255, 255, 255), Font.Formatting + Font.CenterH + Font.CenterV )			
							else
								fonts.minifont:render( bitmap, worm:player():name(), x, y - 10, color(255, 255, 255), Font.Formatting + Font.CenterH + Font.CenterV )			
							end
						else
							--if abs(angle_diff(ownerWorm:angle(), relAngle)) < 135 then
								--if not map_is_blocked(x1, y1, x2, y2) then
									if cg_enemybox == 1 or isTeamPlay() then
										fonts.minifont:render( bitmap, worm:player():name(), x, y - 14, color(255, 255, 255), Font.Formatting + Font.CenterH + Font.CenterV )	
									else
										fonts.minifont:render( bitmap, worm:player():name(), x, y - 10, color(255, 255, 255), Font.Formatting + Font.CenterH + Font.CenterV )	
									end
								--end
							--end
						end
					end
				
					if ownerWorm:health() > 0 then									
						if cg_teambox == 1 or isTeamPlay() then	
							if isFriend then
								if isTeamPlay() then
									drawWormBox(bitmap, x, y, worm, true )
								else
									if abs(angle_diff(ownerWorm:angle(), relAngle)) < 135 then
										if not map_is_blocked(x1, y1, x2, y2) then
											--drawWormBox(bitmap, x, y, worm, false )
										end
									end
								end
							end
						end
	
						if cg_enemybox == 1 or isTeamPlay() then
							if not isFriend or not isTeamPlay() then
								if ownerWorm then
									--if abs(angle_diff(ownerWorm:angle(), relAngle)) < 135 then
										--if not map_is_blocked(x1, y1, x2, y2) then
											drawWormBox(bitmap, x, y, worm, false)
										--end
									--end
								end
							end
						end
					end
				end
			else
				if cg_draw2d == 1 then
				if worm:is_changing() then
					fonts.minifont:render( bitmap, worm:current_weapon():type():name(), x, y - 10, color(255, 255, 255), Font.Formatting + Font.CenterH + Font.CenterV )
				end

				local w = worm:current_weapon()
				if ch_reloadtimer == 1 then
					if w:is_reloading() then			
						hudFont:render(bitmap,floor(w:reload_time()/30),x+1,y-17,color(255,255,255),Font.Formatting +Font.CenterH + Font.CenterV )
						for i=1,36 do
							tx,ty = angle_vector(i*10,5)
							if i/36 < 1 - w:reload_time() / w:type():reload_time() then
								bitmap:putpixelwu(x+tx,y+ty-17,color(0,255-(36-i)*4,0))
							else
								bitmap:putpixelwu(x+tx,y+ty-17,color(0,0,0))
							end
						end
					end
					
					if is_reloading(player, w:type():name()) then			
						hudFont:render(bitmap,floor(reload_time(player, w:type():name())/30),x+1,y-17,color(255,255,255),Font.Formatting +Font.CenterH + Font.CenterV )
						for i=1,36 do
							tx,ty = angle_vector(i*10,5)
							if i/36 < 1 - reload_time(player, w:type():name()) / w:type():reload_time() then
								bitmap:putpixelwu(x+tx,y+ty-17,color(0,255-(36-i)*4,0))
							else
								bitmap:putpixelwu(x+tx,y+ty-17,color(0,0,0))
							end
						end
					end
					
					if ch_playerhealth_own == 1 then
						bitmap:draw_box(x - 5, y + 9, x + 5, y + 11, color(0, 0, 0))
						local filled = (worm:health() / 100)
						if filled <= 1 then
							bitmap:draw_box(x - 4, y + 10, x - 4 + filled * 8, y + 10, hudtools.greenRedInterpol(1-filled))
						else
							bitmap:draw_box(x - 4, y + 10, x - 4 + 8, y + 10, color(150,150,255))
						end
					end
				end

				local dx, dy = angle_vector(worm:angle(), ch_crosshairdist)
				local dxl, dyl = angle_vector(worm:angle(), ch_crosshairdist+4)
		
				if ch_crosshairtype == 1 then
					bitmap:putpixel(x+dx, y+dy, crossh.r, crossh.g, crossh.b)
				elseif ch_crosshairtype == 2 then
					bitmap:putpixel(x+dx, y+dy, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx-1, y+dy, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx+1, y+dy, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx, y+dy+1, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx, y+dy-1, crossh.r, crossh.g, crossh.b)
				elseif ch_crosshairtype == 3 then
					bitmap:putpixel(x+dx-1, y+dy, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx+1, y+dy, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx, y+dy+1, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx, y+dy-1, crossh.r, crossh.g, crossh.b)
				elseif ch_crosshairtype == 4 then
					bitmap:putpixel(x+dx-2, y+dy, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx+2, y+dy, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx, y+dy+2, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx, y+dy-2, crossh.r, crossh.g, crossh.b)
				elseif ch_crosshairtype == 5 then
					bitmap:putpixel(x+dx-1, y+dy, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx+1, y+dy, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx, y+dy+1, crossh.r, crossh.g, crossh.b0)
					bitmap:putpixel(x+dx, y+dy-1, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx-1, y+dy-1, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx-1, y+dy+1, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx+1, y+dy+1, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx+1, y+dy-1, crossh.r, crossh.g, crossh.b)					
				elseif ch_crosshairtype == 6 then			
					bitmap:putpixel(x+dx-1, y+dy-1, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx-1, y+dy+1, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx+1, y+dy+1, crossh.r, crossh.g, crossh.b)
					bitmap:putpixel(x+dx+1, y+dy-1, crossh.r, crossh.g, crossh.b)
				elseif ch_crosshairtype == 7 then			
					bitmap:line(x+dx, y+dy, x+dxl, y+dyl, color(crossh.r, crossh.g, crossh.b))
				end

				ox,oy = worm:pos()

				if ch_radar == 1 then
					for p in game_players() do
						if p ~= worm:player() then
							local worm2 = p:worm()
	
							if worm2:health() > 0 then
								ex,ey = worm2:pos()
								local angle = vector_direction(ex,ey,ox,oy)
								px = math.sin(math.rad(angle)) * dot_offset2
								py = math.cos(math.rad(angle)) * dot_offset2
								--if isVisible(worm, ownerPlayer) then
									if not isTeamPlay() then
										if abs(angle_diff(worm:angle(), angle)) > 135 then
											if not map_is_blocked(ex, ey, ox, oy) then
												if vector_distance(ex,ey,ox,oy) > 70 then													
													drawWormDot(bitmap, x-px, y+py, worm, false)
												end
											end
										end
									else
										if worm:player():team() ~= p:team() then
											if abs(angle_diff(worm:angle(), angle)) > 135 then
												if not map_is_blocked(ex, ey, ox, oy) then
													if vector_distance(ex,ey,ox,oy) > 70 then			
														drawWormDot(bitmap, x-px, y+py, worm, false)
													end
												end
											end
										else
											if vector_distance(ex,ey,ox,oy) > 70 then
												drawWormDot(bitmap, x-px, y+py, worm, true)
											end
										end
									end
								--end
							end
						end
					end
				end
				end
			end
			end
		end
	end
end
