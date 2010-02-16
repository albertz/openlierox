


--No need to change anything in this file, unless you want to change hud layout and add more cool stuff :)



function ctf.init()


--INITIALIZE STUFF

	function ctf.start()
		print("CTF-LIB initializing")
		nmax = -1
		base = {}
		flagobj = {}
		flag = {}
		stolen = {}
		teamname = {}
		trailobj = {}
		score = {}
		scoresprite = {}
		r = {}
		b = {}
		n = 0
	end


--BASE THINKING



	

	function ctf.base_reach(base, worm)
		if AUTH then
		 local player = worm:get_player()
		 local basenumber = base:data().number
		 local playernumber = player:data().number

		 if playernumber == basenumber and player:data().gotflag ~= -1 and stolen[basenumber] == 0 then
		  ctf.score(playernumber, player:data().gotflag, player)
		 end
		end
	end

	function ctf.base_think(base)
		local basenumber = base:data().number
		if not flag[basenumber] then
		 local tempflag = flagobj[basenumber]:put(base:pos())
		 tempflag:data().number = basenumber
		 flag[basenumber] = tempflag
		end
	end


--FLAG THINKING

	function ctf.flag_reach(flag, worm)
		if AUTH then
		 local player = worm:get_player()
		 local flagnumber = flag:data().number
		 local playernumber = player:data().number
		 if not (flag:data().owner or playernumber == flagnumber) and player:data().gotflag == -1 then
		  flag:data().owner = player
		  stolen[flagnumber] = 1
		  player:data().gotflag = flagnumber
		  ctf.setflagowner(player, flagnumber)
		 end
		 if playernumber == flagnumber and stolen[flagnumber] == 1 and not flag:data().owner then
		  ctf.return_flag(playernumber, player)
		 end
		end
	end

	function ctf.flag_think(flag)
		local owned
		for player in game_players() do
		 if player == flag:data().owner then
		  owned = 1
		 end
		end
		if not owned then
		 flag:data().owner = nil
		end
		if flag:data().owner then
		 flag:set_pos(flag:data().owner:worm():pos())
		end
	end



--SCORING

	local Nscore = network_player_event("Nscore", function(self, player, data)
		local playernumber = data:get_int()
		local flagnumber = data:get_int()
		ctf.score(playernumber, flagnumber, player)	
	end)

	function ctf.score(playernumber, flagnumber, player)
		score[playernumber] = score[playernumber] + 1
		
		local tempflag = flag[flagnumber]
		stolen[flagnumber] = 0
		flag[flagnumber] = nil
		tempflag:remove()

		player:data().gotflag = -1
		if AUTH then
		 local data = new_bitstream()
		 data:add_int(playernumber)
		 data:add_int(flagnumber)
		 Nscore:send(player, data)
		end
	end


--RETURN FLAG TO BASE (IE. REMOVE IT)

	local Nreturn = network_player_event("Nreturn", function(self, player, data)
		local flagnumber = data:get_int()
		ctf.return_flag(flagnumber, player)	
	end)

	function ctf.return_flag(flagnumber, player)
		local tempflag = flag[flagnumber]
		stolen[flagnumber] = 0
		flag[flagnumber] = nil
		tempflag:remove()
		if AUTH then
		 local data = new_bitstream()
		 data:add_int(flagnumber)
		 Nreturn:send(player, data)
		end
	end

--DROP FLAG (DEATH ETC)

	local Ndrop = network_player_event("Ndrop", function(self, player, data)
		local x = data:get_int()
		local y = data:get_int()
		ctf.dropflag(player, x, y)	
	end)

	function ctf.dropflag(player, x, y)
		if player:data().gotflag ~= -1 then
		 flagnumber = player:data().gotflag
		 flag[flagnumber]:data().owner = nil
		 player:data().gotflag = -1
		 flag[flagnumber]:set_pos(x, y)
		 if AUTH then
		  local data = new_bitstream()
		  data:add_int(x)
		  data:add_int(y)
		  Ndrop:send(player, data)
		 end
		end
	end


--SET FLAG OWNER

	local Nsetflagowner = network_player_event("Nsetflagowner", function(self, player, data)
		local flagnumber = data:get_int()
		flag[flagnumber]:data().owner = player
		stolen[flagnumber] = 1
		player:data().gotflag = flagnumber		
	end)

	function ctf.setflagowner(player, flagnumber)
		local data = new_bitstream()
		data:add_int(flagnumber)
		Nsetflagowner:send(player, data)
	end

--SET FLAG POS

	local Nsetflagpos = network_player_event("Nsetflagpos", function(self, player, data)
		local flagnumber = data:get_int()
		local x = data:get_int()
		local y = data:get_int()
		flag[flagnumber]:set_pos(x, y)		
	end)

	function ctf.setflagpos(flagnumber, x, y)
		local data = new_bitstream()
		data:add_int(flagnumber)
		data:add_int(x)
		data:add_int(y)
		Nsetflagpos:send(game_local_player(0), data)
	end


