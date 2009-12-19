function teamm.init()
	gui_load_gss("team-menu")
		
	local menu = gui_group({id = "team-menu", group = "team"})
	local win = gui_window({id = "team-win"})
	
	local red = gui_button({})
	local blue = gui_button({})
	local join = gui_button({})
	local spect = gui_button({})
	
	--if gameMode == 3 or gameMode == 4 then
		local join = gui_button({id = "join2T", label = "Join"})
		local red = gui_button({id = "red1T", label="Red"})
		local blue = gui_button({id = "blue1T", label="Blue"})
		local spect = gui_button({id = "spect1T", label = "Spectator"})
		
		win:add(join, red, blue, spect)

		function red:onAction()
			if gameMode == 3 or gameMode == 4 or gameMode == 7 or gameMode == 8 then
				--console.p0_team = 1
				local hax = console["joinred"]
				console.p1_team = 1
				teamm.hide()
			end
		end
	
		function blue:onAction()
			if isTeamPlay() then
				console.p0_team = 2
				local hax = console["joinblue"]
				teamm.hide()
			end
		end
		
		function spect:onAction()
			console.p0_team = 0
			console.p1_team = 0
			teamm.hide()
		end
	--else
		--[[local join = gui_button({id = "join2T", label = "Join"})
		local spect = gui_button({id = "spect2T", label = "Spectator"})
	
		win:add(join, spect)]]
			
		function join:onAction()
			if not isTeamPlay() then
				if gameMode == 2 and inGame() >= 2 then
					draw.message(3, fonts.liero, "You can't join to the 1v1 match at the progress!", 160, 60, 200, 255, 255, 255)
				else
					console.p0_team = 1
					console.p1_team = 1
				end
				teamm.hide()
			end
		end
	
		--[[function spect:onAction()
			console.p0_team = 0
			console.p1_team = 0
			teamm.hide()
		end--]]
	--end
	
	menu:add(win)
	gui_root():add(menu)
		
	function teamm.isShown()
		return menu:is_visible()
	end

	function teamm.show()
		if not menu:is_visible() then
			menu:set_visibility(true)
			menu:focus()
		end
	end

	function teamm.hide()
		menu:set_visibility(false)
	end

	teamm.hide()

end
