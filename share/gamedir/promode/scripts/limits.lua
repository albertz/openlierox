function limits.init()

	local onceFlag = 1
	local limitType = ""
	timelimit = 90000
	killslimit = 20
	deathslimit = 20
	local actualKills = 0
	local actualDeaths = 0
	local actualTimes = 0
	local gameEndSound = load_particle("game_end.obj")
	local ch_showtime = 1
	local screenDelay = 0
	local playersJoining = {}
	sv_maxclients = 8
	cg_autoscreenshot = 1
	
	function setLimitType(_type)
		if limitType == "" then
			limitType = _type
		end
	end
	
	console_register_command("CH_SHOWTIME",function(i)
		if i == nil then
			return "CH_SHOWTIME IS: "..ch_showtime.." DEFAULT: 1"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 1 and i >= 0 then
				ch_showtime = i
			else
				return "CH_SHOWTIME IS: "..ch_showtime.." DEFAULT: 1"
			end
		end
	end)
	
	console_register_command("CG_AUTOSCREENSHOT",function(i)
		if i == nil then
			return "CG_AUTOSCREENSHOT IS: "..cg_autoscreenshot.." DEFAULT: 1"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 1 and i >= 0 then
				cg_autoscreenshot = i
			else
				return "CG_AUTOSCREENSHOT IS: "..cg_autoscreenshot.." DEFAULT: 1"
			end
		end
	end)
	
	function bindings.viewportRender(viewport, worm)
		local bitmap = viewport:bitmap()
		local timeHeight = 30
		
		if ch_showtime == 1 and cg_draw2d == 1 then
			if worm:player() == game_local_player(0) then
				if console.cl_showfps == "1" and console.cl_showdebug == "1" then
					timeHeight = 30
				elseif console.cl_showfps == "1" and console.cl_showdebug == "0" then
					timeHeight = 11
				elseif console.cl_showfps == "0"  then
					timeHeight = 5
				end
				fonts.minifont:render( bitmap, "TIME:", 5, timeHeight, color(255, 255, 255), Font.Left)
				fonts.minifont:render( bitmap, string.sub(to_time_string(worm:player():stats().timer), 4, 9), 48, timeHeight + 2, color(0, 255, 0), Font.CenterV + Font.Right)
			end
		end
	end
	
	function bindings.playerInit(player)
		player:stats().timer = 0
	end
	
	function bindings.playerUpdate(player)
		if gameEnd > 0 then
		else
			player:stats().timer = player:stats().timer + 1
		end
	end

	function bindings.localplayerEvent(player, event, state)
		if gameEnd == 1 then
			if event == Player.Fire and state then
				return true
			end
			if event == Player.Change and state then
				return true
			end
			if event == Player.Jump and state then
				return true
			end
			if event == Player.Left and state then
				return true
			end
			if event == Player.Right and state then
				return true
			end
			if event == Player.Up and state then
				return true
			end
			if event == Player.Down and state then
				return true
			end
		end
	end

	function limits.hold(object,worm)
		local vx,vy = worm:spd()
		local gravity = console.sv_worm_gravity

		worm:set_spd(vx-vx,vy-vy-gravity)
	end

	local gameEnded = network_player_event("ge", function(self, player, game_end_data)
		gameEnd = 1
	end)

	function limits.MaxTimes()
		for p in game_players() do
			if gameMode == 6 then
				if p:stats().kotb_timer > actualTimes then
					actualTimes = p:stats().kotb_timer
				end
			else
				if p:stats().timer > actualTimes then
					actualTimes = p:stats().timer
				end
			end
		end
		return actualTimes
	end

	function limits.MaxKills()
		for p in game_players() do
			if p:kills() > actualKills then
				actualKills = p:kills()
			end
		end
		return actualKills
	end

	function limits.MaxDeaths()
		for p in game_players() do
			if p:deaths() > actualDeaths then
				actualDeaths = p:deaths()
			end
		end
		return actualDeaths
	end

	local maxTimeCount = limits.MaxTimes()
	local maxKillCount = limits.MaxKills()
	local maxDeathCount = limits.MaxDeaths()

	function bindings.playerUpdate(player)
		local teamKills = {}
			teamKills[1] = 0
			teamKills[2] = 0
			
		local teamDeaths = {}
			teamDeaths[1] = 0
			teamDeaths[2] = 0
		local worm = player:worm()

		maxTimeCount = limits.MaxTimes()
		maxKillCount = limits.MaxKills()
		maxDeathCount = limits.MaxDeaths()
		
		if gameMode == 4 then
			teamKills[1] = teamKills[1] + (teams[1].score*5)
			teamKills[2] = teamKills[2] + (teams[2].score*5)
		end

		if gameEnd > 0 then
			if onceFlag == 1 then
				stats.update(player)
				stats.save()
				gameEndSound:put(1,1)
				onceFlag = 0
				addMessage(limitType)
			else
				if worm:health()<100 then
					worm:damage(-(100-worm:health()))
				end
			end
			
			if screenDelay == 10 then
				if not DEDSERV and cg_autoscreenshot == 1 then
					hax = console["screenshot"]
				end
			end
			
			if screenDelay <= 10 then
				screenDelay = screenDelay + 1
			end
		end

		if maxTimeCount >= timelimit and timelimit > 0 then
			if AUTH then
				gameEnd = 1
				gameEnded:send(player)
			end
			setLimitType("Timelimit hit")
		end
		
		if gameMode ~= 3 and gameMode ~= 4 and gameMode ~= 7 and gameMode ~= 8 then
			if maxKillCount >= killslimit and killslimit > 0 then
				if AUTH then
					gameEnd = 1
					gameEnded:send(player)
				end	
				setLimitType("Killslimit hit")
			end
	
			if maxDeathCount >= deathslimit and deathslimit > 0 then
				if AUTH then
					gameEnd = 1
					gameEnded:send(player)
				end
				setLimitType("Deathslimit hit")
			end
		else
			for p in game_players() do
				if p:team() == 1 then
					if gameMode ~= 7 and gameMode ~= 8 then
						teamKills[1] = teamKills[1] + p:kills()
						teamDeaths[1] = teamDeaths[1] + p:deaths()
					end
					
					if gameMode == 8 then
						teamKills[1] = teamKills[1] + teams[1].score
					end
				end
				if p:team() == 2 then
					if gameMode ~= 7 and gameMode ~= 8 then
						teamKills[2] = teamKills[2] + p:kills()
						teamDeaths[2] = teamDeaths[2] + p:deaths()
					end
					
					if gameMode == 8 then
						teamKills[2] = teamKills[2] + teams[2].score
					end
				end
			end
			for i = 1,2 do
				if teamKills[i] ~= nil and teamKills[i] >= killslimit and killslimit > 0 then
					if AUTH then
						gameEnd = 1
						gameEnded:send(player)
					end
					if mode == 4 then
						setLimitType("Scorelimit hit")
					else
						setLimitType("Killslimit hit")
					end
				end
				if teamDeaths[i] ~= nil and teamDeaths[i] >= killslimit and killslimit > 0 then
					if AUTH then
						gameEnd = 1
						gameEnded:send(player)
					end
					setLimitType("Deathslimit hit")
				end
			end
		end
	end

	console_register_command("SV_TIMELIMIT", function(i)
			if i == nil then
				return "SV_TIMELIMIT IS: "..(timelimit/6000).." DEFAULT: 15"
			end
	
			if i ~= nil and AUTH then
				if tonumber(i) then
					local i = i*6000
					timelimit =  i
				else
					return "SV_TIMELIMIT: a number is required"
				end
			end
	end)

	console_register_command("SV_KILLSLIMIT", function(i)
		if i == nil then
			return "SV_KILLSLIMIT IS: "..(killslimit).." DEFAULT: 20"
		end

		if i ~= nil and AUTH then
			if tonumber(i) then
				local i = i*1
				killslimit = i
			else
				return "SV_KILLSLIMIT: a number is required"
			end
		end
	end)

	console_register_command("SV_DEATHSLIMIT", function(i)
		if i == nil then
			return "SV_DEATHSLIMIT IS: "..(deathslimit).." DEFAULT: 20"
		end

		if i ~= nil and AUTH then
			if tonumber(i) then
				local i = i*1
				deathslimit = i
			else
				return "SV_DEATHSLIMIT: a number is required"
			end
		end
	end)
	
	function bindings.playerInit(player)
		if not AUTH then return end
		
		table.insert(playersJoining, {player})
	end

	function bindings.afterUpdate()
		if AUTH and playersJoining then
			if table.getn(playersJoining) > 0 then
				table.foreach(playersJoining, function(i, _table)
						if sv_maxclients > 1 then
							local players = {}

							for player in game_players() do
								table.insert(players, player)
							end
						
							if sv_maxclients < table.getn(players) then
								for i = table.getn(players), sv_maxclients+1, -1 do
									if players[i] then
										console.kick = _table[1]:name()
										table.remove(players, i)
									end
								end
							end
						end
						table.remove(playersJoining, i)
					return
				end)
			end
		end
	end
	
	console_register_command("SV_MAXCLIENTS", function(i)
		if i == nil then
			return 'SV_MAXCLIENTS IS: '..sv_maxclients..' DEFAULT: 8'
		end
		
		if i ~= nil and AUTH then
			if tonumber(i) then
				local i = i*1
				sv_maxclients = i
				if i <= 1 then
					sv_maxclients = -1
				end
			else
				return "SV_MAXCLIENTS: a number is required"
			end
		end
	end)
	
end
