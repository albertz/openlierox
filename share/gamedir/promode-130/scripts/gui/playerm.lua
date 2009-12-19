function playerm.init()
		-- Connect Menu
	gui_load_gss("menu-common")
	gui_load_gss("options-menu")

	local menu = gui_group({id = "options-menu"})
	local win = gui_window({id = "options-win"})

	function pname(i)
		local def = console["p"..i.."_name"]
		return def
	end

	function paccel(i)
		return console["p"..i.."_aim_accel"]
	end
		
	function pfriction(i)
		return console["p"..i.."_aim_friction"]
	end
	
	function pspeed(i)
		return console["p"..i.."_aim_speed"]
	end

	function viewport(p)
		local def = tonumber(console["p"..p.."_viewport_follow"])
		local val = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 0}
		local var = {}
		for i = 1,11 do
			table.insert(var, "-")
			if def == val[i] then
				var[i] = "|"
			end
		end
		return "Viewport: "..table.concat(var)
	end

	local player0Btn = gui_button({id = "playeropt0", label = "Player 0"})
	local player1Btn = gui_button({id = "playeropt1", label = "Player 1"})
	local applyBtn = gui_button({id = "apply", label = "Apply"})
	local backBtn = gui_button({id = "back", label = "Back"})

	local playerID = gui_label({id = "playerID", label = "Player 0"})
	local name0Lb = gui_label({id = "nameLb", label = "Name:"})
	local name0 = gui_edit({id = "name", label = pname(0)})
	local colorLb = gui_label({id = "colorLb", label = "Color:"})
	local red0 = gui_edit({id = "red", label = "100"})
	local green0 = gui_edit({id = "green", label = "100"})
	local blue0 = gui_edit({id = "blue", label = "200"})
	local team0 = gui_button({id = "team", label = "Team: None"})
	local accelLb = gui_label({id = "accelLb", label = "Aim accel:"})
	local frictionLb = gui_label({id = "frictionLb", label = "Aim friction:"})
	local speedLb = gui_label({id = "speedLb", label = "Aim speed:"})
	local p0accel = gui_edit({id = "accel", label = paccel(0)})
	local p0friction = gui_edit({id = "friction", label = pfriction(0)})
	local p0speed = gui_edit({id = "speed", label = pspeed(0)})
	local p0viewfollow = gui_button({id = "viewport", label = viewport(0)})

	local name1 = gui_edit({id = "name", label = pname(1)})
	local red1 = gui_edit({id = "red", label = "100"})
	local green1 = gui_edit({id = "green", label = "100"})
	local blue1 = gui_edit({id = "blue", label = "200"})
	local team1 = gui_button({id = "team", label = "Team: None"})
	local p1accel = gui_edit({id = "accel", label = paccel(1)})
	local p1friction = gui_edit({id = "friction", label = pfriction(1)})
	local p1speed = gui_edit({id = "speed", label = pspeed(1)})
	local p1viewfollow = gui_button({id = "viewport", label = viewport(1)})

	win:add( {player0Btn, player1Btn, backBtn, applyBtn, playerID, name0Lb, name0, colorLb, red0, green0, blue0, team0, accelLb, frictionLb, speedLb, p0accel, p0friction, p0speed, p0viewfollow, name1, red1, green1, blue1, team1, p1accel, p1friction, p1speed, p1viewfollow})
	menu:add( {win} )
	gui_root():add(menu)

	function playerm.isShown()
		return menu:is_visible()
	end

	function playerm.show()
		if not menu:is_visible() then
			menu:set_visibility(true)
			player0Tab(true)
			player1Tab(false)
			playerID:set_text("Player 0")
			menu:focus()
		end
	end

	function playerm.hide()
		menu:set_visibility(false)
	end

	function player0Tab(state)
		name0:set_visibility(state)
		red0:set_visibility(state)
		green0:set_visibility(state)
		blue0:set_visibility(state)
		team0:set_visibility(state)
		p0accel:set_visibility(state)
		p0friction:set_visibility(state)
		p0speed:set_visibility(state)
		p0viewfollow:set_visibility(state)
	end

	function player1Tab(state)
		name1:set_visibility(state)
		red1:set_visibility(state)
		green1:set_visibility(state)
		blue1:set_visibility(state)
		team1:set_visibility(state)
		p1accel:set_visibility(state)
		p1friction:set_visibility(state)
		p1speed:set_visibility(state)
		p1viewfollow:set_visibility(state)
	end

	function player0Btn:onAction()		player0Tab(true)
		player1Tab(false)
		playerID:set_text("Player 0")
	end

	function player1Btn:onAction()
		player0Tab(false)
		player1Tab(true)
		playerID:set_text("Player 1")
	end

	function team0:onAction()
		if team0:text() == "Team: None" then
			team0:set_text("Team: Blue")
		elseif team0:text() == "Team: Blue" then
			team0:set_text("Team: Red")
		elseif team0:text() == "Team: Red" then
			team0:set_text("Team: None")
		end
	end

	function team1:onAction()
		if team1:text() == "Team: None" then
			team1:set_text("Team: Blue")
		elseif team1:text() == "Team: Blue" then
			team1:set_text("Team: Red")
		elseif team1:text() == "Team: Red" then
			team1:set_text("Team: None")
		end
	end

	function p0viewfollow:onAction()
		local def = tonumber(console["p0_viewport_follow"])
		def = def + 0.1
		if def > 1 then
			def = 0
		end
		local val = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 0}
		local var = {}
		for i = 1,10 do
			table.insert(var, "-")
			if def == val[i] then
				var[i-1] = "|"
			else
				var[i-1] = "-"
			end
						 
		end
		p0viewfollow:set_text("Viewport: "..table.concat(var))
		console.p0_viewport_follow = def
	end

	function p1viewfollow:onAction()
		local def = tonumber(console["p1_viewport_follow"])
		def = def + 0.1
		if def > 1 then
			def = 0
		end
		local val = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 0}
		local var = {}
		for i = 1,10 do
			table.insert(var, "-")
			if def == val[i] then
				var[i-1] = "|"
			else
				var[i-1] = "-"
			end
						 
		end
		p1viewfollow:set_text("Viewport: "..table.concat(var))
		console.p1_viewport_follow = def
	end

	function applyBtn:onAction()
		local teams = {}
			teams["None"] = ""
			teams["Blue"] = "0"
			teams["Red"] = "1"

		if playerID:text() == "Player 0" then
			if pname(0) ~= name0:text() then
				console.p0_name = name0:text()
			end
			console.p0_team = teams[string.gsub(team0:text(), "Team: ", "")]
			
			if paccel(0) ~= p0accel:text() then
				console.p0_aim_accel = p0accel:text()
			end

			if pfriction(0) ~= p0friction:text() then
				console.p0_aim_friction = p0friction:text()
			end

			if pspeed(0) ~= p0speed:text() then
				console.p0_aim_speed = p0speed:text()
			end
		end

		if playerID:text() == "Player 1" then
			if pname(1) ~= name1:text() then
				console.p1_name = name1:text()
			end
			console.p1_team = teams[string.gsub(team1:text(), "Team: ", "")]
			
			if paccel(1) ~= p1accel:text() then
				console.p1_aim_accel = p1accel:text()
			end

			if pfriction(1) ~= p1friction:text() then
				console.p1_aim_friction = p1friction:text()
			end

			if pspeed(1) ~= p1speed:text() then
				console.p1_aim_speed = p1speed:text()
			end
		end	
	end

	function backBtn:onAction()
		playerm.hide()
		optionsm.show()
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC then
			playerm.hide()
			optionsm.show()
			return true
		end
	end

	playerm.hide()

end
