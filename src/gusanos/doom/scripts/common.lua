function common.serverList(window)
	if window then
		window:clear()
		window:insert("Fetching server list...")
		
		fetch_server_list(function(list)
			window:clear()
			if list then
				for _, server in ipairs (list) do
					local n = window:insert(server.title, server.mod, server.map)
					n:data().ip = server.ip
				end
			else
				window:insert("Couldn't fetch list from server")
			end
		end)
	end
end



function common.initScoreboard()
	if not DEDSERV then
		gui_load_gss("scoreboard")
		local scoreboardw = gui_load_xml("scoreboard")
		
		scoreboardw:set_visibility(false)
		
		local scoreFields = {}

		function addScoreField(name, size, func)
			scoreboardw:add_column(name, size)
			table.insert(scoreFields, func)
		end
		
		addScoreField("Name", 1, function(p)
			return p:name()
		end)
		
		addScoreField("Kills", 1, function(p)
			return p:kills()
		end)
		
		addScoreField("Deaths", 1, function(p)
			return p:deaths()
		end)
		
		function fillScoreboard(p)
			local l = {}
			for i, v in ipairs(scoreFields) do
				l[i] = v(p)
			end
			local item = scoreboardw:insert(unpack(l))
			item:data().p = p
		end
		
		function scoreboardComparer(a, b)
			return a:data().p:kills() > b:data().p:kills() 
		end
				
		console_register_command("+SCORES", function()
			scoreboardw:clear()
			for p in game_players() do
				fillScoreboard(p)
			end

			scoreboardw:sort(scoreboardComparer)
			
			scoreboardw:set_visibility(true)
		end)
		
		console_register_command("-SCORES", function()
			scoreboardw:set_visibility(false)
		end)
	end
end

function common.initChat()
	local menu_sound = sounds["menu_use.ogg"]	

	if not DEDSERV then
		gui_load_gss("chat")
		local chatboxw = gui_load_xml("chat")
		
		chatboxw:set_visibility(false)
			
		function chatboxw:onKeyDown(k)
			if k == Keys.ESC then
				
				menu_sound:play(player, nil, 100, 1, 0)
	
				self:set_visibility(false)
				return true
			end
		end
		
		local chattextw = chatboxw:child("chattext")
		
		function chattextw:onAction()
			local p = game_local_player(0)
			if p then
				p:say(self:text())
				chatboxw:set_visibility(false)
			end
		end
		
		console_register_command("SHOWCHAT", function()
			if game_local_player(0) and not mainm.isShown() then
				chatboxw:set_visibility(true)
				local w = chatboxw:child("chattext")
				w:set_text("")
				w:focus()
				w:activate()
				clear_keybuf()
			end
		end)
	end
end

function common.initHUD(options)

	if not DEDSERV then
		options = options or {} 

-------crosshair------
		local testCrosshair = sprites_load("crosshair")
		local testCrosshair_m = sprites_load("crosshair_medium")
		local testCrosshair_l = sprites_load("crosshair_large")

		local testCrosshair_b = sprites_load("crosshair_bfg")
		local testCrosshair_f = sprites_load("crosshair_flamethrower")
		local testCrosshair_i = sprites_load("crosshair_ioncannon")
		local testCrosshair_n = sprites_load("crosshair_needler")
		local testCrosshair_p = sprites_load("crosshair_plasmarifle")
		local testCrosshair_r = sprites_load("crosshair_railgun")
		local testCrosshair_c = sprites_load("crosshair_chaingun")


