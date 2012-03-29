print("Hello from Lua, ", system.version)

do
	local t = 0
	local f
	function f()
		print("timer demo ", t, ", cur time: ", system.time)
		t = t + 1
		if t <= 5 then setTimeout(f, 1000) end
	end
	f()
end

function bindings.exit()
	print("Good Bye from Lua")
end

function bindings.serverStart()
	print("Lua: server started …")
end

function bindings.serverStop()
	print("Lua: server stopped …")
end

function bindings.gamePrepare()
	print("Lua: game prepare …")
end

function bindings.gameBegin()
	print("Lua: game begin …")
end

function bindings.gameOver()
	print("Lua: game over …")
end

function bindings.gotoLobby()
	print("Lua: goto lobby …")
end
