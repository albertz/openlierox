function getKey(k)
	return getVar("GameOptions.Ply1Controls." .. k)
end

function map_intro1.init()
	exec("chatMsg", "Hello from the introduction level")

	message("Welcome to OpenLieroX\n"
		.. "Go to the right top of this level!\n\n"
		.. "You can move left/right (keys " .. getKey("Left") .. "/" .. getKey("Right") .. ").\n"
		.. "You can jump (key " .. getKey("Jump") .. ").\n"
		.. "And you should use the ninja rope :) (key " .. getKey("Rope") .. ").\n"
		.. "Release the ninja by jumping (key " .. getKey("Jump") .. ").\n"
		.. "Don't forget to aim (keys " .. getKey("Up") .. "/" .. getKey("Down") .. ")."
		)

end

function goalhit()
	setLevelSucceeded()
end

