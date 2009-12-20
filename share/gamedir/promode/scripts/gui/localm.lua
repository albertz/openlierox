function localm.init()
		-- Connect Menu
	gui_load_gss("menu-common")
	gui_load_gss("local-menu")

	local menu = gui_group({id = "local-menu"})
	local win = gui_window({id = "local-win"})
	
	local mapBtn = gui_button({id = "local-choosemap", label = "Select map"})
	local playBtn = gui_button({id = "play", label = "Play"})
	local backBtn = gui_button({id = "back", label = "Back"})
	local splitCheck = gui_check({id = "splitscreen", label = "Splitscreen"})
	local mapLabel = gui_label({id = "local-maptext", label = "None"})
	local mapList = gui_list({id = "map_list"})

	win:add( {backBtn, playBtn, splitCheck, mapList} )
	menu:add( {win} )
	gui_root():add(menu)
	
	mapList:add_column("Map name", 1)

	function localm.isShown()
		return menu:is_visible()
	end

	function localm.show()
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

		if console.CL_SPLITSCREEN ~= "0" then
			splitCheck:set_state( true )
		else
			splitCheck:set_state( false )
		end

		if mapm.chosenMap then
			mapLabel:set_text(mapm.chosenMap)
		else
			mapLabel:set_text("None")
		end
	end

	function localm.hide()
		menu:set_visibility(false)
	end

	function playBtn:onAction()
		--if mapm.chosenMap then
		--	map(mapm.chosenMap)
		--end
		if mapList:selection() then
			gameMap = mapList:selection()
			host(mapList:selection())
		end
	end

	function mapBtn:onAction()
		localm.hide()
		mapm.show(localm.show)
	end

	function backBtn:onAction()
		localm.hide()
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
			localm.hide()
			mainm.show()
			return true
		end
	end

	localm.hide()

end