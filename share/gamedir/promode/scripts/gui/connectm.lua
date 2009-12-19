function connectm.init()
	-- Connect Menu
	--byipm.init()
	hostm.init()
	gui_load_gss("menu-common")
	gui_load_gss("connect-menu")

	local menu = gui_group({id = "connect-menu"})
	local win = gui_window({id = "connect-win"})

	function defPort()
		return console["net_server_port"]
	end
	
	local connectBtn = gui_button({id = "connect", label = "Connect"})
	local refreshBtn = gui_button({id = "refresh", label = "Refresh"})
	local hostBtn = gui_button({id = "create", label = "Create"})
	local byipBtn = gui_button({id = "byip", label = "By IP"})
	local backBtn = gui_button({id = "back", label = "Back"})
	local serverListw = gui_list({id = "serverlist"})
	local splitCheck = gui_check({id = "splitscreen", label = "Splitscreen"})
	local serverip = gui_edit({id = "serverip", label = ""})
	local serverport = gui_edit({id = "serverport", label = defPort()})
	local serveripLb = gui_label({id = "serveripLb", label = "Specify server ip:"})
	local serverportLb = gui_label({id = "serverportLb", label = "Port:"})
	local connectSpec = gui_button({ id = "connectSpec", label = "Connect"})

	win:add( {serveripLb, serverip, serverportLb, serverport, connectSpec, hostBtn, connectBtn, refreshBtn, backBtn, serverListw, splitChec})
	menu:add( {win} )
	gui_root():add(menu)

	serverListw:add_column("Name", 0.7)
	--serverListw:add_column("Mod", 0.3)
	--serverListw:add_column("Mode", 0.4)
	serverListw:add_column("Map", 0.4)
	--serverListw:add_column("Limit", 0.2)
	serverListw:add_column("IP", 0.5)
	serverListw:add_column("Port", 0.3)
	win:set_sub_focus(serverListw)
	function explode(seperator, str) 
		local pos, arr = 0, {}
		
		for st, sp in function() return string.find(str, seperator, pos, true ) end do
			table.insert(arr, string.sub(str, pos, st-1))
			pos = sp + 1
		end
		
		table.insert(arr, string.sub(str, pos))
		return arr
	end
	function serverList(window)
		if window then
			window:clear()
			window:insert("Fetching server list...")
			
			fetch_server_list(function(list)
				window:clear()
				if list then
					for _, server in ipairs (list) do
						if server.mod == "promode" then
							local str = server.ip
							local fnd = string.find(str, ":")
							local pos = string.sub(fnd, string.len(fnd)/2)
							local ip = string.sub(str, 0, pos-1)
							local port = string.sub(str, pos+1)
							local svDesc = string.sub(server.desc, string.len(server.desc)-3)
							--print(server.desc)
							--if server.desc ~= "bar" then
							--	local svDesc = explode("|", server.desc)
							--	local svMode = svDesc[2]
							--	local plimit = svDesc[3]
							--else
								local svMode = "FFA"
								local plimit = "5"
							--end
							--local n = window:insert(server.title, svMode, server.map, plimit, ip, port)
							local n = window:insert(server.title, server.map, ip, port)
							n:data().ip = server.ip
						end
					end
				else
					window:insert("Couldn't fetch list from server")
				end
			end)
		end
	end

	function connectm.isShown()
		return menu:is_visible()
	end

	function connectm.refreshServers()
		serverList(serverListw)
	end

	function connectm.show()
		if not menu:is_visible() then
			menu:set_visibility(true)
			menu:focus()
		end
		if console.CL_SPLITSCREEN ~= "0" then
			splitCheck:set_state( true )
		else
			splitCheck:set_state( false )
		end
		connectm.refreshServers()
	end

	function connectm.hide()
		menu:set_visibility(false)
	end

	function connectSpec:onAction()
		if serverip:text() ~= "" then
			if serverport:text() ~= "" then
				connect(serverip:text()..":"..serverport:text())
			else
				connect(serverip:text()..":"..defPort())
			end

			connectm.hide()
			mainm.show()
		end
	end

	function connectBtn:onAction()
		local sel = serverListw:main_selection()
		if sel then
			local ip = sel:data().ip
			if ip ~= "" then
				connect(ip)
				connectm.hide()
				mainm.show()
			end
		end
	end
	
	function refreshBtn:onAction()
		connectm.refreshServers()
	end

	function hostBtn:onAction()
		connectm.hide()
		hostm.show()
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
		if k == Keys.ESC then
			connectm.hide()
			mainm.show()
			return true
		end
	end

	connectm.hide()

end
