function promode.init()
	ver = "1.30"
	debugState = false
	gameEnd = 0
	darkMode = false
	ctfMode = false
	sv_password = ""
	wrongPassword = false
	tooManyPlayers = false
	
	console.p0_team = 0
	console.p1_team = 0
	
	modes = {}
		modes["ffa"] = 1
		modes["1v1"] = 2
		modes["tdm"] = 3
		modes["ctf"] = 4
		modes["instagib"] = 5
		modes["ballpark"] = 6
		modes["ca"] = 7
		modes["domination"] = 8
		
	modes_ = {}
		modes_[0] = "ffa"
		modes_[1] = "ffa"
		modes_[2] = "1v1"
		modes_[3] = "tdm"
		modes_[4] = "ctf"
		modes_[5] = "instagib"
		modes_[6] = "ballpark"
		modes_[7] = "ca"
		modes_[8] = "domination"
		
	console_register_command("SV_PASSWORD",function(i)
		if AUTH or DEDSERV then
			if i == nil then
				return "SV_PASSWORD: "..sv_password.." DEFAULT: "
			elseif i ~= nil then
				sv_password = i
			end
		end
	end)
	
	--tables for statistic syncing hax
	weaponsNumber = {}
	weaponsName = {}
	--
	weaponsList = {"Bazooka", "Chaingun", "Doomsday", "Flak Cannon", "Grenade Launcher", "Gauss Gun", "Grenade", "Lightning Gun", "Mortar", "Shotgun", "Throwing Knife", "Winchester", "Autocannon"}

	function bindings.playerInit(p)
		local d = p:data()
		
		d.hits = {}
		d.atts = {}
		d.awards = {}
	
		for i = 1,table.getn(weaponsList) do
			d.hits[weaponsList[i]] = 0
			d.atts[weaponsList[i]] = 0
			--d.hitz[i] = 0
		end
		
		
		d.hits["Rifle "] = 0
		d.atts["Rifle "] = 0
	
		for i = 0,2 do
			d.awards[i] = 0
		end
	end

	if AUTH then
		gameMode = tonumber(console.sv_team_play)
		if gameMode == 0 then
			gameMode = 1
		end
	else
		gameMode = 0
	end
		
	function debug(string)
		if debugState then
			print(string)
		end
	end
	
	local netMode = network_game_event("nm", function(self, data)
		local serverMode = data:get_int()
		local serverPassword = data:get_string()
		
		if serverMode ~= gameMode then
			debug("Server game mode: "..serverMode..", client game mode "..gameMode)
			gameMode = serverMode
			debug("Update: Server game mode: "..serverMode..", client game mode "..gameMode)
			weaponsel.init()
			mainm.init()
			teamm.init()
			spectator.init()
			stats.init()
			----skins.init()
			
			console.exec = "promode.cfg"
			if serverPassword ~= "" then
				if console.rcon_password ~= serverPassword then
					wrongPassword = true
					hax = console["disconnect"]
				end
			end
		end
	end)
    
	function promode.sendMode(clients)
		if not clients then 
			clients = 0 
		end
	
		local data = new_bitstream()
		data:add_int(gameMode)
		data:add_string(sv_password)
	
		netMode:send(data, clients, SendMode.ReliableOrdered, RepRule.Auth2All)
	end
	
	function bindings.playerNetworkInit(newb, connID)
		if AUTH then
			promode.sendMode(connID)
		end
	end
	
	function promode.svsay(msg)
		if AUTH and DEDSERV then
			game_local_player(0):say("02"..string.upper(msg).."00")
		end
	end
	
	config.init()
	chat.init()
	limits.init()
	mode.init()
	damage.init()
	scoreboard.init()
	hud.init()
	ircbot.init()
	weaponeffects.init()
	effects.init()
	draw.init()
	vote.init()
	weaponsyncing.init()
	awards.init()
	skins.init()
	anticamp.init()
	
	console_register_command("SHOWMENU", function()
		mainm.show()
	end)
	
	console_register_command("SHOWTEAMMENU", function()
		teamm.show()
	end)
	
	console_register_command("VERSION", function()
		return "06"..ver.."00"
	end)

	if AUTH then
		
		if not DEDSERV then
			mainm.init()
			teamm.init()
		end
		weaponsel.init()
		spectator.init()
		stats.init()
		netauth.init()
		----skins.init()
		console.exec = "promode.cfg"
		--console.net_server_desc = console.net_server_desc.."|"..string.upper(modes_[gameMode]).."|"..tostring(sv_maxclients)
	end

	if not DEDSERV then
		if map_is_loaded() then
			draw.message(0, fonts.liero, "Gusanos Promode "..ver, 160, 170, 500, 0, 254, 254)
			draw.message(1, fonts.liero, "http://promode.pordesign.eu/", 160, 179, 500, 255, 255, 0)
		end
		cecho("05Loaded PROMODE client version 06"..ver.."00")
	end
	
	console_register_command("PLAYER_LIST", function()
		local pcount = 0
		local players = {}
		for p in game_players() do
			pcount = pcount + 1
			cecho(pcount..". "..p:name())
			--table.insert(players, p:name())
		end
	end)
	
	console_register_command("PLAYER_KICK", function(i)
		local players = {}
		for p in game_players() do
			table.insert(players, p:name())
		end
		
		i = i*1
		
		if AUTH then
			if players[i] ~= "" then
				console.kick = players[i]
			end
		else
			--console.callvote = "kick "..players[i]
		end
	end)
	
	function isTeamPlay()
		if gameMode == 3 or gameMode == 4 or gameMode == 7 or gameMode == 8 then
			return true
		else
			return false
		end
	end
	
	function isSpectator(p)
		if gameMode == 3 or gameMode == 4 or gameMode == 7 or gameMode == 8 then
			if p:team() == 1 or p:team() == 2 then
				return false
			else
				return true
			end
		else
			if p:team() == 1 then
				return false
			else
				return true
			end
		end
	end
	
	if DEDSERV then
		dedserv.init()
	end
	
	if AUTH or DEDSERV then
		console.exec = "cfgs/"..modes_[gameMode]..".cfg"
	end
	
	--[[ idd test ]]

	--[[local weapons = {}
	weapons["Autocannon"] = 250
	weapons["Bazooka"] = 280
	weapons["Chaingun"] = 220
	weapons["Doomsday"] = 357
	weapons["Flak Cannon"] = 200
	weapons["Grenade Launcher"] = 320
	weapons["Gauss Gun"] = 300
	weapons["Grenade"] = 220
	weapons["Rifle"] = 160
	weapons["Lightning Gun"] = 286
	weapons["Mortar"] = 290
	weapons["Shotgun"] = 150 
	weapons["Throwing Knife"] = 80
	weapons["Winchester"] = 175
	
	
	function bindings.playerInit(p)
		local d = p:data()
		
		d.weapons = {}
		d.weapons["Autocannon"] = 250
		d.weapons["Bazooka"] = 280
		d.weapons["Chaingun"] = 220
		d.weapons["Doomsday"] = 357
		d.weapons["Flak Cannon"] = 200
		d.weapons["Grenade Launcher"] = 320
		d.weapons["Gauss Gun"] = 300
		d.weapons["Grenade"] = 220
		d.weapons["Rifle"] = 160
		d.weapons["Lightning Gun"] = 286
		d.weapons["Mortar"] = 290
		d.weapons["Shotgun"] = 150 
		d.weapons["Throwing Knife"] = 80
		d.weapons["Winchester"] = 175
	end
	
	function bindings.playerUpdate(p)
		local d = p:data()
		local w = p:worm():current_weapon()
		
		if p:worm():health() > 0 then
			if d.weapons[w:type():name()] > 0 then
				d.weapons[w:type():name()] = d.weapons[w:type():name()] - 1
			end
		end
	end
	
	function bindings.wormDeath(worm)
		local d = worm:player():data()
		
		d.weapons["Autocannon"] = 250
		d.weapons["Bazooka"] = 280
		d.weapons["Chaingun"] = 220
		d.weapons["Doomsday"] = 357
		d.weapons["Flak Cannon"] = 200
		d.weapons["Grenade Launcher"] = 320
		d.weapons["Gauss Gun"] = 300
		d.weapons["Grenade"] = 220
		d.weapons["Rifle"] = 160
		d.weapons["Lightning Gun"] = 286
		d.weapons["Mortar"] = 290
		d.weapons["Shotgun"] = 150 
		d.weapons["Throwing Knife"] = 80
		d.weapons["Winchester"] = 175
	end
	
	function bindings.localplayerEvent(player, event, state)
		local d = player:data()
		local w = player:worm():current_weapon()
		
		if (event == Player.Fire and state) and (d.weapons[w:type():name()] > 0) then
			return true
		end
	end]]
	
	function is_reloading(player, weapon)
		--[[if player:data().weapons[weapon] > 0 then
			return true
		else
			return false
		end]]
		return false
	end
	
	function reload_time(player, weapon)
		--return player:data().weapons[weapon]
		return 0
	end
	--[[ idd test]]
	debug("Game mode: "..gameMode)
end
