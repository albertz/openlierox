function systemm.init()
		-- Connect Menu
	gui_load_gss("menu-common")
	gui_load_gss("options-menu")

	local menu = gui_group({id = "options-menu"})
	local win = gui_window({id = "options-win"})

	-- GRAPHICS 
	function videoMode()
		local def = console["vid_doubleres"]
		if def == "0" then
			return "Video mode: 320 x 240"
		elseif def == "1" then
			return "Video mode: 640 x 480"
		end
	end

	function fullscreen()
		local def = console["vid_fullscreen"]
		if def == "0" then
			return "Fullscreen: Off"
		elseif def == "1" then
			return "Fullscreen: On"
		end
	end

	function bitDepth()
		local def = console["vid_bitdepth"]
		if def == "16" then
			return "Bit depth: 16"
		elseif def == "32" then
			return "Bit depth: 32"
		end
	end

	function driver()
		local def = console["vid_driver"]
		return "Driver: "..def
	end

	function filter()
		local def = console["vid_filter"]
		return "Filter: "..def
	end

	function clearBuffer()
		local def = console["vid_clear_buffer"]
		if def == "0" then
			return "Clear buffer: Off"
		elseif def == "1" then
			return "Clear buffer: On"
		end
	end

	function sync()
		local def = console["vid_vsync"]
		if def == "0" then
			return "Vertical sync: Off"
		elseif def == "1" then
			return "Vertical sync: On"
		end
	end

	-- SOUND
	function volume()
		local def = tonumber(console["sfx_volume"])
		local vol = {0, 25, 50, 75, 100, 125, 150, 175, 200, 225, 255}
		local eq = {}
		for i = 1,10 do
			table.insert(eq, "-")
			if def >= vol[i] and def <= vol[i+1]+1 then
				eq[i] = "|"
			end
		end
		return "Volume: "..table.concat(eq)
	end

	function outputMode()
		local def = console["sfx_output_mode"]
		return "Driver: "..def
	end

	function listenerDistance()
		local def = console["sfx_listener_distance"]
		return def
	end

	function bpp()
		local def = console["net_down_bpp"]
		return def
	end
		
	function pps()
		local def = console["net_down_pps"]
		return def
	end

	function netLog()
		local def = console["net_log"]
		if def == "0" then
			return "Network log: None"
		elseif def == "1" then
			return "Network log: Console"
		end
	end

	function simLag()
		local def = console["net_sim_lag"]
		return def
	end

	function upLimit()
		local def = console["net_up_limit"]
		return def
	end

	local graphicsBtn = gui_button({id = "system-graphics", label = "Graphics"})
	local soundBtn = gui_button({id = "system-sound", label = "Sound"})
	local networkBtn = gui_button({id = "system-network", label = "Network"})
	local applyBtn = gui_button({id = "apply", label = "Apply"})
	local backBtn = gui_button({id = "back", label = "Back"})

	local fullscreenBtn = gui_button({id = "fullscreen", label = fullscreen()})
	local videoModeBtn = gui_button({id = "videoMode", label = videoMode()})
	local driverBtn = gui_button({id = "driver", label = driver()})
	local bitDepthBtn = gui_button({id = "bitDepth", label = bitDepth()})
	local filterBtn = gui_button({id = "filter", label = filter()})
	local clearBufferBtn = gui_button({id = "clearBuffer", label = clearBuffer()})
	local syncBtn = gui_button({id = "sync", label = sync()})

	local volumeBtn = gui_button({id = "volume", label = volume()})
	local outputModeBtn = gui_button({id = "outputMode", label = outputMode()})
	local listenerDistanceEd = gui_edit({id = "listenerDistance", label = listenerDistance()})
	local listenerDistanceLb = gui_label({id = "listenerDistanceLb", label = "Listener distance: "})

	local autoDownCh = gui_check({id = "autodownloads", label = "Autodownloads"})
	local checkCRCch = gui_check({id = "checkCRC", label = "Check CRC"})
	local BPPEd = gui_edit({id = "BPP", label = bpp()})
	local BPPLb = gui_label({id = "BPPLb", label = "Bytes per packet: "})
	local PPSEd = gui_edit({id = "PPS", label = pps()})
	local PPSLb = gui_label({id = "PPSLb", label = "Packets per second: "})
	local netLogBtn = gui_button({id = "netLog", label = netLog()})
	local simLagEd = gui_edit({id = "simLag", label = simLag()})
	local simLagLb = gui_label({id = "simLagLb", label = "Simulate lag: "})
	local upLimitEd = gui_edit({id = "upLimit", label = upLimit()})
	local upLimitLb = gui_label({id = "upLimitLb", label = "Upstream limit: "})

	win:add( {graphicsBtn, backBtn, soundBtn, networkBtn, videoModeBtn, fullscreenBtn, applyBtn, bitDepthBtn, driverBtn, filterBtn, clearBufferBtn, syncBtn, volumeBtn, outputModeBtn, listenerDistanceEd, listenerDistanceLb, autoDownCh, checkCRCch, BPPEd, BPPLb, PPSEd, PPSLb, netLogBtn, simLagEd, simLagLb, upLimitEd, upLimitLb})
	menu:add( {win} )
	gui_root():add(menu)

	function systemm.isShown()
		return menu:is_visible()
	end

	function systemm.show()
		if not menu:is_visible() then
			menu:set_visibility(true)
			graphicsTab(true)
			soundTab(false)
			networkTab(false)
			menu:focus()
			if console.net_autodownloads == "1" then
				autoDownCh:set_state(true)
			else
				autoDownCh:set_state(false)
			end
			if console.net_check_crc == "1" then
				checkCRCch:set_state(true)
			else
				checkCRCch:set_state(false)
			end
		end
	end

	function systemm.hide()
		menu:set_visibility(false)
	end

	function graphicsTab(state)
		fullscreenBtn:set_visibility(state)
		videoModeBtn:set_visibility(state)
		driverBtn:set_visibility(state)
		bitDepthBtn:set_visibility(state)
		filterBtn:set_visibility(state)
		clearBufferBtn:set_visibility(state)
		syncBtn:set_visibility(state)
	end

	function soundTab(state)
		volumeBtn:set_visibility(state)
		outputModeBtn:set_visibility(state)
		listenerDistanceEd:set_visibility(state)
		listenerDistanceLb:set_visibility(state)
	end

	function networkTab(state)
		autoDownCh:set_visibility(state)
		checkCRCch:set_visibility(state)
		BPPEd:set_visibility(state)
		BPPLb:set_visibility(state)
		PPSEd:set_visibility(state)
		PPSLb:set_visibility(state)
		netLogBtn:set_visibility(state)
		simLagEd:set_visibility(state)
		simLagLb:set_visibility(state)
		upLimitEd:set_visibility(state)
		upLimitLb:set_visibility(state)
	end

	function graphicsBtn:onAction()		graphicsTab(true)
		soundTab(false)
		networkTab(false)
	end

	function soundBtn:onAction()
		graphicsTab(false)
		soundTab(true)
		networkTab(false)
	end

	function networkBtn:onAction()
		graphicsTab(false)
		soundTab(false)
		networkTab(true)
	end

	function fullscreenBtn:onAction()
		if fullscreenBtn:text() == "Fullscreen: On" then
			fullscreenBtn:set_text("Fullscreen: Off")
		elseif fullscreenBtn:text() == "Fullscreen: Off" then
			fullscreenBtn:set_text("Fullscreen: On")
		end
	end

	function videoModeBtn:onAction()
		if videoModeBtn:text() == "Video mode: 320 x 240" then
			videoModeBtn:set_text("Video mode: 640 x 480")
		elseif videoModeBtn:text() == "Video mode: 640 x 480" then
			videoModeBtn:set_text("Video mode: 320 x 240")
		end
	end
	
	function bitDepthBtn:onAction()
		if bitDepthBtn:text() == "Bit depth: 16" then
			bitDepthBtn:set_text("Bit depth: 32")
		elseif bitDepthBtn:text() == "Bit depth: 32" then
			bitDepthBtn:set_text("Bit depth: 16")
		end
	end

	function driverBtn:onAction()
		local drv = driverBtn:text()
		local drvList = {"AUTO", "DIRECTX", "XDGA", "XDGA2", "XWINDOWS"}

		for i = 1, 5 do
			if drv == "Driver: "..drvList[i] then
				if i == 5 then
					driverBtn:set_text("Driver: "..drvList[1])
				else
					driverBtn:set_text("Driver: "..drvList[i+1])
				end
			end
		end
	end

	function filterBtn:onAction()
		local fil = filterBtn:text()
		local filList = {"BILINEAR", "NOFILTER", "NOFILTER2", "SCANLINES", "SCANLINES2", "SUPER2XSAI", "SUPEREAGLE"}

		for i = 1, 7 do
			if fil == "Filter: "..filList[i] then
				if i == 7 then
					filterBtn:set_text("Filter: "..filList[1])
				else
					filterBtn:set_text("Filter: "..filList[i+1])
				end
			end
		end
	end

	function clearBufferBtn:onAction()
		if clearBufferBtn:text() == "Clear buffer: On" then
			clearBufferBtn:set_text("Clear buffer: Off")
		elseif clearBufferBtn:text() == "Clear buffer: Off" then
			clearBufferBtn:set_text("Clear buffer: On")
		end
	end

	function syncBtn:onAction()
		if syncBtn:text() == "Vertical sync: On" then
			syncBtn:set_text("Vertical sync: Off")
		elseif syncBtn:text() == "Vertical sync: Off" then
			syncBtn:set_text("Vertical sync: On")
		end
	end

	function volumeBtn:onAction()
		local def = tonumber(console["sfx_volume"])
		def = def + 25
		if def > 254 then
			def = 0
		end
		local vol = {0, 25, 50, 75, 100, 125, 150, 175, 200, 225, 250}
		local eq = {}
		for i = 1,10 do
			table.insert(eq, "-")
			if def >= vol[i+1] and def <= vol[i+1] then
			
				eq[i] = "|"
			else
				eq[i] = "-"
			end
						 
		end
		volumeBtn:set_text("Volume: "..table.concat(eq))
		console.sfx_volume = def
	end

	function outputModeBtn:onAction()
		local drv = outputModeBtn:text()
		local drvList = {"AUTO", "NOSFX", "WINMM", "DSOUND", "A3D", "OSS", "ESD", "ALSA"}

		for i = 1, 8 do
			if drv == "Driver: "..drvList[i] then
				if i == 8 then
					outputModeBtn:set_text("Driver: "..drvList[1])
				else
					outputModeBtn:set_text("Driver: "..drvList[i+1])
				end
			end
		end
	end

	function netLogBtn:onAction()
		if netLogBtn:text() == "Network log: Console" then
			netLogBtn:set_text("Network log: None")
		elseif netLogBtn:text() == "Network log: None" then
			netLogBtn:set_text("Network log: Console")
		end
	end

	function applyBtn:onAction()
		local vars = {fullscreen(), videoMode(), driver(), bitDepth(), filter(), clearBuffer(), sync(), outputMode(), listenerDistance(), console["net_autodownloads"], console["net_check_crc"], bpp(), pps(), netLog(), simLag(), upLimit()}
		local sets = {fullscreenBtn:text(), videoModeBtn:text(), driverBtn:text(), bitDepthBtn:text(), filterBtn:text(), clearBufferBtn:text(), syncBtn:text(), outputModeBtn:text(), listenerDistanceEd:text(), autoDownCh:state(), checkCRCch:state(), BPPEd:text(), PPSEd:text(), netLogBtn:text(), simLagEd:text(), upLimitEd:text() }		
		local text = {"Fullscreen: ", "Video mode: ", "Driver: ", "Bit depth: ", "Filter: ", "Clear buffer: ", "Vertical sync: ", "Driver: ", "Listener distance: ", "Autodownloads", "Check CRC", "Bits per packet", "Packets per second", "Network log: ", "Simulate lag", "Upstream limit"}		
		local option = {}
			option["On"] = 1
			option["Off"] = 0
			option["320 x 240"] = 0
			option["640 x 480"] = 1
			option["None"] = 0
			option["Console"] = 1
			option[true] = 1
			option[false] = 0

		if vars[1] ~= sets[1] then
			console.vid_fullscreen = option[string.gsub(sets[1], text[1], "")]
		end

		if vars[2] ~= sets[2] then
			console.vid_doubleres = option[string.gsub(sets[2], text[2], "")]
		end

		if vars[3] ~= sets[3] then
				console.vid_driver = string.gsub(sets[3], text[3], "")
		end

		if vars[4] ~= sets[4] then
			console.vid_bitdepth = string.gsub(sets[4], text[4], "")
		end
		
		if vars[5] ~= sets[5] then
			console.vid_filter = string.gsub(sets[5], text[5], "")
		end
		
		if vars[6] ~= sets[6] then
			console.vid_clear_buffer = option[string.gsub(sets[6], text[6], "")]
		end

		if vars[7] ~= sets[7] then
			console.vid_vsync = option[string.gsub(sets[7], text[7], "")]
		end

		if vars[8] ~= sets[8] then
			console.sfx_output_mode = string.gsub(sets[8], text[8], "")
		end
		
		if vars[9] ~= sets[9] then
				console.sfx_listener_distance = sets[9]
		end

		if vars[10] ~= option[sets[10]] then 
			console.net_autodownloads = option[sets[10]]
		end

		if vars[11] ~= option[sets[11]] then
			console.net_check_crc = option[sets[11]]
		end

		if vars[12] ~= sets[12] then
			console.net_down_bpp = sets[12]
		end
		
		if vars[13] ~= sets[13] then
			console.net_down_pps = sets[13]
		end
		
		if vars[14] ~= sets[14] then
			console.net_log = option[string.gsub(sets[14], text[14], "")]
		end

		if vars[15] ~= sets[15] then
			console.net_sim_lag = sets[15]
		end
		
		if vars[16] ~= sets[16] then
			console.net_up_limit = sets[16]
		end
	end

	function backBtn:onAction()
		systemm.hide()
		optionsm.show()
	end

	function menu:onKeyDown(k)
		if k == Keys.ESC then
			systemm.hide()
			optionsm.show()
			return true
		end
	end

	systemm.hide()

end
