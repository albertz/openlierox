function mode.init()

	local ballOwner = nil
	local balltype = load_particle("ballpark/poop.obj")
	local ballCount = 0
	cg_autojoin = 0
	gameMap = "Unknown"
	
	--[[console_register_command("CG_AUTOJOIN",function(i)
		if i == nil then
			return "CG_AUTOJOIN IS: "..cg_autojoin.." DEFAULT: 0"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 1 and i >= 0 then
				cg_autojoin = i
			else
				return "CG_AUTOJOIN IS: "..cg_autojoin.." DEFAULT: 0"
			end
		end
	end)]]
	
	teams = {}
	
	table.insert(teams, {
		id = 0,
		name = "02RED00", 
		kills = 0, 
		deaths = 0, 
		suicides = 0,
		score = 0,
		stolen = false,
		owner,
		flagObject = load_particle("ctf/red/flag.obj"),
		baseObject = load_particle("ctf/red/base.obj"),
		vocCapture = load_particle("ctf/red/capture.obj"),
		vocReturn = load_particle("ctf/red/return.obj"),
		capture = load_particle("ctf/red/capture.obj"),
		alarm = load_particle("ctf/red/alarm.obj"),
    	flagReturn = load_particle("ctf/red/return.obj"),
		point = load_particle("domination/red.obj")
		
	})

	table.insert(teams, {
		id = 1,
		name = "04BLUE00", 
		kills = 0, 
		deaths = 0, 
		suicides = 0,
		score = 0,
		stolen = false,
		owner,
		flagObject = load_particle("ctf/blue/flag.obj"),
		baseObject = load_particle("ctf/blue/base.obj"),
		vocCapture = load_particle("ctf/blue/capture.obj"),
		vocReturn = load_particle("ctf/blue/return.obj"),
		capture = load_particle("ctf/blue/capture.obj"),
    	alarm = load_particle("ctf/blue/alarm.obj"),
    	flagReturn = load_particle("ctf/blue/return.obj"),
		point = load_particle("domination/blue.obj")
	})
		
	function mode.change(id, map)
		if id == nil then
			return "MODE IS: "..modes_[gameMode].." DEFAULT: ffa"
		elseif id ~= nil and map ~= nil then
			if AUTH or DEDSERV and id ~= "" and map ~= "" then
				if tostring(id) and tostring(map) then
					if string.lower(id) == "ffa" or string.lower(id) == "1v1" or string.lower(id) == "tdm" or string.lower(id) == "ctf" or string.lower(id) == "instagib" or string.lower(id) == "ballpark" or string.lower(id) == "ca" or string.lower(id) == "domination" then
						for m in maps() do
							if string.lower(map) == string.lower(m) then
								console.sv_team_play = modes[id]
								gameMap = string.lower(map)
								host(string.lower(map))
							end
						end
					else
						return "MODE IS: "..modes_[gameMode].." DEFAULT: ffa"
					end
				else
					return "MODE IS: "..modes_[gameMode].." DEFAULT: ffa"
				end
			else
				return "MODE IS: "..modes_[gameMode].." DEFAULT: ffa"
			end
		else
			return "MODE IS: "..modes_[gameMode].." DEFAULT: ffa"
		end
	end
		

	console_register_command("MODE",function(id, map)
		if id == nil then
			return "MODE IS: "..modes_[gameMode].." DEFAULT: ffa"
		elseif id ~= nil and map ~= nil then
			if AUTH or DEDSERV and id ~= "" and map ~= "" then
				if tostring(id) and tostring(map) then
					if string.lower(id) == "ffa" or string.lower(id) == "1v1" or string.lower(id) == "tdm" or string.lower(id) == "ctf" or string.lower(id) == "instagib" or string.lower(id) == "ballpark" or string.lower(id) == "ca" or string.lower(id) == "domination" then
						for m in maps() do
							if string.lower(map) == string.lower(m) then
								console.sv_team_play = modes[id]
								gameMap = string.lower(map)
								host(string.lower(map))
							end
						end
					else
						return "MODE IS: "..modes_[gameMode].." DEFAULT: ffa"
					end
				else
					return "MODE IS: "..modes_[gameMode].." DEFAULT: ffa"
				end
			else
				return "MODE IS: "..modes_[gameMode].." DEFAULT: ffa"
			end
		else
			return "MODE IS: "..modes_[gameMode].." DEFAULT: ffa"
		end
	end)
	
	function inGame()
		local count = 0
		
		for p in game_players() do
			if p:team() == 1 then
				count = count + 1
			end
		end
		return count
	end
	
	function inRedTeam()
		local count = 0
		
		for p in game_players() do
			if p:team() == 1 then
				count = count + 1
			end
		end
		return count
	end
	
	function inBlueTeam()
		local count = 0
		
		for p in game_players() do
			if p:team() == 2 then
				count = count + 1
			end
		end
		return count
	end

	function mode.message(message, worm)
		draw.message(message, 1, 300, worm, -110, color(255, 127, 0), 1, true, 15)
	end
	
	local timerStart = network_player_event("kts", function(self, p, data)
		p:stats().kotb_timer = data:get_int()
		p:data().kotb_timerRunning = true
		ballOwner = p:worm()
	end)
	
	local timerStop = network_player_event("ktsp", function(self, p, data)
		p:stats().kotb_timer = data:get_int()
		p:data().kotb_timerRunning = false
		if ballOwner == p:worm() then
			ballOwner = nil
		end
	end)

	function mode.bpCreateStuff(x, y)
		if ballCount == 0 then
			balltype:put(x, y)
			ballCount = 1
		end
	end
	
	function mode.bpBallThink(object, worm)
		if ballOwner and gameMode == 6 then
			object:set_pos(ballOwner:pos())
		end
	end
	
	local function pickupBall(worm)
		if not AUTH then return end
		
		if not ballOwner then
			local p = worm:player()
			local b = new_bitstream()
			b:add_int(p:stats().kotb_timer)
			timerStart:send(p, b)
			ballOwner = worm
			p:data().kotb_timerRunning = true
		end
	end
	
	function mode.bpSetOwner(object, worm)
		if gameMode == 6 then
			pickupBall(worm)
		end
	end
	
	local function dropBall(worm)
		if not AUTH then return end
		
		if ballOwner == worm then
			local p = worm:player()
			if p then
				local b = new_bitstream()
				b:add_int(p:stats().kotb_timer)
				timerStop:send(p, b)
				p:data().kotb_timerRunning = false
			end
			ballOwner = nil
		end
	end
			
	function bindings.playerNetworkInit(p, connID)
		local d = p:data()
		local b = new_bitstream()
		b:add_int(p:stats().kotb_timer)
		if d.kotb_timerRunning then
			timerStart:send(p, b, connID)
		else
			timerStop:send(p, b, connID)
		end
	end
	
	function bindings.playerInit(p)
		local d = p:data()
		
		d.canJoin = false
		d.team = 0
		d.kotb_timerRunning = false
		p:stats().kotb_timer = 0
		d.respawnTimer = 0
		d.lmsTimer = 0
		if AUTH and gameMode == 6 and ballCount == 0 then
			repeat
				ballX = randomint(1, 1000)
				ballY = randomint(1, 1000)
			until map_is_particle_pass(ballX, ballY)
			mode.bpCreateStuff(ballX, ballY)
			debug("Ball pos: "..ballX..", "..ballY)
		end
		
		if cg_autojoin == 1 then
			if isTeamPlay() then
				if inRedTeam() == inBlueTeam() then
					
				else
					if inRedTeam() > inBlueTeam() then
						console.p0_team = 2
						console.p1_team = 2
					else
						console.p0_team = 1
						console.p1_team = 1
					end
				end
			elseif gameMode == 2 then
				if inGame() < 2 then
					console.p0_team = 1
				end
			else
				console.p0_team = 1
				console.p1_team = 1
			end
		end
	end
		
	function bindings.playerUpdate(p)
		local d = p:data()
		local x,y = p:worm():pos()
		
		if p:worm():health() > 0 and not d.canJoin then
			--d.canJoin = false
			--p:worm():damage(100)
		end
		
		if p:worm():health() <= 0 then
			if d.canJoin then
				if d.respawnTimer < 500 then
					d.respawnTimer = d.respawnTimer + 1
				else
					d.respawnTimer = 0
					local hax = console["+p0_jump"]
				end
			end
		else
			d.respawnTimer = 0
		end
		
		--[[if not isTeamPlay() then
			if p:team() ~= 1 then
				d.canJoin = false
			end
		
			if p:worm():health() > 0 and p:team() ~= 1 then
				d.canJoin = false
				p:worm():damage(1000)
			end
		end
		
		if isTeamPlay() then
			if p:team() ~= 1 and p:team() ~= 2 then
				d.canJoin = false
			end
			if p:team() ~= 1 and p:team() ~= 2 and p:worm():health() > 0 then
				d.canJoin = false
				p:worm():damage(1000)
			end
			
			if p:worm():health() > 0 then
				if d.team ~= p:team() then
					p:worm():damage(1000)
				end

			end
		end]]
		
		d.team = p:team()
		
		if d.kotb_timerRunning and gameMode == 6 and gameEnd == 0 then
			p:stats().kotb_timer = p:stats().kotb_timer + 1
		end
		
		--[[if gameMode == 7 then
			if p:worm():health() > 0 then
				if not map_is_particle_pass(x, y + 5) then
					d.lmsTimer = d.lmsTimer + 1
				else
					d.lmsTimer = 0
				end
			end
	
			if d.lmsTimer == 33 then
				p:worm():damage(100)
			end
		end]]
	end
	
	function bindings.wormDeath(worm)
		if gameMode == 6 then
			dropBall(worm)
		end
	end
	
	function bindings.wormRemoved(worm)
		if gameMode == 6 then
			dropBall(worm)
		end
	end
	
	--utillity for bots
	function bindings.playerInit(player)
		local d = player:data()
		
		
		if string.sub(player:name(), 0, 3) == "bot" then
			if isSpectator(player) then
				console.kick = player:name()
				cecho("YOU SHOULD ADD BOT TO THE PLAYERS TEAM")
			end
		end
	end
	
	function bindings.localplayerEvent(player, event, state)
		local d = player:data()
		
		if not isTeamPlay() then
			if event == Player.Jump and state then
				if gameMode ~= 2 then
					d.canJoin = true
				else
					if player:team() == 1 and inGame() <= 2 then
						d.canJoin = true
					end
				end
				if player:worm():health() > 0 then
					if not d.canJoin or player:team() ~= 1 then
						d.canJoin = false
						player:worm():damage(1000)
					end
				end
			end
			if (event == Player.Jump and state) and (player:team() ~= 1 or not d.canJoin) then
				return true
			end
		end
		
		if isTeamPlay() then
			if player:team() == 1 or player:team() == 2 then
				d.canJoin = true
			end
			
			if player:worm():health() > 0 then
				if not d.canJoin or (player:team() ~= 1 or player:team() ~= 2) then
					--d.canJoin = false
				--	player:worm():damage(1000)
				end
			end
			
			if (event == Player.Jump and state) and ((player:team() < 1 and player:team() > 2) or not d.canJoin) then
				return true
			end
		end
	end
end