-------crosshair------
		local killsMeter = sprites_load("gui")
		
		local hudFont = fonts.liero

		function bindings.wormRender(x, y, worm, viewport, ownerPlayer)

			local ownViewport = ( ownerPlayer == worm:player() )

			local bitmap = viewport:bitmap()

			if not ownViewport then
				if not options.hideEnemyHealth then
					gfx_set_alpha(90)
					bitmap:draw_box(x - 6, y - 17, x + 6, y - 16, color(148,240,245))
					gfx_reset_blending()
					local filled = (worm:health() / 100)
					bitmap:draw_box(x - 6, y - 17, x - 6 + filled * 12, y - 16, hudtools.greenRedInterpol(1-filled))
				end
				
				if not options.hideNames then
					fonts.minifont:render( bitmap, worm:player():name(), x, y - 10, color(148,240,245), Font.Formatting + Font.CenterH + Font.CenterV )
				end
				
			else
				if worm:is_changing() then
					fonts.minifont:render( bitmap, worm:current_weapon():type():name(), x, y - 10, color(148,240,245), Font.Formatting + Font.CenterH + Font.CenterV )
				end

----------------------------------------------crosshairs----------------------------
			if not worm:player():data().nocrosshair then
					local dx, dy = angle_vector(worm:angle(), 25)
					testCrosshair:render(bitmap, 0, x + dx, y + dy)
			end
			if worm:player():data().nocrosshair_medium then
					local dx, dy = angle_vector(worm:angle(), 25)
					testCrosshair_m:render(bitmap, 0, x + dx, y + dy)
			end
			if worm:player():data().nocrosshair_large then
					local dx, dy = angle_vector(worm:angle(), 25)
					testCrosshair_l:render(bitmap, 0, x + dx, y + dy)
			end
			if worm:player():data().crosshair_plasmarifle then
					local dx, dy = angle_vector(worm:angle(), 25)
					gfx_set_alpha(175)
					testCrosshair_p:render(bitmap, 0, x + dx, y + dy)
					gfx_reset_blending()
			end
			if worm:player():data().crosshair_needler then
					local dx, dy = angle_vector(worm:angle(), 25)
					gfx_set_alpha(175)
					testCrosshair_n:render(bitmap, 0, x + dx, y + dy)
					gfx_reset_blending()
			end
			if worm:player():data().crosshair_bfg then
					local dx, dy = angle_vector(worm:angle(), 25)
					testCrosshair_b:render(bitmap, 0, x + dx, y + dy)
			end
			if worm:player():data().crosshair_flamethrower then
					local dx, dy = angle_vector(worm:angle(), 25)
					gfx_set_alpha(175)
					testCrosshair_f:render(bitmap, 0, x + dx, y + dy)
					gfx_reset_blending()
			end
			if worm:player():data().crosshair_ioncannon then
					local dx, dy = angle_vector(worm:angle(), 25)
					testCrosshair_i:render(bitmap, 0, x + dx, y + dy)
			end
			if worm:player():data().crosshair_chaingun then
					local dx, dy = angle_vector(worm:angle(), 25)
					testCrosshair_c:render(bitmap, 0, x + dx, y + dy)
			end
			if worm:player():data().crosshair_railgun then
					local dx, dy = angle_vector(worm:angle(), 35)
					testCrosshair_r:render(bitmap, 0, x + dx, y + dy)
			end
			end
						
			--fonts.minifont:render( bitmap, worm:get_player():team(), x, y - 30, 0, 255, 0, Font.CenterH )
		end
		
		function bindings.viewportRender(viewport, worm)
			local bitmap = viewport:bitmap()
			local filled = (worm:health() / 100)
			local limit = 200-filled*2
			
			gfx_set_alpha(190)
			killsMeter:render(bitmap, 0, bitmap:w()-94, 1)
			bitmap:linewu(-1, 27,-94,27,color(83,125,138))
			gfx_reset_blending()

			fonts.minifont:render(bitmap, worm:player():kills(), bitmap:w()-21, 5, color(148,240,245), 0)
				
			if filled <= 0 then
				hudFont:render( bitmap, "Press [JUMP] to respawn!", bitmap:w()/2, 35, color(254, 217, 5), Font.Shadow + Font.CenterH )
			end
			
			--hudtools.drawHBar( bitmap, filled, bitmap:w()-100, 5, 75, 4 )
			fonts.minifont:render(bitmap, floor(worm:health()), bitmap:w()-83, 15, color(148,240,245),  0 )	

			local w = worm:current_weapon()
			if w:is_reloading() then
				filled = 1 - w:reload_time() / w:type():reload_time()
			else
				filled = w:ammo() / w:type():ammo()
			end
			hudtools.drawHBar( bitmap, filled, bitmap:w()-92, 3, 55, 3 )
			fonts.minifont:render(bitmap, w:ammo(), bitmap:w()-31, 16, color(148,240,245),  0 )
			fonts.minifont:render(bitmap, w:type():ammo(), bitmap:w()-18, 16, color(148,240,245),  0 )

				
		if worm:player():data().charge_on then
		filled = (worm:player():data().charge_on/ 100)
		hudtools.drawHBar(bitmap, filled, bitmap:w() - 92, 8, 45, 3)
		else
		hudtools.drawHBar(bitmap, 0, bitmap:w() - 92, 8, 45, 3)
