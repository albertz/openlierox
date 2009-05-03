function map_ballpark.init()
	local ballOwner = nil
	local balltype = load_particle("poop.obj")
	local timerStart = network_player_event("kotb_timerStart", function(self, p, data)
		p:stats().kotb_timer = data:get_int()
		p:data().kotb_timerRunning = true
		ballOwner = p:worm()
	end)
	
	local timerStop = network_player_event("kotb_timerStop", function(self, p, data)
		p:stats().kotb_timer = data:get_int()
		p:data().kotb_timerRunning = false
		if ballOwner == p:worm() then
			ballOwner = nil
		end
	end)
	
	addScoreField("Ball time", 1, function(p)
		return floor(p:stats().kotb_timer / 100)
	end)
	
	function scoreboardComparer(a, b)
		return a:data().p:stats().kotb_timer > b:data().p:stats().kotb_timer
	end

	function map_ballpark.createStuff()
		balltype:put(80, 80)
	end
	
	function map_ballpark.ballThink(object, worm)
		if ballOwner then
			object:set_pos(ballOwner:pos())
		end
	end
	
	local function pickupBall(worm)
		if not AUTH then return end
		
		if not ballOwner then
			local p = worm:get_player()
			local b = new_bitstream()
			b:add_int(p:stats().kotb_timer)
			timerStart:send(p, b)
			ballOwner = worm
			p:data().kotb_timerRunning = true
		end
	end
	
	function map_ballpark.setOwner(object, worm)
		pickupBall(worm)
	end
	
	local function dropBall(worm)
		if not AUTH then return end
		
		if ballOwner == worm then
			local p = worm:get_player()
			if p then
				local b = new_bitstream()
				b:add_int(p:stats().kotb_timer)
				timerStop:send(p, b)
				p:data().kotb_timerRunning = false
			end
			ballOwner = nil
		end
	end
	
	function bindings.wormDeath(worm)
		dropBall(worm)
	end
	
	function bindings.wormRemoved(worm)
		dropBall(worm)
	end
	
	function bindings.playerInit(p)
		local d = p:data()
		d.kotb_timerRunning = false
		p:stats().kotb_timer = 0
	end
	
	function bindings.playerNetworkInit(p, connID)
		local d = p:data()
		local b = new_bitstream()
		b:add_int(p:stats().kotb_timer)
		if d.kotb_timerRunning then
			timerStart:send(p, b, connID)
		else
			timerStop:send(p, b, connID)
		end
	end
	
	function bindings.playerUpdate(p)
		local d = p:data()
		if d.kotb_timerRunning then
			p:stats().kotb_timer = p:stats().kotb_timer + 1
		end
	end
	
	function bindings.viewportRender(viewport, worm)
		local bitmap = viewport:get_bitmap()
		local p = worm:get_player()
		
		local r, g, b
		
		if p:data().kotb_timerRunning then
			r, g, b = 0, 255, 0
		elseif ballOwner then
			r, g, b = 255, 0, 0
		else
			r, g, b = 255, 255, 255
		end
		
		fonts.liero:render( bitmap, to_time_string(p:stats().kotb_timer), bitmap:w() - 5, 20, r, g, b, Font.CenterV + Font.Right )
	end
end

