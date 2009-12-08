function map_mars.init()
			
	end

local cling = load_particle("clang.obj")
local explode = load_particle("fuel_script_bang.obj")	

local cling2 = load_particle("smallclang.obj")
local hurt = load_particle("hurt.obj")	
			
	function map_mars.explode(object)
		if vector_distance( object:spd() ) > 0.1 then
			object:shoot(cling, 1, 0, 0, 0, 0, 0, 0, 0)
	if vector_distance( object:spd() ) > 1.5 then
			object:shoot(explode, 1, 0, 0, 0, 0, 0, 0, 0)
	end
	end


	function map_mars.smallcling(object)
		if vector_distance( object:spd() ) > 0.1 then
			object:shoot(cling2, 1, 0, 0, 0, 0, 0, 0, 0)
	end
	end

	function map_mars.parts_worm(object)
	if vector_distance( object:spd() ) > 1.8 then
			object:shoot(hurt, 1, 0, 0, 0, 0, 0, 0, 0)
	end
	end


end

