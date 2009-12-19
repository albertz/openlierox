function domination.init()

	local dnPoint = load_particle("domination/point.obj")
	local nonePoint = load_particle("domination/white.obj")
	
	local points = {}
	local pointsCount = 0

	function domination.add(x, y)
		pointsCount = pointsCount + 1
		table.insert(points, {
			pointObject = load_particle("domination/point.obj"),
			id = pointsCount
		})
		
		points[pointsCount].point = points[pointsCount].pointObject:put(x, y)
		--points[pointsCount].point:data().id = pointsCount
		--points[pointsCount].point:data().team = 0
		--points[pointsCount].point:data().angle = 0
		--points[pointsCount].point:data().timer = 0
	end
	
	function domination.pointInit(object)
		object:data().id = pointsCount
		object:data().team = 0
		object:data().angle = 0
		object:data().timer = 0
		object:data().checkTimer = 0
		object:data().toTake = false
	end
	
	local dnp = network_particle_event("dnp", function(self, particle, data)
		local team = data:get_int()
		
		particle:data().team = team
	end)
	
	function domination.pointSetTeam(object, worm)
		if object:data().team ~= worm:player():team() and worm ~= nil then
			toTake = true
			object:data().checkTimer = object:data().checkTimer + 1

			if object:data().checkTimer == 500 then
				object:data().team = worm:player():team()
				if AUTH then
					local data = new_bitstream()
					data:add_int(worm:player():team())
					
					dnp:send(object, data)
				end
				object:data().timer = 0
				object:data().checkTimer = 0
				object:data().toTake = false
			end
		end
	end
	
	function domination.pointThink(object)
		local x, y = object:pos()
		
		if not toTake then
			object:data().checkTimer = 0
		end
		
		object:data().angle = object:data().angle + 1

		if object:data().team == 0 then
			nonePoint:put(x, y, 0, 0, object:data().angle)
			object:data().timer = 0
		else
			if gameEnd == 0 then
				object:data().timer = object:data().timer + 1
				teams[object:data().team].point:put(x, y, 0, 0, object:data().angle)
			
				if object:data().timer == 1000 then
					object:data().timer = 0
					domination.score(object:data().team)
				end
			end
		end
		
		if object:data().angle == 360 then
			object:data().angle = 0
		end
		
		toTake = false
	end
	
	--[[function bindings.playerUpdate(p)
		for i = 1, pointsCount do
			if points[i].point:data().checkTimer > 0 then
				if p:team() == 1 then
					draw.message(4, fonts.liero, math.ceil(points[i].point:data().checkTimer/5).."%", 160, 90, 100, 255, 0, 0)
				elseif p:team() == 2 then
					draw.message(4, fonts.liero, math.ceil(points[i].point:data().checkTimer/5).."%", 160, 90, 100, 90, 90, 255)
				end
			end
		end
	end]]
	
	local dns = network_game_event("dns", function(self, data)
		local team = data:get_int()
		domination.score(team)
	end)
	
	local dni = network_game_event("dni", function(self, data)
		local team = data:get_int()
		local tscore = data:get_int()
		teams[team].score = tscore
	end)
	
	function domination.score(team)
		if teams[team].score and gameEnd == 0 then
			if AUTH then
				teams[team].score = teams[team].score + 1
				
				local data = new_bitstream()
				data:add_int(team)
				data:add_int(teams[team].score)
				dni:send(data, connID)
			end
		end
	end
		
	function domination.pointNetworkInit(point)
		if AUTH then
			local data = new_bitstream()
			data:add_int(point:data().team)
	  
			dnp:send(point, data)
		end
	end

	function bindings.gameNetworkInit(connID)
		for i = 1, 2 do
			local data = new_bitstream()
			data:add_int(i)
			data:add_int(teams[i].score)
			dni:send(data, connID)
		end
	end
end