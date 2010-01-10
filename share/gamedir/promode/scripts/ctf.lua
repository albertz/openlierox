--by wesz;OOO
function ctf.init()
	ctfMode = true

	
	local teamCount = 0
	
	function ctf.makeTeam(x, y, id)
		teamCount = teamCount + 1
	
		teams[id].flag = teams[id].flagObject:put(x,y)
		--if AUTH then
			teams[id].flag:data().number = id
		--end
		
		teams[id].base = teams[id].baseObject:put(x,y)
		teams[id].base:data().number = id
	end
	
	function ctf.message(msg)
		--if not DEDSERV then
			--draw.message(2, fonts.liero, msg, 160, 80, 300, 255, 255, 255)
			addMessage(msg)
		--end
	end
	
	function ctf.base_reach(base, worm)
		if AUTH then
			local player = worm:player()
			local basenumber = base:data().number
			local playernumber = player:team()
			
			if playernumber == basenumber and player:data().gotflag ~= -1 and not teams[basenumber].stolen then
				ctf.score(playernumber, player:data().gotflag)
				teams[playernumber].vocCapture:put(1, 1)
			end
		end
	end

	function ctf.flag_reach(flag, worm)
	--	if AUTH then
			local player = worm:player()
			local flagnumber = flag:data().number
			local playernumber = player:team()
			if teams[playernumber] ~= nil and teams[playernumber].score ~= nil then
			if teams[playernumber].score then
				if not (teams[flagnumber].owner or playernumber == flagnumber) and player:data().gotflag == -1 then
					teams[flagnumber].owner = player
					teams[flagnumber].stolen = true
					player:data().gotflag = flagnumber
					ctf.setflagowner(player, flagnumber)
					ctf.message(worm:player():name().." got the "..teams[flagnumber].name.." flag")
					teams[playernumber].alarm:put(1, 1)
				end
				if playernumber == flagnumber and teams[flagnumber].stolen and not teams[flagnumber].owner then
					ctf.return_flag(playernumber, player)
					ctf.message(worm:player():name().." defends the flag")
					teams[playernumber].vocReturn:put(1, 1)
				end
			end
			end
		--end
	end

	function ctf.flag_think(flag)
		if teams[flag:data().number].owner then
			local w = teams[flag:data().number].owner:worm()
			if w then
				flag:set_pos(w:pos())
				flag:set_angle(w:angle())
			end
		end
	end

	local Nscore = network_game_event("ns", function(self, data)
		local flagnumber = data:get_int()
		local playernumber = data:get_int()
		ctf.score(playernumber, flagnumber)
		--print("Nscore recieved")
	end)

	function ctf.score(playernumber, flagnumber)
		if teams[playernumber].score then
			teams[playernumber].score = teams[playernumber].score + 1
			
			if teams[flagnumber].owner ~= nil then
				ctf.message(teams[flagnumber].owner:name().." captured the flag. "..teams[playernumber].name.." scores!")
			end
			
			teams[flagnumber].owner:data().gotflag = -1
			teams[flagnumber].owner = nil
			teams[flagnumber].stolen = false
			
			local flag = teams[flagnumber].flag
			flag:set_replication(Particle.Position, true)
			flag:set_pos(teams[flagnumber].base:pos())
			local data = new_bitstream()
				
			if AUTH then
				data:add_int(flagnumber)
				data:add_int(playernumber)
				Nscore:send(data)
				--print("Nscore sent")
			end
		end
	end

	local Nurn = network_game_event("rn", function(self, data)
		local flagnumber = data:get_int()
		ctf.return_flag(flagnumber)
		--print("nurn recieved")
	end)

	function ctf.return_flag(flagnumber)
		local flag = teams[flagnumber].flag
		teams[flagnumber].stolen = false	
		flag:set_replication(Particle.Position, true)
		flag:set_pos(teams[flagnumber].base:pos())
			
		if AUTH then
			local data = new_bitstream()
			data:add_int(flagnumber)
			Nurn:send(data)
			--print("nurn sent")
		end
	end

	local Ndrop = network_game_event("nd", function(self, data)
		local flagnumber = data:get_int()
		ctf.dropflag(flagnumber, x, y)
		--print("ndrop recieved")
	end)

	function ctf.dropflag(flagnumber)
		if flagnumber == -1 then
			return
		end
		
		if not teams[flagnumber].flag then
			--print("dropflag: flag #", flagnumber, " does not exist!")
		else
			local p = teams[flagnumber].owner
			if p ~= nil then
				ctf.message(p:name().." dropped the "..teams[flagnumber].name.." flag")
			end
			teams[flagnumber].owner = nil
			if not p then
				--print("dropflag: nobody seemed to own flag #", flagnumber)
			else
				p:data().gotflag = -1
			end
			
			if AUTH then
				teams[flagnumber].flag:set_replication(Particle.Position, true)
				local data = new_bitstream()
				data:add_int(flagnumber)
				Ndrop:send(data)
				--print("ndrop sent")
			end
		end
	end

	local Nsetflagowner = network_player_event("nso", function(self, player, data)
		local flagnumber = data:get_int()
		teams[flagnumber].owner = player
		teams[flagnumber].stolen = true
		player:data().gotflag = flagnumber
		--print("nsetflagowner recieved")
	end)

	function ctf.setflagowner(player, flagnumber)
    if AUTH then
      teams[flagnumber].flag:set_replication(Particle.Position, false)
      local data = new_bitstream()
      data:add_int(flagnumber)
      Nsetflagowner:send(player, data)
      --print("nsetflagowner sent")
    end
	end

	function bindings.playerInit(player)
		player:data().gotflag = -1
	end

	function bindings.wormDeath(worm)
		if AUTH then
			ctf.dropflag(worm:player():data().gotflag)
		end
	end
	
	function bindings.playerRemoved(player)
		if AUTH then
			ctf.dropflag(player:data().gotflag)
		end
	end

	local Nplayerupdate = network_player_event("npu", function(self, player, data)
		player:data().gotflag = data:get_int()
		--print("nplayerupdate recieved")
	end)

	function bindings.playerNetworkInit(newb, connID)
		if AUTH then
			local data = new_bitstream()
			data:add_int(newb:data().gotflag)
			Nplayerupdate:send(newb, data, connID)
			--print("nplayerupdate sent")
		end
	end
		
	local Nflaginit = network_particle_event("nfi", function(self, particle, data)
		local number = data:get_int()
		particle:data().number = number
		teams[number].flag = particle
		--print("nflaginit recieved")
	end)
	
	function ctf.flagNetworkInit(flag)
    if AUTH then
      local data = new_bitstream()
      data:add_int(flag:data().number)
      Nflaginit:send(flag, data)
      --print("nflaginit sent")
    end
	end

	local Nteaminit = network_game_event("nti", function(self, data)
		local basenumber = data:get_int()
		local teamscore = data:get_int()
		teams[basenumber].score = teamscore
		--print("nteaminit recieved")
	end)

	function bindings.gameNetworkInit(connID)
		for T = 1, teamCount do
			local data = new_bitstream()
			data:add_int(T)
			data:add_int(teams[T].score)
			Nteaminit:send(data, connID)
			--print("nteaminit sent")
		end
	end

	if not DEDSERV then
		function bindings.viewportRender(viewport, worm)
			local bitmap = viewport:bitmap()
			local player = worm:player()
			local playernumber = player:team()
			--if player:data().gotflag ~= -1 then
			--	fonts.liero:render( bitmap, "YOU GOT THE FLAG, RETURN TO BASE", bitmap:w() / 2, 30, color(255, 255, 255), Font.CenterH + Font.CenterV )
			--end
--			if teams[playernumber].stolen then
---				fonts.liero:render( bitmap, "THE ENEMY HAS YOUR FLAG, RETRIEVE IT", bitmap:w() / 2, 22, color(255, 255, 255), Font.CenterH + Font.CenterV )
--			end
		end		
	end

end