--SET BASE POS

	local Nsetbasepos = network_player_event("Nsetbasepos", function(self, player, data)
		local basenumber = data:get_int()
		local x = data:get_int()
		local y = data:get_int()
		base[basenumber]:set_pos(x, y)	
	end)

	function ctf.setbasepos(basenumber, x, y)
		local data = new_bitstream()
		data:add_int(basenumber)
		data:add_int(x)
		data:add_int(y)
		Nsetbasepos:send(game_local_player(0), data)
	end

--SET SCORE

	local Nsetscore = network_player_event("Nsetscore", function(self, player, data)
		local basenumber = data:get_int()
		local teamscore = data:get_int()
		score[basenumber] = teamscore
	end)

	function ctf.setscore(teamnumber)
		local data = new_bitstream()
		data:add_int(teamnumber)
		data:add_int(score[teamnumber])
		Nsetscore:send(game_local_player(0), data)
	end



--BASE MAKING

	function ctf.make_base(object1, object2, object3, sprite1, teamnamestring, x, y, r, g, b)
		nmax = nmax + 1
		scoresprite[nmax] = sprite1
		score[nmax] = 0
		teamname[nmax] = teamnamestring
		stolen[nmax] = 0

		flagobj[nmax] = object1
		base[nmax] = object2:put(x,y)
		trailobj[nmax] = object3
		base[nmax]:data().number = nmax
	end


--INIT

	function bindings.playerInit(player)
		player:data().number = n
		player:data().teamname = teamname[n]
		player:data().trailobj = trailobj[n]
		player:data().gotflag = -1
		n = n + 1	
		if n > nmax then
		 n = 0
		end
	end


--WORM REMOVED

	function bindings.wormDeath(worm)
		if AUTH then
		 ctf.dropflag(worm:get_player(), worm:pos())
		end
	end
	
--	function bindings.wormRemoved(worm)
--		if AUTH then
--		 dropflag(worm:get_player(), worm:pos())
--		end
--	end

--UPDATING FOR PLAYERS

	local Nplayerupdate = network_player_event("Nplayerupdate", function(self, player, data)
		local playernumber = data:get_int()
		player:data().gotflag = data:get_int()
		player:data().number = playernumber

		player:data().teamname = teamname[playernumber]
		player:data().trailobj = trailobj[playernumber]
	end)

	function bindings.playerNetworkInit(newb, connID)
		if AUTH then
			for player in game_players() do
			 local data = new_bitstream()
			 data:add_int(player:data().number)
			 data:add_int(player:data().gotflag)
			 Nplayerupdate:send(player, data)
			end
			for T = 0, nmax do
			 if flag[T]:data().owner then
			  ctf.setflagowner(flag[T]:data().owner, T)
			 else
			  ctf.setflagpos(T, flag[T]:pos())
			 end
			  ctf.setbasepos(T, base[T]:pos())
			  ctf.setscore(T)
			end
		end
	end


--MAKE UGLY TRAIL

	function bindings.playerUpdate(player)
		if player:worm():get_health() > 0 then
		 player:data().trailobj:put(player:worm():pos())
		end
	end


--VIEWPORT ADDITIONS

	function bindings.viewportRender(viewport, worm)
		local bitmap = viewport:get_bitmap()
		local player = worm:get_player()
		local playernumber = player:data().number
		if player:data().gotflag ~= -1 then
			fonts.liero:render( bitmap, "YOU GOT THE FLAG, RETURN TO BASE", bitmap:w() / 2, 30, 255, 255, 0, Font.CenterH + Font.CenterV )
		end
		if stolen[playernumber] == 1 then
			fonts.liero:render( bitmap, "THE ENEMY HAS YOUR FLAG, RETRIEVE IT", bitmap:w() / 2, 22, 255, 0, 0, Font.CenterH + Font.CenterV )
		end
		for T = 0, nmax do
			scoresprite[T]:render(bitmap, 0, bitmap:w()-14, 20 + 8 * T)
			fonts.liero:render( bitmap, score[T], bitmap:w() - 10, 20 + 8 * T, 255, 255, 255, Font.CenterH + Font.CenterV )
		end
	end

--RENDERING TEAM NAMES


	function bindings.wormRender(x, y, worm, viewport, ownerPlayer)

		local ownViewport = ( ownerPlayer == worm:get_player() )
		local bitmap = viewport:get_bitmap()
		if not ownViewport then
		 fonts.minifont:render( bitmap, teamname[worm:get_player():data().number], x, y - 15, 255, 255, 255, Font.Formatting + Font.CenterH )
		 --fonts.minifont:render( bitmap, worm:get_player():team(), x, y - 30, 0, 255, 0, Font.CenterH )
		end
	end

end

--TODO:
-- Add text messages
-- Team selection menu