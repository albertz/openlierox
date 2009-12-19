function controlsm.init()
		-- Connect Menu
	gui_load_gss("menu-common")
	gui_load_gss("options-menu")

	local menu = gui_group({id = "options-menu"})
	local win = gui_window({id = "options-win"})

	local moveBtn = gui_button({id = "system-graphics", label = "Move"})
	local shootBtn = gui_button({id = "system-sound", label = "Shoot"})
	local miscBtn = gui_button({id = "system-network", label = "Misc"})

	local applyBtn = gui_button({id = "apply", label = "Apply"})
	local backBtn = gui_button({id = "back", label = "Back"})

	upEd = gui_edit({id = "upEd"})
	downEd = gui_edit({id = "downEd"})
	leftEd = gui_edit({id = "leftEd"})
	rightEd = gui_edit({id = "rightEd"})
	fireEd = gui_edit({id = "fireEd"})
	changeEd = gui_edit({id = "changeEd"})
	jumpEd = gui_edit({id = "jumpEd"})
	up1Ed = gui_edit({id = "upEd"})
	down1Ed = gui_edit({id = "downEd"})
	left1Ed = gui_edit({id = "leftEd"})
	right1Ed = gui_edit({id = "rightEd"})
	fire1Ed = gui_edit({id = "fireEd"})
	change1Ed = gui_edit({id = "changeEd"})
	jump1Ed = gui_edit({id = "jumpEd"})
	local upLb = gui_label({id = "up", label = "Look up: "})
	local downLb = gui_label({id = "down", label = "Look down: "})
	local leftLb = gui_label({id = "left", label = "Move left: "})
	local rightLb = gui_label({id = "right", label = "Move right: "})
	local fireLb = gui_label({id = "fire", label = "Fire: "})
	local changeLb = gui_label({id = "change", label = "Change: "})
	local jumpLb = gui_label({id = "jump", label = "Jump: "})
	local player0Move = gui_button({id = "player1", label = "Player 0"})
	local player1Move = gui_button({id = "player2", label = "Player 1"})
	local playerLbMove = gui_label({id = "playerid", label = "Player 0"})
	local unbind0 = gui_button({id = "unbind", label = "Unbind all keys"})
	local unbind1 = gui_button({id = "unbind", label = "Unbind all keys"})

	local p0shoot = gui_button({id = "player1s", label = "Player 0"})
	local p1shoot = gui_button({id = "player2s", label = "Player 1"})
	local pshootLb = gui_label({id = "pshoot", label = "Player 0"})
	local p0wpn1 = gui_edit({id = "wpn1"})
	local p0wpn2 = gui_edit({id = "wpn2"})
	local p0wpn3 = gui_edit({id = "wpn3"})
	local p0wpn4 = gui_edit({id = "wpn4"})
	local p0wpn5 = gui_edit({id = "wpn5"})
	local p1wpn1 = gui_edit({id = "wpn1"})
	local p1wpn2 = gui_edit({id = "wpn2"})
	local p1wpn3 = gui_edit({id = "wpn3"})
	local p1wpn4 = gui_edit({id = "wpn4"})
	local p1wpn5 = gui_edit({id = "wpn5"})
	local wpn1Lb = gui_label({id = "wpn1Lb", label = "Weapon 1:"})
	local wpn2Lb = gui_label({id = "wpn2Lb", label = "Weapon 2:"})
	local wpn3Lb = gui_label({id = "wpn3Lb", label = "Weapon 3:"})
	local wpn4Lb = gui_label({id = "wpn4Lb", label = "Weapon 4:"})
	local wpn5Lb = gui_label({id = "wpn5Lb", label = "Weapon 5:"})

	local showmenuEdLb = gui_label({id = "showmenuEdLb", label = "Menu:"})
	local showchatEdLb = gui_label({id = "showchatEdLb", label = "Chat:"})
	local showscoreEdLb = gui_label({id = "showscoreEdLb", label = "Scoreboard:"})
	showmenuEd = gui_edit({id = "showmenuEd"})
	showchatEd = gui_edit({id = "showchatEd"})
	showscoreEd = gui_edit({id = "showscoreEd"})

	local function make_key_control(w, name)
		local k = console_key_for_action(name)
		w:set_lock(true)
		w:set_text(key_name(k))
		function w:onKeyDown(newk)
			if self:is_active() then
				local oldaction = console_action_for_key(newk)
				if oldaction then
					self:set_text("Used!")-- .. oldaction)
				else
					local n = key_name(newk)
					console_bind(newk, name)
					console_bind(k, nil)
					self:set_text(n)
					self:deactivate()
					k = newk
				end
				return true
			end
		end
	end
	
	win:add( {moveBtn, backBtn, shootBtn, miscBtn, applyBtn, upEd, downEd, leftEd, rightEd, fireEd, changeEd, jumpEd, up1Ed, down1Ed, left1Ed, right1Ed, fire1Ed, change1Ed, jump1Ed, upLb, downLb, leftLb, rightLb, fireLb, changeLb, jumpLb, player0Move, player1Move, playerLbMove, unbind0, unbind1, p0shoot, p1shoot, pshootLb, p0wpn1, p0wpn2, p0wpn3, p0wpn4, p0wpn5, p1wpn1, p1wpn2, p1wpn3, p1wpn4, p1wpn5, wpn1Lb, wpn2Lb, wpn3Lb, wpn4Lb, wpn5Lb, showmenuEdLb, showchatEdLb, showscoreEdLb, showmenuEd, showchatEd, showscoreEd})
	menu:add( {win} )
	gui_root():add(menu)

	make_key_control(upEd, "+p0_up")
	make_key_control(downEd, "+p0_down")
	make_key_control(leftEd, "+p0_left")
	make_key_control(rightEd, "+p0_right")
	make_key_control(fireEd, "+p0_fire")
	make_key_control(changeEd, "+p0_change")
	make_key_control(jumpEd, "+p0_jump")

	make_key_control(up1Ed, "+p1_up")
	make_key_control(down1Ed, "+p1_down")
	make_key_control(left1Ed, "+p1_left")
	make_key_control(right1Ed, "+p1_right")
	make_key_control(fire1Ed, "+p1_fire")
	make_key_control(change1Ed, "+p1_change")
	make_key_control(jump1Ed, "+p1_jump")

	make_key_control(p0wpn1, "+p0_weapon1")
	make_key_control(p0wpn2, "+p0_weapon2")
	make_key_control(p0wpn3, "+p0_weapon3")
	make_key_control(p0wpn4, "+p0_weapon4")
	make_key_control(p0wpn5, "+p0_weapon5")

	make_key_control(p1wpn1, "+p1_weapon1")
	make_key_control(p1wpn2, "+p1_weapon2")
	make_key_control(p1wpn3, "+p1_weapon3")
	make_key_control(p1wpn4, "+p1_weapon4")
	make_key_control(p1wpn5, "+p1_weapon5")

	make_key_control(showmenuEd, "showmenuEd")
	make_key_control(showchatEd, "showchatEd")
	make_key_control(showscoreEd, "+scores")

	function controlsm.isShown()
		return menu:is_visible()
	end

	function controlsm.show()
		if not menu:is_visible() then
			menu:set_visibility(true)
			moveTab(true)
			shootTab(false)
			miscTab(false)
			playerLbMove:set_text("Player 0")
			pshootLb:set_text("Player 0")
			menu:focus()
		end
	end

	function controlsm.hide()
		menu:set_visibility(false)
	end

	function moveTab(state)
		playerLbMove:set_text("Player 0")
		upEd:set_visibility(state)
		downEd:set_visibility(state)
		leftEd:set_visibility(state)
		rightEd:set_visibility(state)
		fireEd:set_visibility(state)
		changeEd:set_visibility(state)
		jumpEd:set_visibility(state)
		up1Ed:set_visibility(false)
		down1Ed:set_visibility(false)
		left1Ed:set_visibility(false)
		right1Ed:set_visibility(false)
		fire1Ed:set_visibility(false)
		change1Ed:set_visibility(false)
		jump1Ed:set_visibility(false)
		unbind1:set_visibility(false)
		unbind0:set_visibility(state)
		upLb:set_visibility(state)
		downLb:set_visibility(state)
		leftLb:set_visibility(state)
		rightLb:set_visibility(state)
		fireLb:set_visibility(state)
		changeLb:set_visibility(state)
		jumpLb:set_visibility(state)
		player0Move:set_visibility(state)
		player1Move:set_visibility(state)
		playerLbMove:set_visibility(state)
	end

	function shootTab(state)
			pshootLb:set_text("Player 0")
			p0shoot:set_visibility(state)
			p1shoot:set_visibility(state)
			pshootLb:set_visibility(state)
			p0wpn1:set_visibility(state)
			p0wpn2:set_visibility(state)
			p0wpn3:set_visibility(state)
			p0wpn4:set_visibility(state)
			p0wpn5:set_visibility(state)
			p1wpn1:set_visibility(false)
			p1wpn2:set_visibility(false)
			p1wpn3:set_visibility(false)
			p1wpn4:set_visibility(false)
			p1wpn5:set_visibility(false)
			wpn1Lb:set_visibility(state)
			wpn2Lb:set_visibility(state)
			wpn3Lb:set_visibility(state)
			wpn4Lb:set_visibility(state)
			wpn5Lb:set_visibility(state)
	end

	function miscTab(state)
		showmenuEdLb:set_visibility(state)
		showchatEdLb:set_visibility(state)
		showscoreEdLb:set_visibility(state)
		showmenuEd:set_visibility(state)
		showchatEd:set_visibility(state)
		showscoreEd:set_visibility(state)
	end

	function moveBtn:onAction()		moveTab(true)
		shootTab(false)
		miscTab(false)
	end

	function shootBtn:onAction()
		moveTab(false)
		shootTab(true)
		miscTab(false)
	end

	function miscBtn:onAction()
		moveTab(false)
		shootTab(false)
		miscTab(true)
	end

	function player0Move:onAction()
		if playerLbMove:text() == "Player 1" then
			playerLbMove:set_text("Player 0")
			upEd:set_visibility(true)
			downEd:set_visibility(true)
			leftEd:set_visibility(true)
			rightEd:set_visibility(true)
			fireEd:set_visibility(true)
			changeEd:set_visibility(true)
			jumpEd:set_visibility(true)
			unbind0:set_visibility(true)
			unbind1:set_visibility(false)
			up1Ed:set_visibility(false)
			down1Ed:set_visibility(false)
			left1Ed:set_visibility(false)
			right1Ed:set_visibility(false)
			fire1Ed:set_visibility(false)
			change1Ed:set_visibility(false)
			jump1Ed:set_visibility(false)
		end
	end
		
	function player1Move:onAction()
		if playerLbMove:text() == "Player 0" then
			playerLbMove:set_text("Player 1")
			up1Ed:set_visibility(true)
			down1Ed:set_visibility(true)
			left1Ed:set_visibility(true)
			right1Ed:set_visibility(true)
			fire1Ed:set_visibility(true)
			change1Ed:set_visibility(true)
			jump1Ed:set_visibility(true)
			unbind1:set_visibility(true)
			unbind0:set_visibility(false)
			upEd:set_visibility(false)
			downEd:set_visibility(false)
			leftEd:set_visibility(false)
			rightEd:set_visibility(false)
			fireEd:set_visibility(false)
			changeEd:set_visibility(false)
			jumpEd:set_visibility(false)
		end
	end

	function p0shoot:onAction()
		if pshootLb:text() == "Player 1" then
			pshootLb:set_text("Player 0")
			p1wpn1:set_visibility(false)
			p1wpn2:set_visibility(false)
			p1wpn3:set_visibility(false)
			p1wpn4:set_visibility(false)
			p1wpn5:set_visibility(false)
			p0wpn1:set_visibility(true)
			p0wpn2:set_visibility(true)
			p0wpn3:set_visibility(true)
			p0wpn4:set_visibility(true)
			p0wpn5:set_visibility(true)
		end
	end

	function p1shoot:onAction()
		if pshootLb:text() == "Player 0" then
			pshootLb:set_text("Player 1")
			p0wpn1:set_visibility(false)
			p0wpn2:set_visibility(false)
			p0wpn3:set_visibility(false)
			p0wpn4:set_visibility(false)
			p0wpn5:set_visibility(false)
			p1wpn1:set_visibility(true)
			p1wpn2:set_visibility(true)
			p1wpn3:set_visibility(true)
			p1wpn4:set_visibility(true)
			p1wpn5:set_visibility(true)
		end
	end	

	function unbind0:onAction()
	
	end

	function unbind1:onAction()
	
	end

	function applyBtn:onAction()
		
	end

	function backBtn:onAction()
		controlsm.hide()
		optionsm.show()
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC then
			controlsm.hide()
			optionsm.show()
			return true
		end
	end

	controlsm.hide()

end
