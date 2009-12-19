function config.init()
	bind = {}
	var = {}
	
	function config.updateVar()
		var = {}
		function add(name, default)
			--if (default ~= console[name]) then
				table.insert(var, "launcher config "..name.." \""..console[name].."\"")
			--end
		end
		
		function _add(name, default, value)
			--if (default ~= value) then
				if (value ~= nil) then
					table.insert(var, "launcher config "..name.." \""..value.."\"")
				else 
					table.insert(var, "launcher config "..name.." \""..default.."\"")
				end
			--end
		end
		
		add("p0_aim_accel", 0.169988)
		add("p0_aim_friction", 0)
		add("p0_aim_speed", 1.69998)
		add("p0_name", "GusPlayer")
		add("p0_rope_adjust_speed", 0.5)
		add("p0_viewport_follow", 0.1)
		add("p1_aim_accel", 0.169988)
		add("p1_aim_friction", 0)
		add("p1_aim_speed", 1.69998)
		add("p1_name", "GusPlayer")
		add("p1_rope_adjust_speed", 0.5)
		add("p1_viewport_follow", 0.1)
		
		add("cl_showdebug", 0)
		add("cl_showfps", 1)
		add("cl_show_map_debug", 0)
		add("con_font", "minifont")
		add("con_height", 120)
		add("con_speed", 4)
		add("net_autodownloads", 1)
		add("net_check_crc", 1)
		add("net_down_bpp", 200)
		add("net_down_pps", 20)
		add("net_register", 1)
		add("net_server_desc", "")
		add("net_server_name", "")
		add("net_server_port", 9898)
		add("net_sim_lag", 0)
		add("net_sim_loss", -1)
		add("net_up_limit", 10000)
		
		add("sfx_output_mode", "AUTO")
		add("sfx_volume", 256)
		add("vid_bitdepth", 32)
		add("vid_clear_buffer", 0)
		add("vid_doubleres", 0)
		add("vid_driver", "AUTO")
		add("vid_filter", "NOFILTER")
		add("vid_fullscreen", 0)
		add("vid_vsync", 0)
		
		_add("cg_enemybox", 0, cg_enemybox)
		_add("cg_enemycolor", "0\" \"255\" \"0", cg_enemycolor)
		_add("cg_teambox", 0, cg_teambox)
		_add("cg_teamcolor", "0\" \"0\" \"255", cg_teamcolor)
		_add("ch_bloodscreen", 1, ch_bloodscreen)
		_add("ch_crosshaircolor", "0\" \"255\" \"0", ch_crosshaircolor)
		_add("ch_crosshairdist", 25, ch_crosshairdist)
		_add("ch_crosshairtype", 5, ch_crosshairtype)
		_add("ch_playerhealth", 1, ch_playerhealth)
		_add("ch_playerhealth_own", 0, ch_playerhealth_own)
		_add("ch_playernames", 1, ch_playernames)
		_add("ch_spectator", 1, ch_spectator)
		_add("ch_radar", 0, ch_radar)
		_add("ch_radartype", 0, ch_radartype)
		_add("ch_reloadtimer", 0, ch_reloadtimer)
		_add("ch_showtime", 1, ch_showtime)
		_add("ch_statusbar", 2, ch_statusbar)
		_add("cl_weapons", "1 1 1 1 1", cl_weapons)
		_add("sv_deathslimit", 20, sv_deathslimit)
		_add("sv_killslimit", 20, sv_killslimit)
		_add("sv_maxclients", 4, sv_maxclients)
		_add("sv_timelimit", 15, sv_timelimit)
		_add("ch_messages_x", 315, ch_messages_x)
		_add("ch_messages_y", 2, ch_messages_y)
		_add("ch_messages_visible", 1, ch_messages_visible)
		_add("ch_messages_timer", 1000, ch_messages_timer)
		_add("ch_messages_align", 1, ch_messages_align)
		_add("cg_lasercolor", "255\" \"0\" \"0", cg_lasercolor)
		_add("ch_weaponsinfo", 1, ch_weaponsinfo)
		_add("cg_gibs", 6, cg_gibs)
		_add("cg_draw2d", 1, cg_draw2d)
		_add("cg_autoscreenshot", 1, cg_autoscreenshot)
		_add("sv_healthpacks", 0, sv_healthpacks)
		_add("sv_healthpacks_delay", 60, sv_healthpacks_delay)
		_add("sv_password", "", sv_password)
		_add("cg_logstats", 1, cg_logstats)
	end

	function config.updateBind()
		bind = {}
		function add(name, value)
			if (value ~= "NULL") then
				table.insert(bind, "launcher config bind "..value.." \""..name.."\"")
			end
		end
		
		add("+p0_up", upEd:text())
		add("+p0_down", downEd:text())
		add("+p0_left", leftEd:text())
		add("+p0_right", rightEd:text())
		add("+p0_fire", fireEd:text())
		add("+p0_change", changeEd:text())
		add("+p0_jump", jumpEd:text())

		add("+p1_up", up1Ed:text())
		add("+p1_down", down1Ed:text())
		add("+p1_left", left1Ed:text())
		add("+p1_right", right1Ed:text())
		add("+p1_fire", fire1Ed:text())
		add("+p1_change", change1Ed:text())
		add("+p1_jump", jump1Ed:text())

		add("showmenu", showmenuEd:text())
		add("showchat", showchatEd:text())
		add("showteammenu", "f4")
		add("+scores", showscoreEd:text())
		add("+stats", "lshift")
		add("p0_vote yes", "f1")
		add("p0_vote no", "f2")
	end
	
	function config.save()
		config.updateBind()
		config.updateVar()
		
		for i = 1, #bind do
			print(bind[i])
		end
		for i = 1, #var do
			print(var[i])
		end
		print("launcher savecfg;")
	end
end