function netauth.init()	
	console_register_command("CL_NETAUTH",function(nick)
		print("launcher netauth "..string.gsub(console["p0_name"], "[%d%s%a%c][%d%s%a%c]", ""))
	end)
	
	console_register_command("SV_NETAUTH",function(nick)
		if nick == nil then
			return ""
		elseif nick ~= nil then
			print("launcher checkauth "..nick)
		end
	end)
end