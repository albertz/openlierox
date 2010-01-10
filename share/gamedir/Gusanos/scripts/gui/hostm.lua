function hostm.init()
		-- Connect Menu
	gui_load_gss("menu-common")
	gui_load_gss("host-menu")

	local menu = gui_group({id = "host-menu"})
	local win = gui_window({id = "host-win"})
	
	local mapBtn = gui_button({id = "host-choosemap", label = "Select map"})
	local hostBtn = gui_button({id = "host", label = "Host"})
	local backBtn = gui_button({id = "back", label = "Back"})
	local splitCheck = gui_check({id = "splitscreen", label = "Splitscreen"})
	local registerCheck = gui_check({id = "register", label = "Register online"})
	local mapLabel = gui_label({id = "host-maptext", label = "None"})

	win:add( {mapBtn, backBtn, hostBtn, splitCheck, registerCheck, mapLabel} )
	menu:add( {win} )
	gui_root():add(menu)

	function hostm.isShown()
		return menu:is_visible()
	end

	function hostm.show()
		if not menu:is_visible() then
			menu:set_visibility(true)
			menu:focus()
		end

		if console.CL_SPLITSCREEN ~= "0" then
			splitCheck:set_state( true )
		else
			splitCheck:set_state( false )
		end
		if console.NET_REGISTER ~= "0" then
			registerCheck:set_state( true )
		else
			registerCheck:set_state( false )
		end

		if mapm.chosenMap then
			mapLabel:set_text(mapm.chosenMap)
		else
			mapLabel:set_text("None")
		end
	end

	function hostm.hide()
		menu:set_visibility(false)
	end

	function hostBtn:onAction()
		if mapm.chosenMap then
			host(mapm.chosenMap)
		end
	end

	function mapBtn:onAction()
		hostm.hide()
		mapm.show(hostm.show)
	end

	function backBtn:onAction()
		hostm.hide()
		mainm.show()
	end

	function splitCheck:onAction()
		if splitCheck:state() then
			console.CL_SPLITSCREEN = 1
		else
			console.CL_SPLITSCREEN = 0
		end
	end

	function registerCheck:onAction()
		if registerCheck:state() then
			console.NET_REGISTER = 1
		else
			console.NET_REGISTER = 0
		end
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC and map_is_loaded() then
			hostm.hide()
			mainm.show()
			return true
		end
	end

	hostm.hide()

end