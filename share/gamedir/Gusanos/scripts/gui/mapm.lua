function mapm.init()
	
	local callBack = nil

	gui_load_gss("menu-common")
	gui_load_gss("map-menu")

	local menu = gui_group({id = "map-menu"})
	local win = gui_window({id = "map-win"})
	
	local chooseBtn = gui_button({id = "map-choose", label = "Choose"})
	local backBtn = gui_button({id = "back", label = "Back"})
	local mapList = gui_list({id = "maplist"})

	win:add( {chooseBtn, backBtn, mapList} )
	menu:add( {win} )
	gui_root():add(menu)

	mapList:add_column("Map name", 1)
	win:set_sub_focus(mapList)

	function mapm.isShown()
		return menu:is_visible()
	end

	function mapm.show( newCallBack )
		if not menu:is_visible() then
			menu:set_visibility(true)
			menu:focus()
		end
		mapList:clear()
		for m in maps() do
			mapList:insert(m)
		end
		callBack = newCallBack
	end

	function mapm.hide()
		menu:set_visibility(false)
	end

	function chooseBtn:onAction()
		mapm.hide()
		mapm.chosenMap = mapList:selection()
		if callBack then
			callBack()
		end
	end

	function backBtn:onAction()
		mapm.hide()
		if callBack then
			callBack()
		end
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC and map_is_loaded() then
			mapm.hide()
			if callBack then
				callBack()
			end
			return true
		end
	end

	mapm.hide()

end