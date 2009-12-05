	function ut.rocketGetTarget(worm)
		local player = worm:get_player()
		if ammopool.ammoEmptyCheck("rocket", 1, player) then
			if not player:data().rockettarget then
				if not player:data().rockettargeting then
					for p in game_players() do
						if player ~= p then
							if ut.objectInArc(90,p:worm(),worm) then
								local x1,y1 = worm:pos()
								local x2,y2 = p:worm():pos()
								if not map_is_blocked(x1,y1,x2,y2) then
									player:data().rockettargeting = true
									player:data().rocketpretarget = p:worm()
									print(player:data().rocketpretarget:get_player():name())
								end
							end
						end
					end
				else
					if player:data().rocketpretarget then
						local x1,y1 = worm:pos()
						local x2,y2 = player:data().rocketpretarget:pos()
						if ut.objectInArc(90,player:data().rocketpretarget,worm) then

							if not map_is_blocked(x1,y1,x2,y2) then
								if player:data().rockettargetcount < 400 and not player:data().rockettarget then
									player:data().rockettargetcount = player:data().rockettargetcount + 1
									print(player:data().rockettargetcount)
								else
									player:data().rockettarget = player:data().rocketpretarget
									print(player:data().rocketpretarget:get_player():name())
									if player:data().localplayer == true then
										rocketsound_3:put(x1,y1)
									end
									local x,y = worm:pos()
								end
							else
								ut.rocketInit(player)
							end
						else
							ut.rocketInit(player)
						end
					else
						ut.rocketInit(player)
					end
				end
			else
				local x1,y1 = worm:pos()
				local x2,y2 = player:data().rockettarget:pos()
				if not ut.objectInArc(90,player:data().rockettarget,worm) or map_is_blocked(x1,y1,x2,y2) then
					ut.rocketInit(player)
					if player:data().localplayer == true then
						rocketsound_4:put(x1,y1)
					end
				end

			end
		end
	end

	function ut.rocketInit(player)
		d = player:data()
		d.rockettarget = nil
		d.rocketpretarget = nil
		d.rockettargeting = false
		d.rockettargetcount = 0
	end

	function ut.rocketRelease(object)
		local player = object:get_player()
		player:data().rocketloading = false
		player:data().grenadeloading = false
	end


on timer(0)
  run_script(ut.rocketGuidance)

	function ut.rocketGuidance(object)
		if object:data().guided then
			local objdata = object:data()
			local target = objdata.target

			if ut.objectInArc(180,target,object) then
				print("guide")
				local Tx,Ty = target:pos()
				local x,y = object:pos()

				local setangle = object:get_angle()
				if vector_direction(x,y,Tx,Ty) ~= object:get_angle() then
					if angle_diff( vector_direction(x,y,Tx,Ty) , object:get_angle() ) > 0 then
						setangle = setangle -2
					else
						setangle = setangle +2
					end
				end
				ut.setObjMoveAngle(object, setangle)
			end
		end
	end


on creation()
 run_script(ut.rocketTargetInit)

	function ut.rocketTargetInit(object)
		player = object:get_player()
		if player:data().rockettarget then
			object:data().target = player:data().rockettarget
			object:data().guided = true
			print("lock")
		end
	end