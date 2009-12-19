function svconfig.init()

	function explode(seperator, str) 
		local pos, arr = 0, {}
		
		for st, sp in function() return string.find(str, seperator, pos, true ) end do
			table.insert(arr, string.sub(str, pos, st-1))
			pos = sp + 1
		end
		
		table.insert(arr, string.sub(str, pos))
		return arr
	end

	svMaps = {"dark_2xkrossb","dark_arkham","dark_cathode","dark_duel2","dark_pokolenia","dark_deathruw","dark_izbla","dark_wtf","dark_memory","dark_fallout2","dark_owl3"}

	svConfig = {
		name = "Play more promode!",
		desc = "Gusanos Promode",
		port = 27960,
		rconpass = "password",
		map = "pokolenia",
		mode = 1,
		timelimit = 15,
		killslimit = 0,
		deathlimit = 0,
		maxclients = 4,
		showmessage = 0,
		messagedelay = 5,
		message = "http://promode.pordesign.eu/",
		randommaps = 1,
		reloadmap = 0,
		reloadtime = 1,
		ircbot = 0,
		botnick = "GIRCBOT",
		botchannel = "#gusanos",
		botserver = "se.quakenet.org",
		botadmin1 = "wesz",
		botadmin2 = "",
		botadmin3 = "",
		healthpacks = 0,
		healthpacksDelay = 60,
		password = "",
		pmc = 0,
		anticamp = 0
	}
	
	console_register_command("SV_DED_NAME",function(var)
		svConfig.name = var
	end)
	
	console_register_command("SV_DED_DESC",function(var)
		svConfig.desc = var
	end)
	
	console_register_command("SV_DED_PORT",function(var)
		svConfig.port = tonumber(var)
	end)
	
	console_register_command("SV_DED_RCON_PASSWORD",function(var)
		svConfig.rconpass = var
	end)
	
	console_register_command("SV_DED_MAP",function(var)
		svConfig.map = var
	end)
	
	console_register_command("SV_DED_MODE",function(var)
		svConfig.mode = modes[var]
	end)
	
	console_register_command("SV_DED_TIMELIMIT",function(var)
		svConfig.timelimit = tonumber(var)
	end)
	
	console_register_command("SV_DED_KILLSLIMIT",function(var)
		svConfig.killslimit = tonumber(var)
	end)
	
	console_register_command("SV_DED_DEATHSLIMIT",function(var)
		svConfig.deathlimit = tonumber(var)
	end)
	
	console_register_command("SV_DED_MAXCLIENTS",function(var)
		svConfig.maxclients = tonumber(var)
	end)
	
	console_register_command("SV_DED_HEALTHPACKS",function(var)
		svConfig.healthpacks = tonumber(var)
	end)
	
	console_register_command("SV_DED_HEALTHPACKS_DELAY",function(var)
		svConfig.healthpacksDelay = tonumber(var)
	end)
	
	console_register_command("SV_DED_SHOWMESSAGE",function(var)
		svConfig.showmessage = tonumber(var)
	end)
	
	console_register_command("SV_DED_MESSAGEDELAY",function(var)
		svConfig.messagedelay = tonumber(var)
	end)
	
	console_register_command("SV_DED_MESSAGE",function(var)
		svConfig.message = var
	end)
		
	console_register_command("SV_DED_MAP_RANDOM",function(var)
		svConfig.randommaps = tonumber(var)
	end)
	
	console_register_command("SV_DED_MAP_RELOAD",function(var)
		svConfig.reloadmap = tonumber(var)
	end)
	
	console_register_command("SV_DED_MAP_RELOADTIME",function(var)
		svConfig.reloadtime = tonumber(var)
	end)
	
	console_register_command("SV_DED_IRCBOT",function(var)
		svConfig.ircbot = tonumber(var)
	end)
	
	console_register_command("SV_DED_IRCBOT_NAME",function(var)
		svConfig.botnick = var
	end)
	
	console_register_command("SV_DED_IRCBOT_CHANNEL",function(var)
		svConfig.botchannel = var
	end)
	
	console_register_command("SV_DED_IRCBOT_SERVER",function(var)
		svConfig.botserver = var
	end)
	
	console_register_command("SV_DED_IRCBOT_ADMIN1",function(var)
		svConfig.botadmin1 = var
	end)
	
	console_register_command("SV_DED_IRCBOT_ADMIN2",function(var)
		svConfig.botadmin2 = var
	end)
	
	console_register_command("SV_DED_IRCBOT_ADMIN3",function(var)
		svConfig.botadmin3 = var
	end)
	
	console_register_command("SV_DED_MAP_LIST",function(var)
		svMaps = explode(",", var)
	end)
	
	console_register_command("SV_DED_PASSWORD", function(var)
		svConfig.password = var
	end)
	
	console_register_command("SV_DED_ANTICAMP", function(var)
		svConfig.anticamp = tonumber(var)
	end)
	
	console_register_command("SV_DED_PMC", function(var)
		svConfig.pmc = tonumber(var)
	end)
	
	console.exec = "server.cfg"
end