end

		end

	end
end

function common.initWeaponSelection()
	if not DEDSERV then
		local maxSelectableWeapons = 5
		local selectWeaponStart = maxSelectableWeapons+1
		local selectWeaponCount = selectWeaponStart+1
		
		function bindings.viewportRender(viewport, worm)
			local bitmap = viewport:bitmap()
			local player = worm:player()
			if player and player:data().weaponSelection then
				local o = player:data().weaponSelection
				local y = (bitmap:h() - selectWeaponCount*10) / 2
				local x = (bitmap:w() - 60) / 2
				local i = 0
				
				local function drawRow(name)
					if i == o.cur then
						fonts.liero:render(
							bitmap,
							name,
							x, y,
							color(255,255,255), Font.Shadow)
					else
						fonts.liero:render(
							bitmap,
							name,
							x, y,
							color(148,240,245), Font.Shadow)
					end
					y = y + 10
					i = i + 1
				end
				
				drawRow("Randomize")
				
				while i <= maxSelectableWeapons do
					drawRow(o.list[i]:name())
				end
				
				drawRow("Start!")
			end
		end
		
		local function randomWeapons()
			local list = {}
			
			for i = 1, maxSelectableWeapons do
				local ok
				
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
			
			return list
		end
		
		function bindings.localplayerEvent(player, event, state)
			local o = player:data().weaponSelection
			if o then
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
						if event == Player.Left then
							o.list[o.cur] = o.list[o.cur]:prev()
						elseif event == Player.Right then
							o.list[o.cur] = o.list[o.cur]:next()
						end
					end
				else
					if event == Player.Fire then
						o.pressed = false
					end
				end
				
				return true -- Swallow all events if we're in weapon selection
			end
		end
		
		function bindings.playerUpdate(player)
		local o = player:data().weaponSelection
			if o and o.pressed then
				if o.cur == 0 then
					o.list = randomWeapons()
				elseif o.cur == selectWeaponStart then
					player:select_weapons(o.list)
					player:data().weaponSelection = nil
				else
					local ok
					repeat
						o.list[o.cur] = weapon_random()
						
						if weapon_count() < maxSelectableWeapons then
							break
						end
						
						ok = true
						for j = 1, maxSelectableWeapons do
							if j ~= o.cur and o.list[j] == o.list[o.cur] then
								ok = false
							end
						end
					until ok
				end
			end
		end

		function bindings.localplayerInit(player)
			player:data().weaponSelection =
			{
				list = randomWeapons(),
				cur = 1
			}
		end
	end
end

function common.init(options)
	common.initHUD(options)
	common.initChat(options)
	common.initScoreboard(options)
	common.initWeaponSelection(options)
	
	console_register_command("SHOWMENU", function()
		mainm.show()
	end)

	if not DEDSERV then
		mainm.init()
	end

end
