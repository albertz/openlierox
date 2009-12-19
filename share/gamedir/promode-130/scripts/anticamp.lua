function anticamp.init()
	
	sv_anticamp = 0
	
	console_register_command("SV_ANTICAMP",function(i)
		if i == nil then
			return "SV_ANTICAMP IS: "..sv_anticamp.." DEFAULT: 0"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 1 and i >= 0 and AUTH then
				sv_anticamp = i
			else
				return "SV_ANTICAMP IS: "..sv_anticamp.." DEFAULT: 0"
			end
		end
	end)
	
	local ac = network_game_event("ac", function(self, data)
		local sv_anticamp_ = data:get_int()
		
		if sv_anticamp_ ~= sv_anticamp then
			sv_anticamp = sv_anticamp_
		end
	end)
    
	function anticamp.send(clients)
		if not clients then 
			clients = 0 
		end
	
		local data = new_bitstream()
		data:add_int(sv_anticamp)
		ac:send(data, clients, SendMode.ReliableOrdered, RepRule.Auth2All)
	end
	
	function bindings.playerNetworkInit(newb, connID)
		if AUTH then
			anticamp.send(connID)
		end
	end

	function bindings.playerInit(p)
		local d = p:data()
		
		d.anticampTimer = 0
	end

	function bindings.playerUpdate(p)
		local d = p:data()
		local x,y = p:worm():pos()

		if sv_anticamp == 1 then
			if p:worm():health() > 0 then
				if not map_is_particle_pass(x, y + 5) or not map_is_particle_pass(x, y + 6) then
					d.anticampTimer = d.anticampTimer + 1
				else
					d.anticampTimer = 0
				end
			end

			if d.anticampTimer == 50 then
				p:worm():damage(1000)
				d.anticampTimer = 0
			end
		end
	end
end
