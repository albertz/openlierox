function chat.init()

	baloon = load_particle("baloon.obj")

	local chatUpdate2 = network_player_event("ch2", function(self, player, data)
		local state = data:get_bool()
		player:data().chat = state
	end)
		
	local chatUpdate = network_player_event("ch", function(self, player, data)
		local state = data:get_bool()
		player:data().chat = state
			
		if AUTH then
			local data2 = new_bitstream()
			data2:add_bool(state)
			chatUpdate2:send(player, data2, 0, SendMode.ReliableUnordered, RepRule.Auth2Proxy)
		end
	end)
		
	local chatStart = network_player_event("chs", function(self, player, data)
		player:data().chat = true
	end)

	local chatStop = network_player_event("chsp", function(self, player, data)
		player:data().chat = false
	end)
		
	function chat.syncc(player, state)
		if state then
			chatStart:send(player)
		else
			chatStop:send(player)
			chat.sync(player,state)
		end
	end

	function chat.sync(player, state)
		player:data().shoot = state
		local data = new_bitstream()
		data:add_bool(state)
		chatUpdate:send(player, data,0, SendMode.ReliableUnordered, RepRule.Owner2Auth)
	end
		
	function bindings.playerUpdate(p)
		local x, y = p:worm():pos()
		
		if p:worm():health() > 0 then
			if p:data().chat then
				baloon:put(x, y - 8, 0, 0, 0)
			end
		end
	end

	if not DEDSERV then
		gui_load_gss("chat")
						
		chatboxw = gui_load_xml("chat")
		
		chatboxw:set_visibility(false)
			
		function chatboxw:onKeyDown(k)
			if k == Keys.ESC then
				self:set_visibility(false)
				game_local_player(0):data().chat = false
				chat.syncc(game_local_player(0), false)
				return true
			end
		end
		
		local chattextw = chatboxw:child("chattext")
		
		function chattextw:onAction()
			local p = game_local_player(0)
			if p then
				if self:text() ~= "" then
					p:say(self:text())
				end
				chatboxw:set_visibility(false)
				game_local_player(0):data().chat = false
				chat.syncc(game_local_player(0), false)
			end
		end
		
		console_register_command("SHOWCHAT", function()
			local worm = game_local_player(0):worm()
			local x,y = worm:pos()
			if game_local_player(0) then
				game_local_player(0):data().chat = true
				chat.sync(game_local_player(0), true)
				chatboxw:set_visibility(true)
				local w = chatboxw:child("chattext")
				w:set_text("")
				w:focus()
				w:activate()
				clear_keybuf()
			end
		end)
	end
end
