function mainm.init()
	connectm.init()
	--hostm.init()
	localm.init()
	optionsm.init()
	mapm.init()
	modsm.init()
	
	gui_load_gss("menu-common")
	gui_load_gss("main-menu")
		
	local menu = gui_group({id = "menu", group = "main"})
	local title = gui_button({id = "titlehax", label = "Gusanos Menu"})
	local continueBtn = gui_button({id = "continue", label = "X"})
	local win = gui_window({id = "win"})
	
	local localBtn = gui_button({id = "split", label = "Practice"})
	local connectBtn = gui_button({id = "multi", label = "Multiplayer"})
	local optionsBtn = gui_button({id = "options", label = "Options"})
	local modsBtn = gui_button({id = "mods", label = "Mods"})
	local quitBtn = gui_button({id = "exit", label = "Quit"})
	
	win:add({connectBtn, quitBtn, localBtn, optionsBtn, modsBtn})
	menu:add({win})
	gui_root():add(menu)
	local status = gui_label({id = "status"})
	--status:set_visibility(true)
	--gui_root():add(status)
	msgm.init()
	local function showStatus(txt)
		status:set_visibility(true)
		status:set_text(txt)
	end
	
	local function hideStatus()
		status:set_visibility(false)
	end
	
	function bindings.transferUpdate(file, bps, done, size)
		msgm.show("Recieving " .. file .. ", " .. floor(100 * done / size) .. "%, " .. round(bps/1000, 2) .. " kB/s")
	end
	
	function bindings.transferFinished()
		hideStatus()
	end
	
	function bindings.gameError(err)
		if err == Error.MapLoading then
			msgm.show("Could not load map")
		elseif err == Error.ModLoading then
			msgm.show("Could not load mod")
		elseif err == Error.MapNotFound then
			msgm.show("Could not find the map")
		elseif err == Error.ModNotFound then
			msgm.show("Could not find the mod")
		end
	end
	
	function bindings.networkStateChange(state)
		if state == Network.Connecting then
			msgm.show("Connecting...")
		elseif state == Network.Disconnecting then
			if wrongPassword then
				msgm.show("Incorrect password")
				wrongPassword = false
			else
				msgm.show("Disconnecting...")
			end
		elseif state == Network.Disconnected then
			msgm.show("Disconnected")
		end
	end
	
	function bindings.gameEnded(reason)
		if reason == EndReason.ServerQuit then
			msgm.show("Server disconnected")
		elseif reason == EndReason.ServerChangeMap then
			msgm.show("Server is changing map...")
		elseif reason == EndReason.Kicked then
			if tooManyPlayers then
				msgm.show("Too many players...")
				tooManyPlayers = false
			else
				msgm.show("You got kicked from the server")
			end
		elseif reason == EndReason.IncompatibleProtocol then
			msgm.show("Your version of gusanos isn't compatible with the server's")
		elseif reason == EndReason.IncompatibleData then
			msgm.show("Your promode is incompatible with the server's")
		end
	end
	
	--[[
	local menu = gui_load_xml("main-menu")
	local win = menu:child("main-win")
	local connectBtn = win:child("main-connect")
	local hostBtn = win:child("main-host")
	local quitBtn = win:child("main-quit")
	local continueBtn = menu:child("main-continue")

	
	local options = gui_group({id = "options"})
	gui_root():add(options)

	local function make_key_control(w, name)
		local k = console_key_for_action(name)
		w:set_lock(true)
		w:set_text(key_name(k))
		function w:onKeyDown(newk)
			if self:is_active() then
				local oldaction = console_action_for_key(newk)
				if oldaction then
					self:set_text("Used by " .. oldaction)
				else
					local n = key_name(newk)
					console_bind(newk, name)
					console_bind(k, nil) -- Unbind old
					self:set_text(n)
					self:deactivate()
					k = newk
				end
				return true
			end
		end
	end
	
	for _, v in ipairs({"+P0_FIRE", "+P0_CHANGE", "+P0_JUMP"}) do
		local t = gui_edit()
		options:add({t})
		make_key_control(t, v)
	end
]]

	function mainm.isShown()
		return connectm.isShown() or menu:is_visible()
	end

	function mainm.show()
		if not menu:is_visible() then
			menu:set_visibility(true)
			menu:focus()
			win:focus()
			connectBtn:focus()
		end
	end

	function mainm.hide()
		menu:set_visibility(false)
	end

	function continueBtn:onAction()
		if map_is_loaded() then
			mainm.hide()
		end
	end

	function localBtn:onAction()
		console.game = "promode"
		mainm.hide()
		localm.show()
	end

	function optionsBtn:onAction()
		mainm.hide()
		optionsm.show()
	end
	
	function modsBtn:onAction()
		mainm.hide()
		modsm.show()
	end

	function connectBtn:onAction()
		mainm.hide()
		connectm.show()
	end

	function quitBtn:onAction()
		quit()
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC and map_is_loaded() then
			if (not firstLoad) then
				mainm.hide()
			end
			return true
		end
	end

	if map_is_loaded() then
		mainm.hide()
	else
		mainm.show()
	end
	
	function bindings.gameEnded()
		mainm.show()
	end

end
