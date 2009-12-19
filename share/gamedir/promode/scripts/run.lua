function run.init(sx, sy, fx, fy, fw, fh, wallDetect)

	function bindings.playerInit(p)
		p:stats().mapTime = {min = 0, sec = 0, msec = 0}
		p:stats().bestMapTime = {min = 99, sec = 99, msec = 999}
		p:stats().lastMapTime = {min = 0, sec = 0, msec = 0}
	end
	
	function updateMapTime(p)
		local s = p:stats()
		
		if p:worm():health() > 0 then
			s.mapTime.msec = s.mapTime.msec + 1
			if s.mapTime.msec >= 100 then
				s.mapTime.msec = 0
				s.mapTime.sec = s.mapTime.sec + 1
			end
			if s.mapTime.sec >= 60 then
				s.mapTime.sec = 0
				s.mapTime.min = s.mapTime.min + 1
			end
		end
	end
	
	function resetMapTime(p)
		local s = p:stats()
		
		s.mapTime.msec = 0
		s.mapTime.sec = 0
		s.mapTime.min = 0
	end
	
	function setBestTime(p, table)
		local s = p:stats()
		
		if table.min <= s.bestMapTime.min and table.sec <= s.bestMapTime.sec and table.msec <= s.bestMapTime.msec then
			s.bestMapTime.msec = table.msec
			s.bestMapTime.sec = table.sec
			s.bestMapTime.min = table.min
		end
		s.lastMapTime.msec = table.msec
		s.lastMapTime.sec = table.sec
		s.lastMapTime.min = table.min
	end
	
	function toTime(table)
		local minadd = ""
		local secadd = ""
		local msecadd = ""
		
		if table.min < 10 then
			minadd = "0"
		end
		if table.sec < 10 then
			secadd = "0"
		end
		if table.msec < 10 then
			msecadd = "00"
		end
		if table.msec < 100 and table.msec > 9 then
			msecadd = "0"
		end
		return minadd..table.min..":"..secadd..table.sec..":"..msecadd..table.msec
	end
	
	function isOnWall(p)
		local x, y = p:worm():pos()
		local collisionPoints = 0
		
		if p:worm():health() > 0 then
			for i = 0, 2 do
				if not map_is_particle_pass(x-i, y+5) then
					collisionPoints = collisionPoints + 1
				end
				if not map_is_particle_pass(x+i, y+5) then
					collisionPoints = collisionPoints + 1
				end
				if not map_is_particle_pass(x-i, y-4) then
					collisionPoints = collisionPoints + 1
				end
				if not map_is_particle_pass(x+i, y-4) then
					collisionPoints = collisionPoints + 1
				end
				
				if collisionPoints > 0 then
					break
				end
			end
			
			if collisionPoints == 0 then
				for i = 0, 4 do
					if not map_is_particle_pass(x-2, y+i+1) then
						collisionPoints = collisionPoints + 1
					end
					if not map_is_particle_pass(x-2, y-i) then
						collisionPoints = collisionPoints + 1
					end
					if not map_is_particle_pass(x+2, y+i+1) then
						collisionPoints = collisionPoints + 1
					end
					if not map_is_particle_pass(x+2, y-i) then
						collisionPoints = collisionPoints + 1
					end
					
					if collisionPoints > 0 then
						break
					end
				end
			end
				
			if collisionPoints > 0 then
				return true
			else
				return false
			end
		else
			return false
		end
	end
	
	function isOnFinish(p)
		local x, y = p:worm():pos()
		
		if x >= fx and x <= fx+fw and y >= fy and y <= fy+fh then
			return true
		else
			return false
		end
	end

	function bindings.playerUpdate(p)
		local x, y = p:worm():pos()
		
		if wallDetect then
			if isOnWall(p) then
				p:worm():set_pos(sx, sy)
				p:worm():set_spd(0, 0)
				resetMapTime(p)
			end
		end
		
		if not isOnFinish(p) then
			if p:worm():health() > 0 then
				updateMapTime(p)
			end
		else
			setBestTime(p, p:stats().mapTime)
			resetMapTime(p)
			p:worm():damage(1000)
			p:worm():set_pos(sx, sy)
		end
	end
	
	function bindings.viewportRender(view, worm)
		local b = view:bitmap()
		local p = worm:player()
		
		fonts.liero:render(b, "CURR: "..toTime(p:stats().mapTime), b:w()-5, b:h()-28, color(255, 255, 255), Font.Right + Font.Shadow)
		fonts.liero:render(b, "LAST: "..toTime(p:stats().lastMapTime), b:w()-5, b:h()-19, color(0, 255, 0), Font.Right + Font.Shadow)
		fonts.liero:render(b, "BEST: "..toTime(p:stats().bestMapTime), b:w()-5, b:h()-10, color(255, 255, 0), Font.Right + Font.Shadow)
	end
end