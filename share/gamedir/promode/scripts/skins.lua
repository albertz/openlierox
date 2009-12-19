function skins.init()

	animSpeed = 35/4
	
	function loadSkin(_name, f1, f2, f3, f4)
		table.insert(skins, {
			name = _name,
			anim = {f1, f2, f3, f3},
			frame = {load_particle("skins/".._name.."/1.obj"), load_particle("skins/".._name.."/2.obj"), load_particle("skins/".._name.."/3.obj")}
		})
	end
	
	loadSkin("skin1", 1, 2, 1, 3)
	loadSkin("skin2", 1, 2, 1, 3)
	loadSkin("skin3", 1, 2, 1, 3)
	loadSkin("skin4", 1, 2, 1, 3)
	loadSkin("skin5", 1, 2, 1, 3)
	loadSkin("skin6", 1, 2, 1, 3)
	loadSkin("skin7", 1, 2, 1, 3)
	
	--[[table.insert(skins, {
		name = "skin8", 
		anim = {1, 2, 3, 4},
		frame = {load_particle("skins/skin8/1.obj"), load_particle("skins/skin8/2.obj"), load_particle("skins/skin8/3.obj"), load_particle("skins/skin8/4.obj")}
	})	]]

	function bindings.playerInit(player)
		local d = player:data()
		
		
		if string.sub(player:name(), 0, 3) == "bot" then
			d.model = randomint(0, 7)
		else
			d.model = 0
		end
		
		d.frame = 1
		d.anim = 1
		d.moving = false
	end
	
	
	for local_player_index = 0,1 do
		console_register_command("P"..local_player_index.."_SKIN",function(i)
			if game_local_player(local_player_index) then
				if i == nil or (i*1) > 8 then
					game_local_player(local_player_index):data().model = 0
					skins.sync(game_local_player(local_player_index))
				elseif i ~= nil then
					if tonumber(i) then
						local i = i *1
						game_local_player(local_player_index):data().model = i
						skins.sync(game_local_player(local_player_index))
					else
						return "SKIN: a number is required"
					end
				end
			end
		end)
	end
	
	function bindings.localplayerEvent(player, event, state)
		local d = player:data()
		if player:worm():health() >= 0 then
			if state and event == Player.Left or state and event == Player.Right then
				--if event == Player.Left or event == Player.Right then
					if not d.moving then
						d.frame = 2
					end
					d.moving = true
				--end
			else
				if event == Player.Left  or event == Player.Right then
					d.moving = false
				end
			end
		end
	end
		
	function bindings.playerUpdate(player)
		local worm = player:worm()
		local x,y = worm:pos()
		local angle = worm:angle()
		local d = player:data()
		local vx,vy = worm:spd()
		
		if d.moving then
			if d.anim > animSpeed then
				d.anim = 1
				
				d.frame = d.frame + 1
				
				if d.frame > 4 then
					d.frame = 1
				end
			else
				d.anim = d.anim + 1
			end
		else
			d.anim = 0
			d.frame = 1
		end
			
		if worm:health() > 0 then
			if d.model > 0 then
				if worm:angle() == 360 then
					skins[d.model].frame[skins[d.model].anim[d.frame]]:put(x+vx,y+vy,vx,vy,359.9)
				else
					skins[d.model].frame[skins[d.model].anim[d.frame]]:put(x+vx,y+vy,vx,vy,worm:angle())
				end
			end
		end
	end

	local ms = network_player_event("ms", function(self, p, data)
		local d = p:data()
		local skin = data:get_int(8)
		
		d.model = skin
	end)

	function bindings.playerNetworkInit(p, connID)
		local data = new_bitstream()
		
		if AUTH then
			for pl in game_players() do
				data = new_bitstream()
				data:add_int(pl:data().model, 8)
				ms:send(pl, data, connID)
			end
		end
	end

	function skins.sync(p)
		local data = new_bitstream()
		
		if AUTH then
			for pl in game_players() do
				data = new_bitstream()
				data:add_int(pl:data().model, 8)
				ms:send(pl, data)
			end
		end
	end
end
