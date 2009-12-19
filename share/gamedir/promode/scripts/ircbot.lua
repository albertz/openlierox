function ircbot.init()
	botMaster = {}
		botMaster[0] = ""
		botMaster[1] = ""
		botMaster[2] = ""

	if AUTH then
	function ircbot.parse(str)
		local msg = {}
		local pos = 1
		local t, e = string.match(str, "^:(%S*)%s*()")
		if t then
			msg.prefix = t
			pos = e
		end
		
		local i = 1
		while true do
			t = string.match(str, "^:(.*)", pos)
			if t then
				msg[i] = t
				return msg
			end
			t, e = string.match(str, "^(%S+)%s*()", pos)
			if t then
				msg[i] = t
				pos = e
				i = i + 1
			else
				break
			end
		end
		
		return msg
	end
	
	function ircbot.parseCommand(str)
		local cmd = {}
		
		local t, e = string.match(str, "^/(.-)%s()")
		if not t then
			return nil
		end
		
		cmd.name = string.lower(t)
		local pos = e
		
		local i = 1
		while true do
			t, e = string.match(str, "^(%S+)%s*()", pos)
			if t then
				cmd[i] = t
				pos = e
				i = i + 1
			else
				break
			end
		end
		
		return cmd
	end
	
	local buffer = ""

	function ircbot.connect(server,channel,nick)
		local sock
		local connected = false, nickAttempt
			
		local function setNick(nick)
			nickAttempt = nick
			sock:send("NICK " .. nickAttempt .. "\r\n")
		end
			
		function bindings.playerInit(player)
			local p = player:name()
			sock:send("PRIVMSG " .. channel .. " :" .. strip(p) .. " joined to the game" .. "\r\n")
		end
		
		function bindings.playerRemoved(player)
			local p = player:name()
			sock:send("PRIVMSG " .. channel .. " :" .. strip(p) .. " disconnected" .. "\r\n")
		end
	
		function error()
			sock:send("PRIVMSG " .. channel .. " :" .. "IT WAS NOT DONE YET" .. "\r\n")
		end
	
		sock = tcp_connect(server, 6667)
		sock:send("USER gusanos 0 * :Moo\r\n")
		setNick(nick)
		print("IRCBOT INITIALIZED")
		local buffer = ""
		local asd = 1
		
		function bindings.afterUpdate()
			local s = sock
			local str = s:think()

			if gameEnd > 0 then
				if asd == 1 then
					print("Match finished")
					sock:send("PRIVMSG " .. channel .. " :" .. "Match finished" .. "\r\n")
					for p in game_players() do					
						if p:name() ~= "02SERVER" then
							print(strip(p:name()) .." Kills: "..p:kills().." Deaths: "..p:deaths())
							sock:send("PRIVMSG " .. channel .. " :" .. strip(p:name()) .." Kills: "..p:kills().." Deaths: "..p:deaths() .. "\r\n")
						end
					end		
					asd = 0
				end
			end
			if str then
				buffer = buffer .. str
				local matche = 1
				for w, e in string.gmatch(buffer, "(.-)\r\n()") do
					matche = e
					local msg = ircbot.parse(w)
					if msg and #msg > 0 then
						local nick
						if msg.prefix then
							nick = string.match(msg.prefix, "^(.-)!")
						end
						if msg[1] == "PING" then
							if #msg >= 2 then
								s:send("PONG :" .. msg[2] .. "\r\n")
							end
						elseif msg[1] == "001" then
							connected = true
							--output("* Successfully connected")
							s:send("JOIN " .. channel .. "\r\n")
						elseif msg[1] == "JOIN" then
							if #msg >= 2 then
								--output("* Joined " .. msg[2])
							end
						elseif msg[1] == "433" then
							if not connected then
								setNick(nickAttempt .. "_")
							else
								--output("* Nickname in use")
							end
						elseif msg[1] == "NICK" then
							if nick and #msg >= 2 then
								--output("* " .. nick .. " now known as " .. msg[2])
							end
						elseif msg[1] == "PRIVMSG" then
							if nick and #msg >= 3 then
								if nick == botMaster[0] or nick == botMaster[1] or nick == botMaster[2] or msg[3] == "!score" or msg[3] == "!plist" or msg[3] == "!info" then
									if string.sub(msg[3], 0, 1) == "!" then
										print("<"..nick..">"..msg[3])
									end
									if msg[3] == "!score" then
										local score = "IT WAS NOT DONE YES"
										for p in game_players() do					
											if p:name() ~= "02SERVER" then
												print(strip(p:name()) .." Kills: "..p:kills().." Deaths: "..p:deaths())
												sock:send("PRIVMSG " .. channel .. " :" .. strip(p:name()) .." Kills: "..p:kills().." Deaths: "..p:deaths() .. "\r\n")
											end
										end						
									end
									if msg[3] == "!plist" then
										local id = 0
										for p in game_players() do		
											if p:name() ~= "02SERVER" then
												id = id + 1			
												print(id..". "..strip(p:name()))
												sock:send("PRIVMSG " .. channel .. " :" .. id..". "..strip(p:name()) .."\r\n")
											end
										end						
									end
									if string.sub(msg[3], 0, 4) == "!say" then
										print(nick..": "..string.gsub(msg[3], "!say ", ""))
										console.say = string.gsub(msg[3], "!say ", "")
									end									
									if string.sub(msg[3], 0, 5) == "!kick" then
										console.kick = string.gsub(msg[3], "!kick ", "")
										print(string.gsub(msg[3], "!kick ", "").." kicked")
										sock:send("PRIVMSG " .. channel .. " :" .. string.gsub(msg[3], "!kick ", "").. " kicked" .."\r\n")			
									end
									if string.sub(msg[3], 0, 4) == "!map" then
										print("map changed to "..string.gsub(msg[3], "!map ", ""))
										sock:send("PRIVMSG " .. channel .. " :" .. "map changed to " .. string.gsub(msg[3], "!map ", "") .."\r\n")			
										console.map = string.gsub(msg[3], "!map ", "")
									end
									if string.sub(msg[3], 0, 4) == "!ban" then
										print(string.gsub(msg[3], "!ban ", "").." banned")
										sock:send("PRIVMSG " .. channel .. " :" .. string.gsub(msg[3], "!ban ", "").. " banned" .."\r\n")			
										console.map = string.gsub(msg[3], "!ban ", "")
									end
									if string.sub(msg[3], 0, 11) == "!killslimit" then
										print("killslimit is now "..string.gsub(msg[3], "!killslimit ", ""))
										sock:send("PRIVMSG " .. channel .. " :" .. "killslimit is now "..string.gsub(msg[3], "!killslimit ", "") .."\r\n")			
										console.pm_sv_killslimit = string.gsub(msg[3], "!killslimit ", "")
									end
									if string.sub(msg[3], 0 , 12) == "!deathslimit" then
										print("deathslimit is now "..string.gsub(msg[3], "!deathslimit ", ""))
										sock:send("PRIVMSG " .. channel .. " :" .. "deathslimit is now "..string.gsub(msg[3], "!deathslimit ", "") .."\r\n")
										console.pm_sv_deathslimit = string.gsub(msg[3], "!deathslimit ", "")
									end
									if string.sub(msg[3], 0 , 10) == "!timelimit" then
										print("timelimit is now "..string.gsub(msg[3], "!timelimit ", ""))
										sock:send("PRIVMSG " .. channel .. " :" .. "timelimit is now "..string.gsub(msg[3], "!timelimit ", "") .."\r\n")
										console.pm_sv_timelimit = string.gsub(msg[3], "!timelimit ", "")
									end
									if string.sub(msg[3], 0 , 11) == "!maxclients" then
										print("maxclients is now "..string.gsub(msg[3], "!maxclients ", ""))
										sock:send("PRIVMSG " .. channel .. " :" .. "maxclients is now "..string.gsub(msg[3], "!maxclients ", "") .."\r\n")
										console.pm_sv_maxclients = string.gsub(msg[3], "!maxclients ", "")+1
									end
									if msg[3] == "!addbot" then
										console.addbot = ""
									end
									if msg[3] == "!info" then
										print("Server info")
										local pcount = 0
										local plist = {}
										for p in game_players() do
											if p:name() ~= "02SERVER" then
												pcount = pcount + 1
												table.insert(plist, strip(p:name())..", ")
											end
										end
										print("Name: "..console["net_server_name"].." | Port: "..console["net_server_port"].." | Mode: "..modes_[svConfig.mode].." | Players: "..pcount.." ("..table.concat(plist)..") | Maxclients: "..(sv_maxclients-1).." | Killslimit/Deathslimit/Timelimit: "..killslimit.."/"..deathslimit.."/"..(timelimit/6000).."")
										sock:send("PRIVMSG " .. channel .. " :" .. "Name: "..console["net_server_name"].." | Port: "..console["net_server_port"].." | Mode: "..modes_[svConfig.mode].." | Players: "..pcount.." ("..table.concat(plist)..") | Maxclients: "..(sv_maxclients-1).." | Killslimit/Deathslimit/Timelimit: "..killslimit.."/"..deathslimit.."/"..(timelimit/6000).."" .."\r\n")
									end
									if msg[3] == "!quit" then
										console.quit = ""
									end
									if msg[3] == "!help" then
										print("!addbot [team], !ban [player], !deathslimit [limit], !help, !info, !kick [player], !killslimit [limit], !map [map name], !maxclients [limit], !plist, !score, !say [message], !timelimit [limit], !quit")
										sock:send("PRIVMSG " .. channel .. " :" .. "!addbot [team], !ban [player], !deathslimit [limit], !help, !info, !kick [player], !killslimit [limit], !map [map name], !maxclients [limit], !plist, !score, !say [message], !timelimit [limit], !quit" .."\r\n")			
									end
								end
							end
						end
					end
					--print(w)
				end
				buffer = string.sub(buffer, matche)
			end
		end
	end
	end
end

