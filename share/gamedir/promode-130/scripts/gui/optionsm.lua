function optionsm.init()
	gameoptionsm.init()
	controlsm.init()
	systemm.init()
	playerm.init()
	gui_load_gss("menu-common")
	gui_load_gss("options-menu")

	local menu = gui_group({id = "options-menu"})
	local win = gui_window({id = "options-win"})
	
	local playerBtn = gui_button({id = "player", label = "Player"})
	local controlsBtn = gui_button({id = "controls", label = "Controls"})
	local systemBtn = gui_button({id = "system", label = "System"})
	local gameoptionsBtn = gui_button({id = "gameoptions", label = "Game Options"})
	local saveBtn = gui_button({id = "save", label = "Save"})
	local backBtn = gui_button({id = "back", label = "Back"})

	win:add( {playerBtn, backBtn, controlsBtn, systemBtn, gameoptionsBtn, saveBtn})
	menu:add( {win} )
	gui_root():add(menu)

	function optionsm.isShown()
		return menu:is_visible()
	end

	function optionsm.show()
		if not menu:is_visible() then
			menu:set_visibility(true)
			menu:focus()
		end
	end

	function optionsm.hide()
		menu:set_visibility(false)
	end

	function controlsBtn:onAction()
		optionsm.hide()
		controlsm.show()
	end

	function playerBtn:onAction()
		optionsm.hide()
		playerm.show()
	end

	function systemBtn:onAction()
		optionsm.hide()
		systemm.show()
	end

	function gameoptionsBtn:onAction()
		optionsm.hide()
		gameoptionsm.show()
	end

	function backBtn:onAction()
		optionsm.hide()
		mainm.show()
	end
	
	function saveBtn:onAction()
		config.save()
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC then
			optionsm.hide()
			mainm.show()
			return true
		end
	end

	optionsm.hide()

end
