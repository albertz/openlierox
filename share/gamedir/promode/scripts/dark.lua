function dark.init()
	darkMode = true

	local wormLight = load_particle("worm_light.obj")

	function bindings.playerInit(p)
		p:data().light = false
	end
	
	function dark.lightFollow(obj)
		local x,y = obj:player():worm():pos()
		local vx, vy = obj:player():worm():spd()
		
		obj:set_pos(x,y)
		obj:set_spd(vx,vy)
	end

	function light(contrast)
		return function(x,y,w,h)
			local delta = vector_distance(0, 0, x, y)
			local radius = w/2
			if delta <= radius then
				return ((radius - delta)*(255 * contrast))/radius
			end
			return 0
		end
	end

	dark.light = light(3)
	
	function bindings.wormRender(x, y, worm, viewport, ownerPlayer)
		local ownViewport = ( ownerPlayer == worm:player() )
		local bitmap = viewport:bitmap()
		local wx,wy = ownerPlayer:worm():pos()
		
		if not ownerPlayer:data().light then
			ownerPlayer:worm():shoot(wormLight,1)
			ownerPlayer:data().light = true
		end
	end
end