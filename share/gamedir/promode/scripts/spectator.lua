function spectator.init()
	if not DEDSERV then
		ch_spectator = 1
		
		console_register_command("CH_SPECTATOR",function(i)
			if i == nil then
				return "CH_SPECTATOR IS: "..ch_spectator.." DEFAULT: 1"
			elseif i ~= nil then
				local i = i *1
				if tonumber(i) and i <= 1 and i >= 0 then
					ch_spectator = i
				else
					return "CH_SPECTATOR IS: "..ch_spectator.." DEFAULT: 1"
				end
			end
		end)
	
		currentMode = {}
			currentMode[0] = "Free for all"
			currentMode[1] = "Free for all"
			currentMode[2] = "Tournament"
			currentMode[3] = "Team deathmatch"
			currentMode[4] = "Capture the flag"
			currentMode[5] = "Instagib"
			currentMode[6] = "Ballpark"
			currentMode[7] = "Clan Arena"
			currentMode[8] = "Domination"
	
		function bindings.playerInit(player)
			if game_local_player(0) then
				game_local_player(0):data().movement = {left=false,right=false,up=false,down=false,fire=false,change=false,jump=false}
			end
			if game_local_player(1) then
				game_local_player(1):data().movement = {left=false,right=false,up=false,down=false,fire=false,change=false,jump=false}
			end
		end

		function bindings.localplayerEvent(player, event, state)
			if player:worm():health() <= 0 and player:data().weaponSelection then
				if player:data().weaponSelection.finished == 1 and player:data().movement then
					local x, y = player:worm():pos()
					if state then
						if event == Player.Up then
							player:data().movement.up = true
						end
						if event == Player.Down then
							player:data().movement.down = true
						end
						if event == Player.Left then
							player:data().movement.left = true
						end
						if event == Player.Right then
							player:data().movement.right = true
						end
						if event == Player.Fire then
							player:data().movement.fire = true
						end
					else
						if event == Player.Up then
							player:data().movement.up = false
						end
						if event == Player.Down then
							player:data().movement.down = false
						end
						if event == Player.Left then
							player:data().movement.left = false
						end
						if event == Player.Right then
							player:data().movement.right = false
						end
						if event == Player.Fire then
							player:data().movement.fire = false
						end
					end
				end
			end
		end
			
		function bindings.afterUpdate()
			for i=0, 1 do
				local player = game_local_player(i)
				if player then
					if player:worm():health() <= 0  and player:data().weaponSelection then
						if player:data().weaponSelection.finished == 1 and player:data().movement and (isSpectator(player) or not player:data().canJoin) then
							local x, y = player:worm():pos()
							local vx, vy = player:worm():spd()
							local map_x = 0
							local map_y = 0
							local map_w = 2147483647
							local map_h = 2147483647
	
							if player:data().movement.up then
								y = y-1
							end
							if player:data().movement.down then
								y = y+1
							end
							if player:data().movement.left then
								x = x-1
							end
							if player:data().movement.right then
								x = x+1
							end
							if not player:worm():data().spectateTarget then
								player:worm():set_pos(x +vx, y + vy)
							end
						end
					end
				end
			end
		end
			
		function bindings.localplayerEvent(player, event, state)
			if player:worm():health() <= 0 and player:data().weaponSelection and player:data().weaponSelection.finished == 1 and (isSpectator(player) or not player:data().canJoin) then
				if event == Player.Change and state then
					spectator.spectateCycle(player, "change")
				elseif event == Player.Fire and state then
					spectator.spectateCycle(player, "fire")
				end
				
			end
		end
	
		function spectator.spectateCycle(player, direction)
			local players = {}
			for p in game_players() do
				table.insert(players, p)
			end
			if not player:data().spectateTarget then
				player:data().spectateNumber = randomint(1,#players)
				if players[player:data().spectateNumber] == player then
					player:data().spectateNumber = player:data().spectateNumber +1
				end
				if player:data().spectateNumber > #players then
					player:data().spectateNumber = 1
				end
				player:data().spectateTarget = players[player:data().spectateNumber]
			else
				if direction == "change" then
					player:data().spectateNumber = player:data().spectateNumber +1
					if player:data().spectateNumber > #players then
						player:data().spectateNumber = 1
					end
				elseif direction == "fire" then
					player:data().spectateNumber = 1
				end
				player:data().spectateTarget = players[player:data().spectateNumber]
			end
		end
	
		function bindings.viewportRender(viewport, worm)
			local selectedWeapon = worm:current_weapon():type():name()
			local bitmap = viewport:bitmap()
			local p = worm:player()
			local d = p:data()
			local filled = (worm:health() / 200)
			local player = worm:player()
	
			if (isSpectator(player) or not p:data().canJoin) and cg_draw2d == 1 then
				if filled <= 0 and player:data().weaponSelection and player:data().weaponSelection.finished == 1 and ch_spectator == 1 then
					gfx_set_alpha(90)
						bitmap:draw_box(1, 1, 320, 24, color(0, 0, 0))
						bitmap:draw_box(1, 216, 320, 240, color(0, 0, 0))
					gfx_reset_blending()
					fonts.liero:render( bitmap, "06SPECTATOR: 05Press 00F4 05 to join ", bitmap:w()/2, 14, color(255, 255, 255),  Font.Formatting + Font.Shadow + Font.CenterH)
	
					if not worm:player():data().spectateTarget then
							fonts.liero:render( bitmap, "06FREEFLOAT: 05Hit 00CHANGE 05to go to ChaseCam" , bitmap:w()/2, 6, color(255,255,255), Font.Formatting + Font.Shadow + Font.CenterH )
					else
						if worm:player():data().spectateTarget:worm() then
							if worm:player():data().spectateTarget:worm():health() <= 0 then
		
							else
								worm:set_pos(worm:player():data().spectateTarget:worm():pos())
							end
						end
						if worm:player():data().spectateTarget == worm:player() then
							fonts.liero:render( bitmap, "06FREEFLOAT: 05Hit 00CHANGE 05to go to ChaseCam" , bitmap:w()/2, 6, color(255,255,255), Font.Formatting + Font.Shadow + Font.CenterH )
						else
							fonts.liero:render( bitmap, "00Following "..worm:player():data().spectateTarget:name() , bitmap:w()/2, 35, color(255,255,255), Font.Formatting + Font.Shadow + Font.CenterH )
							fonts.liero:render( bitmap, "06CHASECAM: 05Hit 00CHANGE 05to cycle. 00FIRE 05to go to FreeFloat" , bitmap:w()/2, 5, color(255,255,255), Font.Formatting + Font.Shadow + Font.CenterH )						
						end
					end
					fonts.bigchars:render( bitmap, "SPECTATOR", bitmap:w()/2, 223, color(255, 255, 0), Font.Formatting + Font.CenterH )					
				end
			else
				if filled <= 0 and player:data().weaponSelection and player:data().weaponSelection.finished == 1 and ch_spectator == 1 and cg_draw2d == 1 then
					gfx_set_alpha(90)
						bitmap:draw_box(1, 1, 320, 24, color(0, 0, 0))
						bitmap:draw_box(1, 216, 320, 240, color(0, 0, 0))
					gfx_reset_blending()
					fonts.liero:render( bitmap, "06GAME: 05Hit 00JUMP 05to respawn" , bitmap:w()/2, 6, color(255,255,255), Font.Formatting + Font.Shadow + Font.CenterH )
					fonts.liero:render( bitmap, "06GAME: 05Hit 00F4 05 to spectate", bitmap:w()/2, 14, color(255, 255, 255),  Font.Formatting + Font.Shadow + Font.CenterH)
					--fonts.liero:render( bitmap, "Press JUMP to respawn", bitmap:w()/2, 120, color(255, 255, 255), Font.Formatting + Font.CenterH )
					fonts.bigchars:render( bitmap, string.lower(currentMode[gameMode]), bitmap:w()/2, 223, color(255, 255, 0), Font.Formatting + Font.CenterH )					
				end
			end
		end
	end
end
