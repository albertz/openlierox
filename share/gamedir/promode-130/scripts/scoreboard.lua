function scoreboard.init()

	function strip(string)
		return string.gsub(string, "[%d%s%a%c][%d%s%a%c]", "")	end
	
	if not DEDSERV then
		local scoreVisible = false
		
		console_register_command("+SCORES", function()
			scoreVisible = true
		end)
			
		console_register_command("-SCORES", function()
			scoreVisible = false
		end)

		function bindings.viewportRender(viewport, worm)
			local bitmap = viewport:bitmap()
			local p = worm:player()
			local height = 61
			local tdmHeight = 75
			local splitX = 0
			
			if (scoreVisible or gameEnd > 0) and cg_draw2d == 1 then
				if p == game_local_player(0) then
					splitX = 0
				end
				
				if p == game_local_player(1) then
					splitX = 160
				end
			
				if gameMode == 2 then
					local playerList = {}
					
					for pl in game_players() do
						if pl:team() == 1 then
							table.insert(playerList, {pl:name(), pl:kills(), pl:deaths(), floor(pl:stats().timer / 6000), pl})
						end	
					end
					
					gfx_set_alpha(100)
						bitmap:draw_box(40 - splitX, 25, 279 - splitX, 135, color(20, 20, 20))
					gfx_reset_blending()
					
					if playerList[1] then
						fonts.bigfont:render(bitmap, playerList[1][2], 140 - splitX, 28, color(255, 255, 255), Font.Right + Font.Shadow)
						fonts.liero:render(bitmap, playerList[1][1], 110 - splitX, 28, color(255, 255, 255), Font.Right + Font.Shadow + Font.Formatting)
						fonts.liero:render(bitmap, playerList[1][4], 140 - splitX, 46, color(0, 255, 0), Font.Right + Font.Shadow + Font.Formatting)
						fonts.liero:render(bitmap, "("..floor(playerList[1][5]:kills()/(playerList[1][5]:kills()+playerList[1][5]:deaths())*100).. " %)", 100 - splitX, 110, color(255, 255, 255), Font.Right + Font.Shadow + Font.Formatting)
						fonts.minifont:render(bitmap, playerList[1][5]:kills(), 90 - splitX, 121, color(0, 255, 0), Font.Right + Font.Shadow + Font.Formatting)
						fonts.minifont:render(bitmap, playerList[1][5]:deaths(), 110 - splitX, 121, color(255, 255, 0), Font.Right + Font.Shadow + Font.Formatting)
						fonts.minifont:render(bitmap, playerList[1][5]:data().suicides, 130 - splitX, 121, color(255, 0, 0), Font.Right + Font.Shadow + Font.Formatting)
						for i = 1, 5 do
							if playerList[1][5]:data().atts[playerList[1][5]:data().weaponSelection.list[i]:name()] > 0 then
								fonts.liero:render(bitmap, floor((playerList[1][5]:data().hits[playerList[1][5]:data().weaponSelection.list[i]:name()] / playerList[1][5]:data().atts[playerList[1][5]:data().weaponSelection.list[i]:name()]) * 100).." %", 130 - splitX, 50 + (9 * i), color(255, 255, 255), Font.Right + Font.Shadow + Font.Formatting)
								fonts.liero:render(bitmap, "06"..playerList[1][5]:data().hits[playerList[1][5]:data().weaponSelection.list[i]:name()].."/ "..playerList[1][5]:data().atts[playerList[1][5]:data().weaponSelection.list[i]:name()].."00", 85 - splitX, 50 + (9 * i), color(255, 255, 255), Font.Right + Font.Shadow + Font.Formatting)
								gfx_set_alphach(255)
									if game_local_player(1) and p == game_local_player(0) then
										icons[playerList[1][5]:data().weaponSelection.list[i]:name()]:render(bitmap, 0, 135, 52 + (9 * i))
									else
										if p ~= game_local_player(1) then
											icons[playerList[1][5]:data().weaponSelection.list[i]:name()]:render(bitmap, 0, bitmap:w()/2 - 25, 52 + (9 * i))
										end
									end
								gfx_reset_blending()
							end
						end
					end
					
					if playerList[2] then
						fonts.bigfont:render(bitmap, playerList[2][2], 180 - splitX, 28, color(255, 255, 255), Font.Right + Font.Shadow)
						fonts.liero:render(bitmap, playerList[2][1], 230 - splitX, 28, color(255, 255, 255), Font.Right + Font.Shadow + Font.Formatting)
						fonts.liero:render(bitmap, playerList[2][4], 180 - splitX, 46, color(0, 255, 0), Font.Right + Font.Shadow + Font.Formatting)
						fonts.liero:render(bitmap, "("..floor(playerList[2][5]:kills()/(playerList[2][5]:kills()+playerList[2][5]:deaths())*100).. " %)", 215 - splitX, 110, color(255, 255, 255), Font.Shadow + Font.Formatting)
						fonts.minifont:render(bitmap, playerList[2][5]:kills(), 185 - splitX, 121, color(0, 255, 0), Font.Shadow + Font.Formatting)
						fonts.minifont:render(bitmap, playerList[2][5]:deaths(), 205 - splitX, 121, color(255, 255, 0), Font.Shadow + Font.Formatting)
						fonts.minifont:render(bitmap, playerList[2][5]:data().suicides, 225 - splitX, 121, color(255, 0, 0), Font.Shadow + Font.Formatting)
						for i = 1, 5 do
							if playerList[2][5]:data().atts[playerList[2][5]:data().weaponSelection.list[i]:name()] > 0 then
								fonts.liero:render(bitmap, floor((playerList[2][5]:data().hits[playerList[2][5]:data().weaponSelection.list[i]:name()] / playerList[2][5]:data().atts[playerList[2][5]:data().weaponSelection.list[i]:name()]) * 100).." %", 193 - splitX, 50 + (9 * i), color(255, 255, 255), Font.Shadow + Font.Formatting)
								fonts.liero:render(bitmap, "06"..playerList[2][5]:data().hits[playerList[2][5]:data().weaponSelection.list[i]:name()].."/ "..playerList[2][5]:data().atts[playerList[2][5]:data().weaponSelection.list[i]:name()].."00", 219 - splitX, 50 + (9 * i), color(255, 255, 255), Font.Shadow + Font.Formatting)
								gfx_set_alphach(255)
									if game_local_player(1) and p == game_local_player(1) then
										icons[playerList[2][5]:data().weaponSelection.list[i]:name()]:render(bitmap, 0, 25, 52 + (9 * i))
									else
										if p ~= game_local_player(0) and console.cl_splitscreen == 1 then
											icons[playerList[2][5]:data().weaponSelection.list[i]:name()]:render(bitmap, 0, bitmap:w()/2 - 25, 52 + (9 * i))
										end
									end
								gfx_reset_blending()
							end
						end
					end
					
					fonts.liero:render(bitmap, "Spectators", 160 - splitX, 145, color(255, 255, 0), Font.CenterH+Font.CenterV+Font.Shadow)
					local spectators = {}
					for spct in game_players() do
						if spct:team() ~= 1 then
							table.insert(spectators, spct:name())
						end
					end
					
					spctLine = 1
					spcts = table.getn(spectators)
					
					for i = 1, spcts do
						fonts.liero:render(bitmap, spectators[i], (60*i) - splitX, 154, color(255, 255, 255), Font.Shadow + Font.Formatting)
					end
				elseif isTeamPlay() then
					local scoreRed = 0
					local scoreBlue = 0
					local playerListRed = {}
					local playerListBlue = {}
					
					for pl in game_players() do
						if pl:team()  == 1 then
							table.insert(playerListRed, {pl:name(), pl:kills(), pl:deaths(), floor(pl:stats().timer / 6000)})
						end
						
						if pl:team() == 2 then
							table.insert(playerListBlue, {pl:name(), pl:kills(), pl:deaths(), floor(pl:stats().timer / 6000)})
						end
					end
	
					scoreboardRed = {}
					scoreboardBlue = {}
	
					for index, sort in ipairs(playerListRed) do
						if #scoreboardRed > 0 then
							local name, sort_score = unpack(sort)
							local sorted = false		
		
							for index, scores in ipairs(scoreboardRed) do
								local name, score = unpack(scores)
								if sort_score >= score then
									table.insert(scoreboardRed,index, sort)
									sorted = true
									break
								end
							end
		
							if not sorted then
								table.insert(scoreboardRed, sort)
							end
						else 
							table.insert(scoreboardRed, sort)	
						end
					end
					for index, sort in ipairs(playerListBlue) do
						if #scoreboardBlue > 0 then
							local name, sort_score = unpack(sort)
							local sorted = false		
		
							for index, scores in ipairs(scoreboardBlue) do
								local name, score = unpack(scores)
								if sort_score >= score then
									table.insert(scoreboardBlue,index, sort)
									sorted = true
									break
								end
							end
		
							if not sorted then
								table.insert(scoreboardBlue, sort)
							end
						else 
							table.insert(scoreboardBlue, sort)	
						end
					end
					
					gfx_set_alphach(30)
						bitmap:draw_box(21 - splitX, 49, 160 - splitX, 72, color(255, 0, 0))
						bitmap:draw_box(165 - splitX, 49, 304 - splitX, 72, color(0, 0, 255))
					gfx_reset_blending()
					gfx_set_alphach(60)
						bitmap:draw_box(21 - splitX, 73, 160 - splitX, 82+(9*table.getn(playerListRed)), color(255, 0, 0))
						bitmap:draw_box(165 - splitX, 73, 304 - splitX, 82+(9*table.getn(playerListBlue)), color(0, 0, 255))
					gfx_reset_blending()
					
					fonts.liero:render(bitmap, "Players", 132 - splitX, 51, color(255, 0, 0), Font.Shadow)
					fonts.liero:render(bitmap, "Players", 275 - splitX, 51, color(0, 0, 255), Font.Shadow)
					fonts.liero:render(bitmap, table.getn(playerListRed), 159 - splitX, 62, color(255, 255, 255), Font.Right + Font.Shadow)
					fonts.liero:render(bitmap, table.getn(playerListBlue), 302 - splitX, 62, color(255, 255, 255), Font.Right + Font.Shadow)
					if gameMode == 4 then
						fonts.liero:render(bitmap, "Flags", 92 - splitX, 51, color(255, 0, 0), Font.Shadow)
						fonts.liero:render(bitmap, "Flags", 235 - splitX, 51, color(0, 0, 255), Font.Shadow)
						fonts.liero:render(bitmap, teams[1].score, 119 - splitX, 62, color(255, 255, 255), Font.Right + Font.Shadow)
						fonts.liero:render(bitmap, teams[2].score, 262 - splitX, 62, color(255, 255, 255), Font.Right + Font.Shadow)
					end
					
					for players = 1, table.getn(scoreboardRed) do
						if gameMode ~= 7 and gameMode ~= 8 then
							scoreRed = scoreRed + scoreboardRed[players][2] + (teams[1].score*5)
						else
							scoreRed = scoreRed + teams[1].score
						end
					end
					
					for players = 1, table.getn(scoreboardBlue) do
						if gameMode ~= 7 and gameMode ~= 8 then
							scoreBlue = scoreBlue + scoreboardBlue[players][2] + (teams[2].score*5)
						else
							scoreBlue = scoreBlue + teams[2].score
						end
					end
					fonts.bigfont:render(bitmap, scoreRed, 25 - splitX, 54, color(255, 255, 255), Font.Shadow )
					fonts.bigfont:render(bitmap, scoreBlue, 169 - splitX, 54, color(255, 255, 255), Font.Shadow )
					fonts.minifont:render(bitmap, "Name", 103 - splitX, 75, color(255, 0, 0), Font.Shadow)
					fonts.minifont:render(bitmap, "Net", 84 - splitX, 75, color(255, 0, 0), Font.Shadow)
					fonts.minifont:render(bitmap, "Deaths", 53 - splitX, 75, color(255, 0, 0), Font.Shadow)
					fonts.minifont:render(bitmap, "Score", 27 - splitX, 75, color(255, 0, 0), Font.Shadow)
					fonts.minifont:render(bitmap, "Name", 247 - splitX, 75, color(0, 0, 255), Font.Shadow)
					fonts.minifont:render(bitmap, "Net", 228 - splitX, 75, color(0, 0, 255), Font.Shadow)
					fonts.minifont:render(bitmap, "Deaths", 197 - splitX, 75, color(0, 0, 255), Font.Shadow)
					fonts.minifont:render(bitmap, "Score", 171 - splitX, 75, color(0, 0, 255), Font.Shadow)
					
					for players = 1, table.getn(scoreboardRed) do
						tdmHeight = tdmHeight  + 9
						fonts.minifont:render(bitmap, scoreboardRed[players][1], 103 - splitX, tdmHeight, color(255,255,255), Font.Shadow + Font.Formatting)
					end
					
					tdmHeight = 75
					for players = 1, table.getn(scoreboardRed) do
						tdmHeight = tdmHeight  + 9
						fonts.minifont:render(bitmap, scoreboardRed[players][2] - scoreboardRed[players][3], 96 - splitX, tdmHeight, color(100,254,254), Font.Right + Font.Shadow)
					end
					
					tdmHeight = 75
					for players = 1, table.getn(scoreboardRed) do
						tdmHeight = tdmHeight  + 9
						fonts.minifont:render(bitmap, scoreboardRed[players][3], 77 - splitX, tdmHeight, color(255,0,0), Font.Right + Font.Shadow)
					end
					
					tdmHeight = 75
					for players = 1, table.getn(scoreboardRed) do
						tdmHeight = tdmHeight  + 9
						fonts.minifont:render(bitmap, scoreboardRed[players][2], 47 - splitX, tdmHeight, color(0,255,0), Font.Right + Font.Shadow)
					end
					
					tdmHeight = 75
					for players = 1, table.getn(scoreboardBlue) do
						tdmHeight = tdmHeight  + 9
						fonts.minifont:render(bitmap, scoreboardBlue[players][1], 247 - splitX, tdmHeight, color(255,255,255), Font.Shadow + Font.Formatting)
					end
					
					tdmHeight = 75
					for players = 1, table.getn(scoreboardBlue) do
						tdmHeight = tdmHeight  + 9
						fonts.minifont:render(bitmap, scoreboardBlue[players][2] - scoreboardBlue[players][3], 240 - splitX, tdmHeight, color(100,254,254), Font.Right + Font.Shadow)
					end
					
					tdmHeight = 75
					for players = 1, table.getn(scoreboardBlue) do
						tdmHeight = tdmHeight  + 9
						fonts.minifont:render(bitmap, scoreboardBlue[players][3], 221 - splitX, tdmHeight, color(255,0,0), Font.Right + Font.Shadow)
					end
					
					tdmHeight = 75
					for players = 1, table.getn(scoreboardBlue) do
						tdmHeight = tdmHeight  + 9
						fonts.minifont:render(bitmap, scoreboardBlue[players][2], 191 - splitX, tdmHeight, color(0,255,0), Font.Right + Font.Shadow)
					end
					
					if table.getn(scoreboardRed) > table.getn(scoreboardBlue) then
						height = 67 + (9 * table.getn(scoreboardRed)) + 28
					else
						height = 67 + (9 * table.getn(scoreboardBlue)) + 28
					end
					
					fonts.liero:render(bitmap, "Spectators", 160 - splitX, height, color(255, 255, 0), Font.CenterH+Font.CenterV+Font.Shadow)
					local spectators = {}
					for spct in game_players() do
						if spct:team() ~= 1 and spct:team() ~= 2 then
							table.insert(spectators, spct:name())
						end
					end
					
					spctLine = 1
					spcts = table.getn(spectators)
					
					for i = 1, spcts do
						fonts.liero:render(bitmap, spectators[i], (60*i) - splitX, height + 9, color(255, 255, 255), Font.Shadow + Font.Formatting)
					end
				else
					local playerList = {}
					
					for pl in game_players() do
						if pl:team() == 1 then
							if gameMode == 6 then
								table.insert(playerList, {pl:name(), pl:stats().kotb_timer, pl:kills(), pl:deaths()})
							else
								table.insert(playerList, {pl:name(), pl:kills(), pl:deaths(), floor(pl:stats().timer / 6000)})
							end
						end	
					end
	
					scoreboard = {}
	
					for index, sort in ipairs(playerList) do
						if #scoreboard > 0 then
							local name, sort_score = unpack(sort)
							local sorted = false		
		
							for index, scores in ipairs(scoreboard) do
								local name, score = unpack(scores)
								if sort_score >= score then
									table.insert(scoreboard,index, sort)
									sorted = true
									break
								end
							end
		
							if not sorted then
								table.insert(scoreboard, sort)
							end
						else 
							table.insert(scoreboard, sort)	
						end
					end
					
					gfx_set_alpha(50)
						bitmap:draw_box(60 - splitX, 68, 260 - splitX, 76, color(220, 220, 220))
					gfx_reset_blending()
					
					fonts.liero:render(bitmap, "Name", 171 - splitX, height, color(255,255,0), Font.Shadow)
					for players = 1, table.getn(scoreboard) do
						height = height  + 9
						fonts.liero:render(bitmap, scoreboard[players][1], 171 - splitX, height, color(255,255,255), Font.Shadow + Font.Formatting)
					end
					
					if gameMode == 6 then
						height = 61
						fonts.liero:render(bitmap, "Score", 80 - splitX, height, color(255, 255, 0), Font.Shadow)
						for players = 1, table.getn(scoreboard) do
							height = height + 9
							fonts.liero:render(bitmap, scoreboard[players][3], 101 - splitX, height, color(0, 255, 0), Font.Right + Font.Shadow)
						end
						
						height = 61
						fonts.liero:render(bitmap, "Deaths", 105 - splitX, height, color(255, 255, 0), Font.Shadow)
						for players = 1, table.getn(scoreboard) do
							height = height + 9
							fonts.liero:render(bitmap, scoreboard[players][4], 130 - splitX, height, color(255, 0, 0), Font.Right + Font.Shadow)
						end
						
						height = 61
						fonts.liero:render(bitmap, "Time", 150 - splitX, height, color(255,255,0), Font.Shadow)
						for players = 1, table.getn(scoreboard) do
							height = height + 9
							fonts.liero:render(bitmap, to_time_string(scoreboard[players][2]), 167 - splitX, height, color(100, 254, 254), Font.Right + Font.Shadow)
						end
					else
						height = 61
						fonts.liero:render(bitmap, "Min", 155 - splitX, height, color(255,255,0), Font.Shadow)
						for players = 1, table.getn(scoreboard) do
							height = height + 9
							fonts.liero:render(bitmap, scoreboard[players][4], 167 - splitX, height, color(100, 254, 254), Font.Right + Font.Shadow)
						end
						
						height = 61
						fonts.liero:render(bitmap, "Net", 137 - splitX, height, color(255, 255, 0), Font.Shadow)
						for players = 1, table.getn(scoreboard) do
							height = height + 9
							fonts.liero:render(bitmap, scoreboard[players][2] - scoreboard[players][3], 150 - splitX, height, color(255, 255, 255), Font.Right + Font.Shadow)
						end
						
						height = 61
						fonts.liero:render(bitmap, "Deaths", 105 - splitX, height, color(255, 255, 0), Font.Shadow)
						for players = 1, table.getn(scoreboard) do
							height = height + 9
							fonts.liero:render(bitmap, scoreboard[players][3], 130 - splitX, height, color(255, 0, 0), Font.Right + Font.Shadow)
						end
						
						height = 61
						fonts.liero:render(bitmap, "Score", 80 - splitX, height, color(255, 255, 0), Font.Shadow)
						for players = 1, table.getn(scoreboard) do
							height = height + 9
							fonts.liero:render(bitmap, scoreboard[players][2], 101 - splitX, height, color(0, 255, 0), Font.Right + Font.Shadow)
						end
					end
					height = 69 + (9 * table.getn(scoreboard)) + 18
					fonts.liero:render(bitmap, "Spectators", 160 - splitX, height, color(255, 255, 0), Font.CenterH+Font.CenterV+Font.Shadow)
					local spectators = {}
					for spct in game_players() do
						if spct:team() ~= 1 then
							table.insert(spectators, spct:name())
						end
					end
					
					spctLine = 1
					spcts = table.getn(spectators)
					
					for i = 1, spcts do
						--if spcts <= 3 then
							fonts.liero:render(bitmap, spectators[i], (60*i) - splitX, height + 9, color(255, 255, 255), Font.Shadow + Font.Formatting)
						--end
					end
				end			
			end
		end
	end
end
