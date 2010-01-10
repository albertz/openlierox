function connectm.init()
	-- Connect Menu
	byipm.init()
	gui_load_gss("menu-common")
	gui_load_gss("connect-menu")

	local menu = gui_group({id = "connect-menu"})
	local win = gui_window({id = "connect-win"})
	
	local connectBtn = gui_button({id = "connect", label = "Connect"})
	local refreshBtn = gui_button({id = "refresh", label = "Refresh"})
	local byipBtn = gui_button({id = "byip", label = "By IP"})
	local backBtn = gui_button({id = "back", label = "Back"})
	local serverListw = gui_list({id = "serverlist"})
	local splitCheck = gui_check({id = "splitscreen", label = "Splitscreen"})

	win:add( {connectBtn, refreshBtn, byipBtn, backBtn, serverListw, splitCheck} )
	menu:add( {win} )
	gui_root():add(menu)

	serverListw:add_column("Name", 0.5)
	serverListw:add_column("Mod", 0.2)
	serverListw:add_column("Map", 0.3)
	win:set_sub_focus(serverListw)

	function connectm.isShown()
		return menu:is_visible() or byipm.isShown()
	end

	function connectm.refreshServers()
		common.serverList(serverListw)
	end

	function connectm.show()
		if not menu:is_visible() then
			menu:set_visibility(true)
			menu:focus()
		end
		print(console.CL_SPLITSCREEN)
		if console.CL_SPLITSCREEN ~= "0" then
			splitCheck:set_state( true )
		else
			splitCheck:set_state( false )
		end
		connectm.refreshServers()
	end

	function connectm.hide()
		menu:set_visibility(false)
		byipm.hide()
	end

	function connectBtn:onAction()
		local sel = serverListw:main_selection()
		if sel then
			local ip = sel:data().ip
			if ip then
				print(ip)
				connect(ip)
				connectm.hide()
				mainm.show()
			end
		end
	end
	
	function refreshBtn:onAction()
		connectm.refreshServers()
	end

	function byipBtn:onAction()
		connectm.hide()
		byipm.show()
	end

	function backBtn:onAction()
		connectm.hide()
		mainm.show()
	end

	function splitCheck:onAction()
		if splitCheck:state() then
			console.CL_SPLITSCREEN = 1
		else
			console.CL_SPLITSCREEN = 0
		end
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC and map_is_loaded() then
			connectm.hide()
			mainm.show()
			return true
		end
	end

	connectm.hide()

end