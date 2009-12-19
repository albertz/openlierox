function modsm.init()
	gui_load_gss("menu-common")
	gui_load_gss("mods-menu")

	local menu = gui_group({id = "local-menu"})
	local win = gui_window({id = "local-win"})
	
	local modsBtn = gui_button({id = "local-choosemods", label = "Select mods"})
	local changeBtn = gui_button({id = "changemod", label = "Change"})
	local backBtn = gui_button({id = "back", label = "Back"})
	local modsLabel = gui_label({id = "local-modstext", label = "None"})
	local modsList = gui_list({id = "mods_list"})

	win:add( {backBtn, changeBtn, modsList} )
	menu:add( {win} )
	gui_root():add(menu)
	
	modsList:add_column("Mod name", 1)

	function modsm.isShown()
		return menu:is_visible()
	end

	function modsm.show()
		if not menu:is_visible() then
			menu:set_visibility(true)
			menu:focus()
		end
		
		modsList:clear()
		for m in mods() do
			if (m ~= "default") then
				modsList:insert(m)
			end
		end
		callBack = newCallBack

		if modsm.chosenmods then
			modsLabel:set_text(modsm.chosenmods)
		else
			modsLabel:set_text("Promode")
		end
	end

	function modsm.hide()
		menu:set_visibility(false)
	end

	function changeBtn:onAction()
		if modsList:selection() then
			console.game = modsList:selection()
			modsm.hide()
			localm.show()
		end
	end

	function modsBtn:onAction()
		modsm.hide()
		modsm.show(modsm.show)
	end

	function backBtn:onAction()
		modsm.hide()
		mainm.show()
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC and map_is_loaded() then
			modsm.hide()
			mainm.show()
			return true
		end
	end

	modsm.hide()

end