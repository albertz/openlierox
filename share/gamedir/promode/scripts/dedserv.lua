function dedserv.init()
	svconfig.init()
	
	local nextMapDelay = 0
	local nextMap = ""
	local messageTimer = 0

	function bindings.afterUpdate()
		if svConfig.showmessage == 1 then
			if messageTimer < (svConfig.messagedelay*6000) then
				messageTimer = messageTimer + 1
			else
				promode.svsay(svConfig.message)
				messageTimer = 0
			end
		end
		if gameEnd > 0 then
			if nextMapDelay == 1 then
				addMessage("Match finished")
			end
			if nextMapDelay == 100 then
				if svConfig.randommaps == 1 and svConfig.reloadmap == 0 then
					nextMap = svMaps[randomint(1,table.getn(svMaps))]
				elseif svConfig.reloadmap == 1 and svConfig.randommaps == 0 then
					nextMap = svConfig.map
				end
				promode.svsay("NEXT MAP: "..nextMap)
			end
			if nextMapDelay < (svConfig.reloadtime*6000) then
				nextMapDelay = nextMapDelay + 1
			else
				nextMapDelay = 0
				if svConfig.randommaps == 1 and svConfig.reloadmap == 0 then
					host(nextMap)
				elseif svConfig.reloadmap == 1 and svConfig.randommaps == 0 then
					host(nextMap)
				end
			end
		end
	end

	function doConfig()
		sv_password = svConfig.password
		console.p0_name = "02SERVER"
		console.net_server_name = svConfig.name
		console.net_server_port = svConfig.port
		console.rcon_password = svConfig.rconpass
		console.sv_healthpacks = svConfig.healthpacks
		console.sv_healthpacks_delay = svConfig.healthpacksDelay
		console.sv_pmc = svConfig.pmc
		console.sv_anticamp = svConfig.anticamp
		if gameMode == 0 or gameMode == nil then
			gameMode = svConfig.mode
		end
		timelimit = (svConfig.timelimit*6000)
		killslimit = svConfig.killslimit
		deathlimit = svConfig.deathlimit
		sv_maxclients = svConfig.maxclients+1
		console.net_server_desc = svConfig.desc --.."|"..tostring(gameMode).."|"..tostring(sv_maxclients)
		botMaster[0] = svConfig.botadmin1
		botMaster[1] = svConfig.botadmin2
		botMaster[2] = svConfig.botadmin3
		if svConfig.ircbot == 1 then
			ircbot.connect(svConfig.botserver, svConfig.botchannel, svConfig.botnick)
		end
	end
	
	doConfig()

	function bindings.playerRemoved(p)
		--promode.svsay(p:name().." DISCONNECTED")
	end

	function bindings.playerInit(p)		
		--promode.svsay(p:name().." JOINED THE GAME ")
	end
end

