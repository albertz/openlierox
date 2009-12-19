function gameoptionsm.init()

	gui_load_gss("menu-common")
	gui_load_gss("options-menu")

	local menu = gui_group({id = "options-menu"})
	local win = gui_window({id = "options-win"})

	local applyBtn = gui_button({id = "apply", label = "Apply"})
	local backBtn = gui_button({id = "back", label = "Back"})

	local fps = gui_check({id = "fps", label = "Show fps"})
	local debug = gui_check({id = "debug", label = "Show debug"})
	local mapdebug = gui_check({id = "mapdebug", label = "Show map debug"})
	local deathsmsg = gui_check({id = "deathmsg", label = "Show death messages"})
	local pnames = gui_check({id = "pnames", label = "Show player names"})
	local radar = gui_check({id = "radar", label = "Show radar"})
	local enemybox = gui_check({id = "enemybox", label = "Show enemy box"})
	local teambox = gui_check({id = "teambox", label = "Show team box"})

	win:add({backBtn, applyBtn, fps, debug, mapdebug, deathsmsg, pnames, radar, enemybox, teambox})
	menu:add( {win} )
	gui_root():add(menu)

	function gameoptionsm.isShown()
		return menu:is_visible()
	end

	function gameoptionsm.show()
		if not menu:is_visible() then
			menu:set_visibility(true)
			menu:focus()
			if console.cl_showfps == "1" then
				fps:set_state(true)
			else
				fps:set_state(false)
			end
			if console.cl_showdebug == "1" then
				debug:set_state(true)
			else
				debug:set_state(false)
			end
			if console.cl_show_map_debug == "1" then
				mapdebug:set_state(true)
			else
				mapdebug:set_state(false)
			end
			if console.cl_show_death_messages == "1" then
				deathsmsg:set_state(true)
			else
				deathsmsg:set_state(false)
			end
			if ch_playernames == 1 then
				pnames:set_state(true)
			else
				pnames:set_state(false)
			end
			if ch_radar == 1 then
				radar:set_state(true)
			else
				radar:set_state(false)
			end
			if cg_enemybox == 1 then
				enemybox:set_state(true)
			else
				enemybox:set_state(false)
			end
			if cg_teambox == 1 then
				teambox:set_state(true)
			else
				teambox:set_state(false)
			end
		end
	end

	function gameoptionsm.hide()
		menu:set_visibility(false)
	end

	function applyBtn:onAction()
		local options = {}
		options[0] = false
		options[1] = true
		options[true] = "1"
		options[false] = "0"

		if options[fps:state()] ~= console["cl_showfps"] then
			console.cl_showfps = options[fps:state()]
		end

		if options[debug:state()] ~= console["cl_showdebug"] then
			console.cl_showdebug = options[debug:state()]
		end

		if options[mapdebug:state()] ~= console["cl_show_map_debug"] then
			console.cl_show_map_debug = options[mapdebug:state()]
		end

		if options[deathsmsg:state()] ~= console["cl_show_death_messages"] then
			console.cl_show_death_messages = options[deathsmsg:state()]
		end

		if tonumber(options[pnames:state()]) ~= ch_playernames then
			console.pm_ch_playernames = tonumber(options[pnames:state()])
		end

		if tonumber(options[radar:state()]) ~= ch_radar then
			console.pm_ch_radar = tonumber(options[radar:state()])
		end

		if tonumber(options[enemybox:state()]) ~= cg_enemybox then
			console.pm_cg_enemybox = tonumber(options[enemybox:state()])
		end

		if tonumber(options[teambox:state()]) ~= cg_teambox then
			console.pm_cg_teambox = tonumber(options[teambox:state()])
		end
	end

	function backBtn:onAction()
		gameoptionsm.hide()
		optionsm.show()
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC then
			gameoptionsm.hide()
			optionsm.show()
			return true
		end
	end

	gameoptionsm.hide()

end
