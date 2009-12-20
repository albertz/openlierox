function msgm.init()

	gui_load_gss("menu-common")
	gui_load_gss("msg-menu")
	
	local menu = gui_group({id = "msg", group = "main"})
	local win = gui_window({id = "win"})
	
	local closeBtn = gui_button({id = "close", label = "Close"})
	local infoBtn = gui_button({id = "infobtn", label = ""})
	
	win:add({closeBtn, infoBtn})
	menu:add({win})
	gui_root():add(menu)
	
	menu:set_visibility(false)

	function msgm.isShown()
		return menu:is_visible()
	end

	function msgm.show(text)
		if not menu:is_visible() then
			menu:set_visibility(true)
			menu:focus()
			win:focus()
			infoBtn:set_text(text)
			--print(text)
			closeBtn:focus()
		end
	end

	function msgm.hide()
		menu:set_visibility(false)
	end


	function closeBtn:onAction()
		msgm.hide()
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC and map_is_loaded() then
			msgm.hide()
			return true
		end
	end




end
