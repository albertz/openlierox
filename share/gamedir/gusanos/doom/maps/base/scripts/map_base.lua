function map_base.init()
			
	end

local cling = load_particle("clang.obj")
local explode = load_particle("fuel_script_bang.obj")	

local cling2 = load_particle("smallclang.obj")
local hurt = load_particle("hurt.obj")	
			
local cling3 = load_particle("clang_box.obj")	
local boxexplode = load_particle("box_remove.obj")

	function map_base.explode(object)
		if vector_distance( object:spd() ) > 0.1 then
			object:shoot(cling, 1, 0, 0, 0, 0, 0, 0, 0)
	if vector_distance( object:spd() ) > 1.5 then
			object:shoot(explode, 1, 0, 0, 0, 0, 0, 0, 0)
	end
	end

	function map_base.smallcling(object)
		if vector_distance( object:spd() ) > 0.1 then
			object:shoot(cling2, 1, 0, 0, 0, 0, 0, 0, 0)
	end
	end

	function map_base.parts_worm(object)
	if vector_distance( object:spd() ) > 1.8 then
			object:shoot(hurt, 1, 0, 0, 0, 0, 0, 0, 0)
	end
	end

	function map_base.clingbox(object)
		if vector_distance( object:spd() ) > 0.2 then
			object:shoot(cling3, 1, 0, 0, 0, 0, 0, 0, 0)
            if vector_distance( object:spd() ) > 1.8 then
			object:shoot(hurt, 1, 0, 0, 0, 0, 0, 0, 0)
		if vector_distance( object:spd() ) > 2.2 then
			object:shoot(boxexplode, 1, 0, 0, 0, 0, 0, 0, 0)
	end
	end
	end
	end


end

