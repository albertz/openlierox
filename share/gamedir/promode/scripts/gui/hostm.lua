function hostm.init()
		-- Connect Menu
	gui_load_gss("menu-common")
	gui_load_gss("host-menu")
	
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
		
	_currentMode = {}
		_currentMode["Free for all"] = "ffa"
		_currentMode["Tournament"] = "1v1"
		_currentMode["Team deathmatch"] = "tdm" 
		_currentMode["Capture the flag"] = "ctf"
		_currentMode["Instagib"] = "instagib"
		_currentMode["Ballpark"] = "ballpark"
		_currentMode["Clan Arena"] = "ca"
		_currentMode["Domination"] = "domination"
	
	
	local selMode = 1

	local menu = gui_group({id = "host-menu"})
	local win = gui_window({id = "host-win"})
	local bots = 0

	function sname()
		return console["net_server_name"]
	end

	function sport()
		return console["net_server_port"]
	end

	function sdesc()
		return console["net_server_desc"]
	end
	
	
	local hostBtn = gui_button({id = "host", label = "Host"})
	local backBtn = gui_button({id = "back", label = "Back"})
	local splitCheck = gui_check({id = "splitscreen_", label = "Splitscreen"})
	local registerCheck = gui_check({id = "register_", label = "Register online"})
	local mapList = gui_list({id = "map_list"})
	local modeList = gui_list({id = "mode_list"})
	local serverNameLb = gui_label({id = "serverNameLb", label = "Name:"})
	local serverName = gui_edit({id = "serverName", label = sname()})
	local serverPort = gui_edit({id = "serverPort", label = sport()})
	local serverPortLb = gui_label({id = "serverPortLb", label = "Port:"})
	local descLb = gui_label({id = "descLb", label = "Description:"})
	local desc = gui_edit({id = "desc", label = sdesc()})
	local killsLb = gui_label({id = "killsLb", label = "Kills limit:"})
	local deathsLb = gui_label({id = "deathsLb", label = "Deaths limit:"})
	local timeLb = gui_label({id = "timeLb", label = "Time limit:"})
	local kills = gui_edit({id = "kills", label = killslimit})
	local deaths = gui_edit({id = "deaths", label = deathslimit})
	local time = gui_edit({id = "time", label = timelimit})
	local ircCheck = gui_check({id = "irc", label = "Irc bot"})
	local clientsLb = gui_label({id = "clientsLb", label = "Max clients:"})
	local slot1 = gui_button({id = "slot1", label = console["p0_name"]})
	slot2 = gui_button({id = "slot2", label = "Open"})
	slot3 = gui_button({id = "slot3", label = "Open"})
	slot4 = gui_button({id = "slot4", label = "Open"})
	slot5 = gui_button({id = "slot5", label = "Open"})
	slot6 = gui_button({id = "slot6", label = "Open"})
	slot7 = gui_button({id = "slot7", label = "Open"})
	slot8 = gui_button({id = "slot8", label = "Open"})
	local slotLb = {}
	for i = 1, 8 do
		slotLb[i] = gui_label({id = "slot"..i.."Lb", label = "Slot "..i..":"})
	end

	win:add( {backBtn, hostBtn, splitCheck, registerCheck, mapList, serverNameLb, serverName, serverPort, serverPortLb, descLb, desc, killsLb, deathsLb, timeLb, ircCheck, kills, deaths, time, modeList})
	menu:add( {win} )
	gui_root():add(menu)

	mapList:add_column("Map name", 1)
	modeList:add_column("Mode", 1)

	function hostm.isShown()
		return menu:is_visible()
	end

	function hostm.show(newCallBack)
		if not menu:is_visible() then
			menu:set_visibility(true)
			menu:focus()
		end

		mapList:clear()
		for m in maps() do
			if (m ~= "blank") then
				mapList:insert(m)
			end
		end
		callBack = newCallBack
		modeList:clear()
		for i = 1, 6 do
			modeList:insert(currentMode[i])
		end
		if console.cl_splitscreen ~= "0" then
			splitCheck:set_state( true )
		else
			splitCheck:set_state( false )
		end
		if console.net_register ~= "0" then
			registerCheck:set_state( true )
		else
			registerCheck:set_state( false )
		end
	end

	function hostm.hide()
		menu:set_visibility(false)
	end

	function slot2:onAction()
		if slot2:text() == "Open" then
			slot2:set_text("Bot")
			bots = bots + 1
		elseif slot2:text() == "Bot" then
			slot2:set_text("...")
			bots = bots - 1
			sv_maxclients = sv_maxclients - 1
			if bots < 0 then
				bots = 0
			end
		elseif slot2:text() == "..." then
			slot2:set_text("Open")
			sv_maxclients = sv_maxclients + 1
		end
	end

	function slot3:onAction()
		if slot3:text() == "Open" then
			slot3:set_text("Bot")
			bots = bots + 1
		elseif slot3:text() == "Bot" then
			slot3:set_text("...")
			bots = bots - 1
			sv_maxclients = sv_maxclients - 1
			if bots < 0 then
				bots = 0
			end
		elseif slot3:text() == "..." then
			slot3:set_text("Open")
			sv_maxclients = sv_maxclients + 1
		end
	end

	function slot4:onAction()
		if slot4:text() == "Open" then
			slot4:set_text("Bot")
			bots = bots + 1
		elseif slot4:text() == "Bot" then
			slot4:set_text("...")
			bots = bots - 1
			sv_maxclients = sv_maxclients - 1
			if bots < 0 then
				bots = 0
			end
		elseif slot4:text() == "..." then
			slot4:set_text("Open")
			sv_maxclients = sv_maxclients + 1
		end
	end

	function slot5:onAction()
		if slot5:text() == "Open" then
			slot5:set_text("Bot")
			bots = bots + 1
		elseif slot5:text() == "Bot" then
			slot5:set_text("...")
			bots = bots - 1
			sv_maxclients = sv_maxclients - 1
			if bots < 0 then
				bots = 0
			end
		elseif slot5:text() == "..." then
			slot5:set_text("Open")
			sv_maxclients = sv_maxclients + 1
		end
	end

	function slot6:onAction()
		if slot6:text() == "Open" then
			slot6:set_text("Bot")
			bots = bots + 1
		elseif slot6:text() == "Bot" then
			slot6:set_text("...")
			bots = bots - 1
			sv_maxclients = sv_maxclients - 1
			if bots < 0 then
				bots = 0
			end
		elseif slot6:text() == "..." then
			slot6:set_text("Open")
			sv_maxclients = sv_maxclients + 1
		end
	end

	function slot7:onAction()
		if slot7:text() == "Open" then
			slot7:set_text("Bot")
			bots = bots + 1
		elseif slot7:text() == "Bot" then
			slot7:set_text("...")
			bots = bots - 1
			sv_maxclients = sv_maxclients - 1
			if bots < 0 then
				bots = 0
			end
		elseif slot7:text() == "..." then
			slot7:set_text("Open")
			sv_maxclients = sv_maxclients + 1
		end
	end

	function slot8:onAction()
		if slot8:text() == "Open" then
			slot8:set_text("Bot")
			bots = bots + 1
		elseif slot8:text() == "Bot" then
			slot8:set_text("...")
			bots = bots - 1
			sv_maxclients = sv_maxclients - 1
			if bots < 0 then
				bots = 0
			end
		elseif slot8:text() == "..." then
			slot8:set_text("Open")
			sv_maxclients = sv_maxclients + 1
		end
	end

	function hostBtn:onAction()
		if desc:text() ~= console["net_server_desc"] then
			console.net_server_desc = desc:text()
		end
		if kills:text() ~= killslimit then
			killslimit = tonumber(kills:text())
		end
		if deaths:text() ~= deathslimit then
			deathslimit = tonumber(deaths:text())
		end
		if time:text() ~= timelimit then
			timelimit = tonumber(time:text())
		end
		if modeList:selection() and mapList:selection() then
			gameMap = mapList:selection()
			mode.change(_currentMode[modeList:selection()], mapList:selection())
		end
	end

	function backBtn:onAction()
		hostm.hide()
		connectm.show()
	end

	function splitCheck:onAction()
		if splitCheck:state() then
			console.cl_splitscreen = 1
		else
			console.cl_splitscreen = 0
		end
	end

	function registerCheck:onAction()
		if registerCheck:state() then
			console.net_register = 1
		else
			console.net_register = 0
		end
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC then
			hostm.hide()
			connectm.show()
			if callBack then
				callBack()
			end
			return true
		end
	end

	hostm.hide()

end
