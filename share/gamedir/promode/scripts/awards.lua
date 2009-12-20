function awards.init()

	bSnd = load_particle("berserkerSound.obj")
	pSnd = load_particle("porSound.obj")
	
	function bindings.playerInit(p)
		p:data().berInit = false
		p:data().berFrags = 0
		p:data().berTimer = 0
		p:data().porCounter = 0
		
		p:data().porCount = 0
		p:data().berCount = 0
	end
	
	function awards.por(w)
		w:player():data().porCounter = w:player():data().porCounter +1
	end
	
	function awards.porReset(w)
		w:player():data().porCounter = 0
	end
	
	function awards.berCheck(object, worm)
		if worm:health() <= 0 then
			if not object:player():data().berInit then
				object:player():data().berFrags = 0
				object:player():data().berTimer = 0
				object:player():data().berInit = true
				object:player():data().berFrags = object:player():data().berFrags + 1
			else
				object:player():data().berFrags = object:player():data().berFrags + 1
			end
		end
	end
			
	function bindings.playerUpdate(p)
		local d = p:data()
		
		if d.berInit then
			d.berTimer = d.berTimer + 1
			
			if d.berTimer > 0 and d.berTimer <= 300 then
				if d.berFrags == 2 then
					--bSnd:put(1, 1)
					d.berCount = d.berCount + 1
					if not DEDSERV then
						--draw.message(4, fonts.liero, "BERSERKER!", 160, 90, 300, 255, 0, 0)
						--draw.message(5, fonts.liero, d.berCount, 160, 99, 300, 255, 255, 255)
					end
					d.berInit = true
					d.berFrags = 1
					d.berTimer = 0
				end
			end
		end
		if d.berTimer > 300 then
			d.berInit = false
			d.berTimer = 0
			d.berFrags = 0
		end
		
		if d.porCounter == 3 then
			--pSnd:put(1, 1)
			d.porCount = d.porCount + 1
			if not DEDSERV then
				--draw.message(4, fonts.liero, "DU BIST POR!", 160, 90, 300, 0, 254, 254)
				--draw.message(5, fonts.liero, d.porCount, 160, 99, 300, 255, 255, 255)
			end
			d.porCounter = 2
		end
	end
end