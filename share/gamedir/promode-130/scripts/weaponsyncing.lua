function weaponsyncing.init()
	local grenade_launcher = load_particle("grenade.obj")
	local grenade = load_particle("kgrenade.obj")
	local mortar = load_particle("mortar.obj")
	local rifle = load_particle("rifle_bullet.obj")
	local gauss = load_particle("gauss_bullet_that_makes_hurt.obj")
	local gausslight = load_particle("gauss_trail.obj")
	local shotgun = load_particle("shotgun_generator.obj")
	local flak = load_particle("flak_generator.obj")
	local bazooka = load_particle("missile.obj")
	local mine = load_particle("mine.obj")
	local flaregun = load_particle("flare_gun_bullet.obj")
	local flarefire = load_particle("flare_gun_fire.obj")
	local knife = load_particle("knife.obj")
	local bfg = load_particle("bfg_ball.obj")
	local autocannon = load_particle("autocannon_bullet.obj")

	local n2fix = function( num, prec )
		return math.floor( num * (2^prec) )
	end

	local f2num = function( fix, prec )
		return fix / ( 2^prec )
	end

	local function putObject( object, objtype, x, y, xspd, yspd )
		local ix, iy = object:pos()
		local ixspd, iyspd = object:spd()
		
		object:set_pos( x, y )
		object:set_spd( xspd, yspd )
		
		object:shoot(objtype, 1, 0, 0, 1 )
		
		object:set_pos( ix, iy )
		object:set_spd( ixspd, iyspd )
	end

	local function shootWeapon( worm, x, y, xspd, yspd, type ) -- x and y are fixed point with 8 bit precision, xspd and yspd are fixed point with 16 bit precision!
		if type == "grenade_launcher" then
			putObject( worm, grenade_launcher, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "grenade" then
			putObject( worm, grenade, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "autocannon" then
			putObject( worm, autocannon, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "mortar" then
			putObject( worm, mortar, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "rifle" then
			putObject( worm, rifle, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "gauss" then
			putObject( worm, gauss, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
			putObject( worm, gausslight, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "flak" then
			putObject( worm, flak, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "shotgun" then
			putObject( worm, shotgun, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "bazooka" then
			putObject( worm, bazooka, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "mine" then
			putObject( worm, mine, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "flaregun" then
			putObject( worm, flaregun, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "knife" then
			putObject( worm, knife, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
		end
		if type == "bfg" then
      putObject( worm, bfg, f2num(x, 8), f2num(y, 8), f2num(xspd,16), f2num(yspd, 16) )
    end
	end

	
	local glShootEvent = network_worm_event( "a", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "grenade_launcher" ) 
		end
	)
	
	local grnShootEvent = network_worm_event( "b", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "grenade" ) 
		end
	)
	
	local autocannonShootEvent = network_worm_event( "z", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "autocannon" ) 
		end
	)
	
	local mortarShootEvent = network_worm_event( "c", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "mortar" ) 
		end
	)
	
	local rifleShootEvent = network_worm_event( "d", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "rifle" ) 
		end
	)
	
	local gaussShootEvent = network_worm_event( "e", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "gauss" ) 
		end
	)
	
	local flakShootEvent = network_worm_event( "f", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "flak" ) 
		end
	)
	
	local shotgunShootEvent = network_worm_event( "g", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "shotgun" ) 
		end
	)
	
	local bazookaShootEvent = network_worm_event( "h", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "bazooka" ) 
		end
	)
	
	local mineShootEvent = network_worm_event( "i", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "mine" ) 
		end
	)
	
	local flaregunShootEvent = network_worm_event( "j", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "flaregun" ) 
		end
	)
	
	local knifeShootEvent = network_worm_event( "k", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "knife" ) 
		end
	)
	
	local syncFlareEvent = network_worm_event("l",
	  function(event, worm, data)
			worm:shoot(flarefire,1,0,0,1,0,0,0,0)
			--worm:player():data().on_fire = 400
	end)
	
	local bfgShootEvent = network_worm_event( "m", 
		function(event, worm, data)
			local x = data:get_int()
			local y = data:get_int()
			local vx = data:get_int()
			local vy = data:get_int()
			shootWeapon( worm, x, y, vx, vy, "bfg" ) 
		end
	)
	
  function weaponsyncing.sync_flare( object,worm )
		if AUTH then
			object:shoot(flarefire,1,0,0,1,0,0,0,0)
			--worm:player():data().on_fire = 500
			local b = new_bitstream()
			b:add_bool(true)
			syncFlareEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
		end
	end
	
	function weaponsyncing.shoot_grnlauncher( worm )
		if AUTH then
			local wx, wy = worm:pos()
			local wvx, wvy = worm:spd()
			local ang = worm:angle()
			local vx, vy = angle_vector( ang, 1.85 )
			
			-- Motion inheritance:
			local MotionInheritance = 0.5882
			vx = vx + MotionInheritance * wvx
			vy = vy + MotionInheritance * wvy
			
			-- Get fixed point values ( necesary since there is no add_float binding :P )
			local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
			
			--Shoot the grenade
			shootWeapon( worm, fx, fy, fvx, fvy, "grenade_launcher" )
			
			--Send the message
			local b = new_bitstream()
			b:add_int(fx)
			b:add_int(fy)
			b:add_int(fvx)
			b:add_int(fvy)
			glShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
		end
	end
	
	function weaponsyncing.shoot_grenade( worm )
		if AUTH then
			local wx, wy = worm:pos()
			local wvx, wvy = worm:spd()
			local ang = worm:angle()
			local vx, vy = angle_vector( ang, 1.6 )
			
			-- Motion inheritance:
			local MotionInheritance = 0.5882
			vx = vx + MotionInheritance * wvx
			vy = vy + MotionInheritance * wvy
			
			local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
			shootWeapon( worm, fx, fy, fvx, fvy, "grenade")
			
			--Send the message
			local b = new_bitstream()
			b:add_int(fx)
			b:add_int(fy)
			b:add_int(fvx)
			b:add_int(fvy)
			grnShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
		end
	end
	
	function weaponsyncing.shoot_gauss( worm )
		if AUTH then
			local wx, wy = worm:pos()
			local wvx, wvy = worm:spd()
			local ang = worm:angle()
			local vx, vy = angle_vector( ang, 1.8 )
			
			local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
			shootWeapon( worm, fx, fy, fvx, fvy, "gauss" )
			
			--Send the message
			local b = new_bitstream()
			b:add_int(fx)
			b:add_int(fy)
			b:add_int(fvx)
			b:add_int(fvy)
			gaussShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
		end
	end
	
	function weaponsyncing.shoot_rifle( worm )
		if AUTH then
			local wx, wy = worm:pos()
			local wvx, wvy = worm:spd()
			local ang = worm:angle()
			local vx, vy = angle_vector( ang, 1 )
			
			local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
			shootWeapon( worm, fx, fy, fvx, fvy, "rifle" )
			
			--Send the message
			local b = new_bitstream()
			b:add_int(fx)
			b:add_int(fy)
			b:add_int(fvx)
			b:add_int(fvy)
			rifleShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
		end
	end
	
	function weaponsyncing.shoot_mortar( worm )
		if AUTH then
			local wx, wy = worm:pos()
			local wvx, wvy = worm:spd()
			local ang = worm:angle()
			local vx, vy = angle_vector( ang, 2.52 )
			
			local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
			shootWeapon( worm, fx, fy, fvx, fvy, "mortar" )
			
			--Send the message
			local b = new_bitstream()
			b:add_int(fx)
			b:add_int(fy)
			b:add_int(fvx)
			b:add_int(fvy)
			mortarShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
		end
	end
	
	function weaponsyncing.shoot_flak( worm )
		if AUTH then
			local wx, wy = worm:pos()
			local wvx, wvy = worm:spd()
			local ang = worm:angle()
			local vx, vy = angle_vector( ang, 0.65 )
			
			local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
			shootWeapon( worm, fx, fy, fvx, fvy, "flak" )
			
			--Send the message
			local b = new_bitstream()
			b:add_int(fx)
			b:add_int(fy)
			b:add_int(fvx)
			b:add_int(fvy)
			flakShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
		end
	end
	
	function weaponsyncing.shoot_shotgun( worm )
		if AUTH then
			local wx, wy = worm:pos()
			local wvx, wvy = worm:spd()
			local ang = worm:angle()
			local vx, vy = angle_vector( ang, 1 )
			
			local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
			shootWeapon( worm, fx, fy, fvx, fvy, "shotgun" )
			
			--Send the message
			local b = new_bitstream()
			b:add_int(fx)
			b:add_int(fy)
			b:add_int(fvx)
			b:add_int(fvy)
			shotgunShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
		end
	end
	
	function weaponsyncing.shoot_autocannon( worm )
		if AUTH then
			local wx, wy = worm:pos()
			local wvx, wvy = worm:spd()
			local ang = worm:angle()
			local vx, vy = angle_vector( ang, 1)
			
			local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
			shootWeapon( worm, fx, fy, fvx, fvy, "autocannon" )
			
			--Send the message
			local b = new_bitstream()
			b:add_int(fx)
			b:add_int(fy)
			b:add_int(fvx)
			b:add_int(fvy)
			autocannonShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
		end
	end
	
	function weaponsyncing.shoot_bazooka( worm )
		if AUTH then
			local wx, wy = worm:pos()
			local wvx, wvy = worm:spd()
			local ang = worm:angle()
			local vx, vy = angle_vector( ang, 0.5 )
			
			local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
			shootWeapon( worm, fx, fy, fvx, fvy, "bazooka" )
			
			--Send the message
			local b = new_bitstream()
			b:add_int(fx)
			b:add_int(fy)
			b:add_int(fvx)
			b:add_int(fvy)
			bazookaShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
		end
	end
	
	function weaponsyncing.shoot_mine( worm )
    local pd = worm:player():data()
    if pd.current_mines < 4 then
      if AUTH then
        local wx, wy = worm:pos()
        local wvx, wvy = worm:spd()
        local ang = worm:angle()
        local vx, vy = angle_vector( ang, 1 )
        
        -- Motion inheritance:
        local MotionInheritance = 0.5882
        vx = vx + MotionInheritance * wvx
        vy = vy + MotionInheritance * wvy
        
        local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
        shootWeapon( worm, fx, fy, fvx, fvy, "mine" )
        
        --Send the message
        local b = new_bitstream()
        b:add_int(fx)
        b:add_int(fy)
        b:add_int(fvx)
        b:add_int(fvy)
        mineShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
        pd.current_mines = pd.current_mines + 1
      end
    end
	end
	
	function weaponsyncing.shoot_flaregun( worm )
    local pd = worm:player():data()
    if AUTH then
      local wx, wy = worm:pos()
      local wvx, wvy = worm:spd()
      local ang = worm:angle()
      local vx, vy = angle_vector( ang, 1.2 )
      
      local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
      shootWeapon( worm, fx, fy, fvx, fvy, "flaregun" )
      
      --Send the message
      local b = new_bitstream()
      b:add_int(fx)
      b:add_int(fy)
      b:add_int(fvx)
      b:add_int(fvy)
      flaregunShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
    end
	end
	
	function weaponsyncing.shoot_bfg( worm )
    worm:player():data().bfg_timer = bfg_charge
	end
	
	function weaponsyncing.shoot_main_bfg( worm )
    local pd = worm:player():data()
    if AUTH then
      local wx, wy = worm:pos()
      local wvx, wvy = worm:spd()
      local ang = worm:angle()
      local vx, vy = angle_vector( ang, 1 )
      
      local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
      shootWeapon( worm, fx, fy, fvx, fvy, "bfg" )
      
      --Send the message
      local b = new_bitstream()
      b:add_int(fx)
      b:add_int(fy)
      b:add_int(fvx)
      b:add_int(fvy)
      bfgShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
    end
	end
	
	function weaponsyncing.shoot_knife( worm )
		if AUTH then
			local wx, wy = worm:pos()
			local wvx, wvy = worm:spd()
			local ang = worm:angle()
			local vx, vy = angle_vector( ang, 2.1 )
			
			-- Motion inheritance:
			local MotionInheritance = 0.2
			vx = vx + MotionInheritance * wvx
			vy = vy + MotionInheritance * wvy
			
			local fx, fy, fvx, fvy = n2fix( wx, 8 ), n2fix(wy, 8), n2fix(vx, 16), n2fix(vy, 16)
			shootWeapon( worm, fx, fy, fvx, fvy, "knife")
			
			--Send the message
			local b = new_bitstream()
			b:add_int(fx)
			b:add_int(fy)
			b:add_int(fvx)
			b:add_int(fvy)
			knifeShootEvent:send( worm, b, 0, SendMode.ReliableUnordered, RepRule.Auth2All )
		end
	end
	
end -- wp_gl.init()


	--[[ ]]
