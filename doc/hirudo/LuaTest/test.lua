
-- test global
myvar = 5235

-- test calling C++ function from lua
awesome.say("I LIVE IN A GIANT BUCKET :D!!")

-- test calling lua functions from C++
function addTwoNumbers (x, y)
	return x + y
end
