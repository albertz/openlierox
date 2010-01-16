function init_intro1()
	exec("chatMsg", "Hello from the introduction level")

	message("Welcome to OpenLieroX\n"
		.. "Go to the right top of this level!\n"
		.. "Use the ninja rope (key " .. getVar("GameOptions.Ply1Controls.Rope") .. ")")

end

function goalhit()
	setLevelSucceeded()
end

