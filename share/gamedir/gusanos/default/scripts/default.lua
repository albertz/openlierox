function default.init()
	common.init()
end


function default.lgrSetTarg(object)
	local x,y = object:pos()
	local player = object:player()
	if player then
		object:player():data().lgrTargX = x
		object:player():data().lgrTargY = y
	end
end

function default.lgrTurnToTarg(object)
	local player = object:get_player()
	if player then
		local x1,y1 = object:pos()
		local x2 = player:data().lgrTargX
		local y2 = player:data().lgrTargY

		local a = object:get_angle()

		--object:set_angle(vector_direction(x1, y1, x2, y2))
		if ( angle_diff(a,vector_direction(x1, y1, x2, y2)) > 0 ) then
			object:set_angle(a + 5)
		else
			object:set_angle(a - 5)
		end
	end
end