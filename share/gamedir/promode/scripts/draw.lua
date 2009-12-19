function draw.init()

	local layers = 5
	local font = {}
	local message	= {}
	local timer = {}
	local alpha = {}
	local fade	= {}
	local speed = {}
	local x = {}
	local y = {}
	local r = {}
	local g = {}
	local b = {}
	local messages = {"", "", ""}
	local messagesCount = 0
	local messagesTimer = 0
	
	ch_messages_x = 315
	ch_messages_y = 2
	ch_messages_visible = 1
	ch_messages_timer = 1000
	ch_messages_align = 1
	
	console_register_command("CH_MESSAGES_X",function(i)
		if i == nil then
			return "CH_MESSAGES_X IS: "..ch_messages_x.." DEFAULT: 315"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 320 and i >= 0 then
				ch_messages_x = i
			else
				return "CH_MESSAGES_X IS: "..ch_messages_x.." DEFAULT: 315"
			end
		end
	end)
	
	console_register_command("CH_MESSAGES_Y",function(i)
		if i == nil then
			return "CH_MESSAGES_Y IS: "..ch_messages_y.." DEFAULT: 2"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 240 and i >= 0 then
				ch_messages_y = i
			else
				return "CH_MESSAGES_Y IS: "..ch_messages_y.." DEFAULT: 2"
			end
		end
	end)
	
	console_register_command("CH_MESSAGES_TIMER",function(i)
		if i == nil then
			return "CH_MESSAGES_TIMER IS: "..ch_messages_timer.." DEFAULT: 1000"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 10000 and i >= 0 then
				ch_messages_timer = i
			else
				return "CH_MESSAGES_TIMER IS: "..ch_messages_timer.." DEFAULT: 1000"
			end
		end
	end)
	
	console_register_command("CH_MESSAGES_VISIBLE",function(i)
		if i == nil then
			return "CH_MESSAGES_VISIBLE IS: "..ch_messages_visible.." DEFAULT: 1"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 1 and i >= 0 then
				ch_messages_visible = i
			else
				return "CH_MESSAGES_VISIBLE IS: "..ch_messages_visible.." DEFAULT: 1"
			end
		end
	end)
	
	console_register_command("CH_MESSAGES_ALIGN",function(i)
		if i == nil then
			return "CH_MESSAGES_ALIGN IS: "..ch_messages_align.." DEFAULT: 1"
		elseif i ~= nil then
			local i = i *1
			if tonumber(i) and i <= 1 and i >= 0 then
				ch_messages_align = i
			else
				return "CH_MESSAGES_ALIGN IS: "..ch_messages_align.." DEFAULT: 1"
			end
		end
	end)
	
	console_message = ""
	
	console_register_command("_CECHO", function()
		return console_message
	end)
	
	function cecho(msg)
		console_message = msg
		console.exec = "scripts/console_echo.cfg"
	end
	
	function addMessage(msg)
		msg = string.upper(msg)
		messagesTimer = ch_messages_timer
		messagesCount = messagesCount + 1
		messages[2] = messages[1]
		messages[1] = messages[0]
		messages[0] = msg
		cecho(msg)
	end
	
	function draw.message(Layer, Font, Message, X, Y, Timer, R, G, B)
		if message then
			font[Layer] = Font
			message[Layer] = Message
			timer[Layer] = Timer
			alpha[Layer] = 255
			fade[Layer] = 100
			x[Layer] = X
			y[Layer] = Y
			r[Layer] = R
			g[Layer] = G
			b[Layer] = B
			speed[Layer] = 255/fade[Layer]
		end
	end
		
	function draw.update()	
		for layer = 0, layers do
			if message[layer] then
				if timer[layer] > 0 then
					timer[layer] = timer[layer] - 1
					if timer[layer] < fade[layer] and alpha[layer] > 0 then 
						alpha[layer] = alpha[layer] - speed[layer]
					end
					if alpha[layer] < 0 then 
						alpha[layer] = 0 
					end
				end
			end   
		end
	end

	function bindings.afterUpdate()
		draw.update()
		
		if messagesTimer > 0 then
			messagesTimer = messagesTimer - 1
		else
			for i = 0, 2 do
				messages[i] = ""
			end
		end
	end

	function draw.render(bitmap)
		for layer = 0, layers do
			if message[layer] then
				gfx_set_alpha(alpha[layer])
					font[layer]:render(bitmap, message[layer], x[layer], y[layer], color(r[layer], g[layer] ,b[layer]), Font.CenterH + Font.CenterV + Font.Shadow + Font.Formatting)
				gfx_reset_blending()
			end			
		end
	end
	if not DEDSERV then
		function bindings.viewportRender(viewport, worm)
			local bitmap = viewport:bitmap()
			if messagesTimer > 0 and ch_messages_visible == 1 and cg_draw2d == 1 then
				for i = 0, 2 do
					if ch_messages_align == 0 then
						fonts.minifont:render(bitmap, messages[i], ch_messages_x, ch_messages_y + (8*i), color(255, 255, 255), Font.Formatting + Font.Shadow)
					else
						fonts.minifont:render(bitmap, messages[i], ch_messages_x, ch_messages_y + (8*i), color(255, 255, 255), Font.Right + Font.Formatting + Font.Shadow)
					end
				end
			end
			for layer = 0, layers do
				if message[layer] then
					if timer[layer] > 0 then 
						draw.render(bitmap) 
					end
				end
			end
		end
	end
end