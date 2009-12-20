function vote.init()

	voteSound = load_particle("voteSound.obj")

	function bindings.playerInit(p)
		local d = p:data()
		
		d.canVote = true
		d.voteTime = 0
	end

	voteSoundPlayed = false
	vote["yes"] = 1
	vote["no"] = 0
	voteInProgress = false
	globalVote = ""
	globalVoteChoose = ""
	voteTime = 0
		
	local voteStart = network_game_event("vs", function(self, player, vote_data)
		voteInProgress = true
		player:data().canVote = true
		vote["yes"] = 1
		vote["no"] = 0
	end)
	
	local voteStop = network_game_event("vsp", function(self, player, vote_data)
		voteInProgress = false
		player:data().canVote = true
		globalVote = ""
		globalVoteChoose = ""
		vote["yes"] = 1
		vote["no"] = 0
	end)
	
	local svote = network_player_event("vsm", 	
	function(self, player, data2)
		if not player:is_local() then
			vote["yes"] = data2:get_int()
			vote["no"] = data2:get_int()
			globalVote = data2:get_string()
			globalVoteChoose = data2:get_string()
			voteInProgress = data2:get_bool()
 		end
	end)

	local pvote = network_player_event("vpm", 	
	function(self, player, data)
		
		if AUTH then
			if not player:is_local() then
				vote["yes"] = data:get_int()
				vote["no"] = data:get_int()
				globalVote = data:get_string()
				globalVoteChoose = data:get_string()
				voteInProgress = data:get_bool()
				local data2 = new_bitstream()
				data2:add_int(vote["yes"])
				data2:add_int(vote["no"])
				data2:add_string(globalVote)
				data2:add_string(globalVoteChoose)
				data2:add_bool(voteInProgress)
				svote:send(player,data2)
 			end
		else
			if not player:is_local() then
				vote["yes"] = data:get_int()
				vote["no"] = data:get_int()
				globalVote = data:get_string()
				globalVoteChoose = data:get_string()
				voteInProgress = data:get_bool()
 			end
		end
	end)	
 		
	function syncVote(player,connID)
		local data = new_bitstream()
		data:add_int(vote["yes"])
		data:add_int(vote["no"])
		data:add_string(globalVote)
		data:add_string(globalVoteChoose)
		data:add_bool(voteInProgress)
		pvote:send(player,data)
	end	
	
	function inGame()
		local count = 0
		
		if isTeamPlay() then
			for p in game_players() do
				if p:team() == 1 or p:team() == 2 then
					count = count + 1
				end
			end
		else
			for p in game_players() do
				if p:team() == 1 then
					count = count + 1
				end
			end
		end
		return count
	end
	
	local voteInfo = false
	
	function bindings.playerUpdate(p)
		local d = p:data()
	
		if voteInProgress and p:team() ~= 0 then
			if not voteSoundPlayed then
				voteSound:put(1, 1)
				voteSoundPlayed = true
			end
		end
		
		if voteInProgress then
			if d.voteTime > 3010 then
				d.voteTime = 0
				voteInProgress = false
				globalVote = ""
				globalVoteChoose = ""
				vote["yes"] = 1
				vote["no"] = 0
				p:data().canVote = true
			end
			d.voteTime = d.voteTime + 1
			if AUTH or DEDSERV then
				if d.voteTime > 3000 or (vote["yes"]+vote["no"]) == inGame() then
					if vote["yes"] > vote["no"] then
						if globalVote == "killslimit" or globalVote == "timelimit" or globalVote == "deathslimit" or globalVote == "mode" or globalVote == "healthpacks" or globalVote == "healthpacks_delay" or globalVote == "anticamp" or globalVote == "pmc" then
							if globalVote == "mode" then
								local str = globalVoteChoose
								local fnd = string.find(str, " ")
								local pos = string.sub(fnd, string.len(fnd)/2)
								local id = string.sub(str, 0, pos-1)
								local map = string.sub(str, pos+1)
								mode.change(id, map)
								console["sv_"..globalVote] = globalVoteChoose
							end
						else
							if globalVote == "kick" then
								local number = tonumber(globalVoteChoose)
								if number > 0 and number <= 10 then
									local players = {}
									
									for p in game_players() do
										table.insert(players, p:name())
									end
									if players[number] ~= "" then
										console[globalVote] = players[number]
									end
								else
									console[globalVote] = globalVoteChoose
								end
							elseif globalVote == "map" then
								for m in maps() do
									if string.lower(globalVoteChoose) == string.lower(m) then
										console[globalVote] = globalVoteChoose
									end
								end
							else
								console[globalVote] = globalVoteChoose
							end
						end
					end
					d.voteTime = 0
					voteInProgress = false
					globalVote = ""
					globalVoteChoose = ""
					vote["yes"] = 1
					vote["no"] = 0
					p:data().canVote = true
					for player in game_players() do
						p:data().canVote = true
						--voteStop:send(player)
						syncVote(player)
					end
				end	
			else
				
			end
			
			if d.voteTime > 3000 or (vote["yes"]+vote["no"]) == inGame() then
				if not voteInfo then
					if vote["yes"] > vote["no"] then
						addMessage("03Vote passed!")
					else
						addMessage("02Vote failed!")
					end
					voteInfo = true
				end
			end
		else
			d.voteTime = 0
			d.canVote = true
			voteSoundPlayed = false
			voteInfo = false
		end
	end
			
	function bindings.viewportRender(viewport, worm)
		local bitmap = viewport:bitmap()
		local p = worm:player()
		
		if voteInProgress and cg_draw2d == 1 and p:data().voteTime <= 3000 then
			if globalVote ~= nil and globalVoteChoose ~= nil then
				--fonts.liero:render(bitmap,"CALLED VOTE: "..globalVote.."  "..globalVoteChoose, bitmap:w()/36, 100, color(255, 255, 0), Font.Shadow)
				fonts.liero:render(bitmap,"05VOTE(06"..((30)-floor(p:data().voteTime/100)).."05): "..string.upper(globalVote).."  "..string.upper(globalVoteChoose).." 03yes: "..vote["yes"].." 02no: "..vote["no"], bitmap:w()/2, 60, color(255, 255, 0),  Font.CenterH+Font.CenterV+Font.Shadow + Font.Formatting)
				--fonts.liero:render(bitmap,"Y: "..vote["yes"]..", N: "..vote["no"], bitmap:w()/36, 109, color(255, 255, 0), Font.Shadow)
				
			end
		end
	end

	console_register_command("CALLVOTE", function(vote, voteChoose, other)
		if vote == nil then
			cecho("Available 05callvote00 commands are:")
			cecho("06MAP                         KICK                    KILLSLIMIT           TIMELIMIT00")
			cecho("06DEATHSLIMIT       TIMELIMIT         HEALTHPACKS       HEALTHPACKS_DELAY00")
			cecho("06ANTICAMP              PMC                     MODE00")
		end
		
		if vote ~= nil and not voteInProgress then
			vote = string.lower(vote)
			if voteChoose ~= nil then
				voteChoose = string.lower(voteChoose)
			end
		
			if game_local_player(local_player_index):team() ~= 0 then
			if tostring(vote) and vote == "map" or vote == "kick" or vote == "killslimit" or vote == "deathslimit" or vote == "timelimit" or vote == "mode" or vote == "healthpacks" or vote == "healthpacks_delay" or vote == "anticamp" or vote == "pmc" then
				if voteChoose ~= nil and tostring(voteChoose) then
					addMessage("Called a vote")
					voteInProgress = true
					globalVote = vote
					
					if other ~= nil then
						globalVoteChoose = voteChoose.." "..other
					else
						globalVoteChoose = voteChoose
					end
					--if AUTH then
					--end
					local b = new_bitstream()
					for player in game_players() do
						player:data().canVote = true
						syncVote(player)
					end
					game_local_player(local_player_index):data().canVote = false
				end
			else
				cecho("Available 05callvote00 commands are:")
				cecho("06MAP                         KICK                    KILLSLIMIT           TIMELIMIT00")
				cecho("06DEATHSLIMIT       TIMELIMIT         HEALTHPACKS       HEALTHPACKS_DELAY00")
				cecho("06ANTICAMP              PMC                     MODE00")
			end
			end
		end			
	end)
	
	for local_player_index = 0,1 do
		console_register_command("P"..local_player_index.."_VOTE",function(str)
			if game_local_player(local_player_index) then
				if str ~= nil then
					if tostring(str) and voteInProgress and game_local_player(local_player_index):data().canVote and game_local_player(local_player_index):team() ~= 0 then
						game_local_player(local_player_index):data().canVote = false
						if str == 'yes' then
							vote["yes"] = vote["yes"] + 1
							game_local_player(local_player_index):data().canVote = false
							for player in game_players() do
								syncVote(player)
							end
						end
		
						if str == 'no' then
							vote["no"] = vote["no"] + 1
							game_local_player(local_player_index):data().canVote = false
							for player in game_players() do
								syncVote(player)
							end
						end
					else
						addMessage("YOU'VE ALREADY VOTED")
					end
				end
			end
		end)
	end
end
