function weaponeffects.init()

	can_sound1 = load_particle("can_bounce_soundgen.obj")
	grenade_sound2 = load_particle("grenade_bounce_soundgen.obj")
	shell_sound = load_particle("shell_bounce_soundgen.obj")
	meat_sound = load_particle("meat_bounce_soundgen.obj")
	bfg_windup_light = load_particle("bfg_windup_light.obj")
	bfg_spark = load_particle("bfg_spark.obj")
	
	laserColor = {r = 255, g = 0, b = 0}
	
	
	console_register_command("CG_LASERCOLOR",function(i,j,k)
		if i == nil then
			return "CG_LASERCOLOR IS: "..laserColor.r.." "..laserColor.g.." "..laserColor.b.." DEFAULT: 255 0 0"
		elseif i ~= nil then
			local i = i *1
			local j = j *1
			local k = k *1
			if tonumber(i) and tonumber(j) and tonumber(k) then
				if i >= 0 and i <= 255 then
					if j >= 0 and j <= 255 then
						if k >= 0 and k <= 255 then
							laserColor.r = tonumber(i)
							laserColor.g = tonumber(j)
							laserColor.b = tonumber(k)
						end
					end
				end
			else
				return "CG_LASERCOLOR IS: "..laserColor.r.." "..laserColor.g.." "..laserColor.b.." DEFAULT: 255 0 0"
			end
		end
	end)

	function weaponeffects.blood_damage(object)
		object:damage(50)
	end

--DATA INIT
	function bindings.playerInit(player)
		player:data().white_flash = 0
		player:data().zap = false
		player:data().draw_firecone = false
		player:data().current_mines = 0
		player:data().on_fire = 0
	end
	
	function bindings.wormDeath(worm)
		worm:player():data().white_flash = 0
		worm:player():data().zap = false
		worm:player():data().draw_firecone = false
	end

--UPDATE ON EACH FRAME
	function bindings.playerUpdate(player)
    local pd = player:data()
	
	--FLASHES
		if pd.white_flash > 0 then
			pd.white_flash = pd.white_flash - 4
		end
		
		if pd.white_flash < 0 then
			pd.white_flash = 0
		end
		
		if player:worm():health() <= 0 then
			pd.white_flash = 0
			pd.zap = false
		end
		
	--FLAREGUN FIRE
  --[[  if pd.on_fire > 0 then
      pd.on_fire = pd.on_fire - 1
      player:worm():damage(0.125)
    end]]--
	end

--FLASH DRAWING
	function bindings.viewportRender(viewport, worm)
		local bitmap = viewport:bitmap()
		local player = worm:player()
		
		if player:data().white_flash >= 0 and worm:health() > 0 then
			gfx_set_add(player:data().white_flash)
			bitmap:draw_box(0, 0, 320, 240, color(150,150,150))
			gfx_reset_blending()
		end
	end
	
--BLOOD ALIGN
  function weaponeffects.checkBloodCollision(object)
		local x,y = object:pos()
		
		if not map_is_particle_pass(x,y+1) then
			object:set_angle(180)
		elseif not map_is_particle_pass(x,y-1) then
			object:set_angle(360)
		elseif not map_is_particle_pass(x+1,y) then
			object:set_angle(90)
		elseif not map_is_particle_pass(x-1,y) then
			object:set_angle(270)
		end
	end
	
--FLASH INITS
--[[
	function weaponeffects.white_flash_large(object,target)
		local player = target:player()
		local worm = player:worm()
		local ox,oy = object:pos()
		local x,y = worm:pos()

		--print(vector_distance(ox,oy,x,y))
		player:data().white_flash = player:data().white_flash + (150-vector_distance(ox,oy,x,y)*1.5)

		--print(player:data().white_flash)

		if player:data().white_flash > 150 then
			player:data().white_flash = 150
		end
	end
	
	function weaponeffects.white_flash_small(object,target)
		local player = target:player()
		local worm = player:worm()
		local ox,oy = object:pos()
		local x,y = worm:pos()

		--print(vector_distance(ox,oy,x,y))
		player:data().white_flash = player:data().white_flash + (150-vector_distance(ox,oy,x,y)*2)

		--print(player:data().white_flash)

		if player:data().white_flash > 100 then
			player:data().white_flash = 100
		end
	end
	
	function weaponeffects.white_flash_minimal(object,target)
		local player = target:player()
		local worm = player:worm()
		local ox,oy = object:pos()
		local x,y = worm:pos()

		--print(vector_distance(ox,oy,x,y))
		player:data().white_flash = player:data().white_flash + (100-vector_distance(ox,oy,x,y)*2)

		--print(player:data().white_flash)

		if player:data().white_flash > 40 then
			player:data().white_flash = 40
		end
	end
]]--
--BOUNCE SOUND SCRIPTS
	function weaponeffects.can_sound(object)
		if vector_distance( object:spd() ) > 0.5 then
			object:shoot(can_sound1, 1, 0, 0, 0, 0, 0, 0, 0)
		end
	end
	
	function weaponeffects.grenade_sound(object)
		if vector_distance( object:spd() ) > 0.5 then
			object:shoot(grenade_sound2, 1, 0, 0, 0, 0, 0, 0, 0)
		end
	end
	
	function weaponeffects.shell_sound(object)
		if vector_distance( object:spd() ) > 0.2 then
			object:shoot(shell_sound, 1, 0, 0, 0, 0, 0, 0, 0)
		end
	end
	
	function weaponeffects.meat_sound(object)
		if vector_distance( object:spd() ) > 0.2 then
			object:shoot(meat_sound, 1, 0, 0, 0, 0, 0, 0, 0)
		end
	end

--MISC
	function weaponeffects.distance_to_line(x1,y1,x2,y2,x3,y3)
		return math.abs( (y2-y1)*x3 + (x1-x2)*y3 + (y1*x1 - y2*x1 + x2*y1 - x1*y1) )/math.sqrt( (y2-y1)*(y2-y1) + (x1-x2)*(x1-x2) )
	end	

	function weaponeffects.objectInArc(angle, object, viewer)
		local x1, y1 = viewer:pos()
		local x2, y2 = object:pos()
		local relAngle = vector_direction( x1,y1,x2,y2 )
		if abs(angle_diff(viewer:angle(), relAngle )) <= (angle/2) then
			return true
		else
			return false
		end
	end

--MINES

  function weaponeffects.decrementMineCount(object)
    object:player():data().current_mines = object:player():data().current_mines - 1
  end
  
--KNIVES

  function weaponeffects.knifeAlign(object)
    local x,y = object:spd()
    object:set_angle(vector_direction(x, y, 0, 0)-180)
  end
  
--FLARE GUN

  function weaponeffects.flarefire_init(object)
    object:data().target = "none"
  end
  
  function weaponeffects.flarefire_set_target(object,worm)
    if object:data().target == "none" then
      object:data().target = worm
      local x,y = worm:pos()
      object:set_pos(x,y)
    end
  end
  
  function weaponeffects.flarefire_align(object)
    if object:data().target ~= "none" then
      local x,y = object:data().target:pos()
      object:set_pos(x,y)
    end
  end
  
--BFG

  bfg_charge = 70

	function bindings.playerInit(player)
    player:data().bfg_timer = 0
  end
  
  function bindings.playerUpdate(player)
    local worm = player:worm()
    if worm:current_weapon():type():name() == "BFG 9000" then
      if player:data().bfg_timer > 0 then
        player:data().bfg_timer = player:data().bfg_timer - 1
        worm:shoot(bfg_windup_light,math.floor((bfg_charge-player:data().bfg_timer)/2),0,0,0,0,0,0,8)
        worm:shoot(bfg_spark,math.floor((bfg_charge-player:data().bfg_timer)/2),3,2,0,1,80,0,2)
      end
   
      if player:data().bfg_timer == 1 then
        weaponsyncing.shoot_main_bfg(worm)
        player:data().bfg_timer = 0
        worm:push(angle_vector(worm:angle(),-1.5))
      end
    else
      player:data().bfg_timer = 0
    end
  end
  
  function bindings.wormDeath(worm)
    worm:player():data().bfg_timer = 0
  end

--LIGHTNING GUN

	--[[function bindings.localplayerEvent(player, event, state)
 		local w = player:worm():current_weapon()
 		if event == Player.Fire and w:type():name() == "Lightning Gun" and not w:is_reloading() then
			if state then
				player:data().zap = true
				--return false
			else
				player:data().zap = false
				--return true
			end
 		end
 	end]]--
	
	function weaponeffects.lg_start_sound(worm)
		if not DEDSERV then
			local lg_fire = sounds["lg_fire.ogg"]
			local x,y = worm:pos()

			if worm:current_weapon():is_reloading() == false then
				lg_fire:play(x,y,100,1,0.05)
			end
		end
	end

	function weaponeffects.lg_hit_sound(object)
		if not DEDSERV then
			local x,y = object:pos()
			local lg_hit1 = sounds["lg_hit1.ogg"]
			local lg_hit2 = sounds["lg_hit2.ogg"]
			local lg_hit3 = sounds["lg_hit3.ogg"]
	
			if randomint(1,10) == 7 then
				lg_hit1:play(x,y,50,1,0.05)
			end
	
			if randomint(1,10) == 8 then
				lg_hit2:play(x,y,50,1,0.05)
			end
	
			if randomint(1,10) == 9 then
				lg_hit3:play(x,y,50,1,0.05)
			end
		end
	end

	function weaponeffects.lg_start_zap(worm)
		local player = worm:player()
		player:data().zap = true
	end
	
	function weaponeffects.lg_end_zap(worm)
		local player = worm:player()
		player:data().zap = false
	end

	--THE LIGHTNING GUN IS RENDERED HERE ONLY, NO GFX OBJECTS IN GAME EXCEPT THOSE ON HIT
	function bindings.wormRender(x, y, worm, viewport, ownerPlayer)
		local player = worm:player()
		local ownViewport = ( ownerPlayer == worm:player() )
		local bitmap = viewport:bitmap()
		local lg_flash = sprites_load("lg_flash")
	
		if worm:current_weapon():type():name() == "Lightning Gun" then
			if worm:current_weapon():is_reloading() == false then
				if player:data().zap == true then

				--BEAM ADJUSTING VALUES, THESE ARE ONLY FOR GRAPHICS AS DAMAGE IS DEALT BY IN-GAME OBJECT
					local range = 145		--TOTAL MAXIMAL LENGTH OF THE BEAM. WARNING: REMEMBER TO CHANGE MAIN DAMAGING OBJECT'S REMOVE TIME! (CURRENTLY IT IS LG_MAIN.OBJ)
					local detect = 2		--DETECT RANGE FOR BEAM (ONLY GFX!)
					local stages = randomint(18,30) --TOTAL MAXIMAL AMOUNT OF STAGES
					local width = 2			--MAXIMAL WIDTH OF LIGHTNING BEAM

				--DON'T TOUCH THESE VALUES
					local temp = 0		
					local tx,ty = worm:pos()--POSITION OF PLAYER'S WORM
					local sx,sy		
					local decr = range	--SETTING ACTUAL RANGE TO MAX, IT WILL BE REDUCED LATER IF BEAM HITS WORM OR WALL
					local fx = {}		--WAYPOINT TABLES
					local fy = {}
					local etx,ety		--POSITION OF ENEMY WORM
					local players = 0	--AMOUNT OF PLAYERS IN GAME
					
				--CHECKING AMOUNT OF PLAYERS IN GAME
					for t in game_players() do
						players = players + 1
					end
					--print(players)
					
					if players > 1 then		
				--THIS PART OF CODE CHECKS IF THERE IS A WORM CLOSE TO LINE OF THE LG, CLOSE ENOUGH TO PLAYER'S WORM AND IS IN FRONT OF PLAYER'S WORM
				--IF YES REDUCES LENGTH OF THE BEAM
						for k in game_players() do
							if k ~= player and k:worm():health() > 0 then
								etx,ety = k:worm():pos()
					
								while decr > 0 do
									local dx, dy = angle_vector(worm:angle(), decr)
									if (weaponeffects.distance_to_line(tx,ty,tx+dx,ty+dy,etx,ety) < 5 and vector_distance(tx,ty,etx,ety) < decr and weaponeffects.objectInArc(180,k:worm(),player:worm()) == true) then
										decr = decr - 5
									else			
										break
									end
								end
							end
						end

					--THIS CODE CHECKS IF THERE IS A WALL IN FRONT OF THE WORM, IF YES REDUCES LENGTH OF THE BEAM
						while decr > 0 do
							local dx, dy = angle_vector(worm:angle(), decr)
							if map_is_blocked(tx,ty,tx+dx,ty+dy) then
								decr = decr - 5
							else			
								break
							end
						end
						
					--NOW THE BEAM HAS ADJUSTED LENGTH AND IS BEING DRAWN 
						if temp <= decr then
						--SETTING NEEDED "LIGHTNIGNG STAGES" AMOUNT, DEPENDS ON ACTUAL LENGTH OF THE BEAM
							stages = floor((decr/range) * stages + 1)
							
						--SETTING RANDOM "WAYPOINTS" FOR LIGHTNING
							for i=1,stages do
								fx[i],fy[i] = angle_vector(worm:angle(),decr/(stages+1)*i)
								fx[i] = fx[i] + randomint(-width,width)
								fy[i] = fy[i] + randomint(-width,width)
								sx,sy = angle_vector(worm:angle(),5)
							end
							
						--DRAWING THE LIGHTNING, FIRST PART IS FROM START TO FIRST WAYPOINT, THEN LOOP FOR ALL WAYPOINTS, AND FINALLY ONE STAGE FROM LAST WAYPOINT TO THE END
							dx, dy = angle_vector(worm:angle(), decr)
							gfx_set_add(255)
							bitmap:linewu(x+sx,y+sy,x+fx[1],y+fy[1],color(180,190,255))
							for c=2,stages do 
								bitmap:linewu(x+fx[c-1],y+fy[c-1],x+fx[c],y+fy[c],color(180-c*5,190-c*5,255-c*5))
							end
							bitmap:linewu(x+fx[stages],y+fy[stages],x+dx,y+dy,color(180-stages*5,190-stages*5,255-stages*5))
							gfx_set_add(80)

						--MAIN STRAIGHT BEAM
							bitmap:linewu(x+sx,y+sy,x+dx,y+dy,color(180,190,255))
									
						--FADING OUT BLUE FLASH, USES PREMADE SPRITES
							for d = 1,decr/2 do
								gfx_set_add(90-d*2.1/decr*100)
								dx, dy = angle_vector(worm:angle(), d * 2)
								lg_flash:render(bitmap, 0, x + dx, y + dy)
							end
									
							gfx_reset_blending()
						end
		
				--THIS IS EXECUTED WHEN THERE IS ONLY ONE PLAYER, BASICALLY THE SAME CODE WITHOUT CHECKING FOR OTHER WORMS
					else
						while decr > 0 do
							local dx, dy = angle_vector(worm:angle(), decr)
							if map_is_blocked(tx,ty,tx+dx,ty+dy) then
								decr = decr - 5
							else			
								if temp <= decr then
									stages = floor((decr/range) * stages + 1)
								
									for i=1,stages do
										fx[i],fy[i] = angle_vector(worm:angle(),decr/(stages+1)*i)
										fx[i] = fx[i] + randomint(-width,width)
										fy[i] = fy[i] + randomint(-width,width)
										sx,sy = angle_vector(worm:angle(),5)
									end
									
									dx, dy = angle_vector(worm:angle(), decr)
									gfx_set_add(255)
									bitmap:linewu(x+sx,y+sy,x+fx[1],y+fy[1],color(180,190,255))
									
									for c=2,stages do 
										bitmap:linewu(x+fx[c-1],y+fy[c-1],x+fx[c],y+fy[c],color(180-c*5,190-c*5,255-c*5))
									end
									
									bitmap:linewu(x+fx[stages],y+fy[stages],x+dx,y+dy,color(180-stages*5,190-stages*5,255-stages*5))
									gfx_set_add(80)
									bitmap:linewu(x+sx,y+sy,x+dx,y+dy,color(180,190,255))
									
									for d = 1,decr/2 do
										gfx_set_add(90-d*2.1/decr*100)
										dx, dy = angle_vector(worm:angle(), d * 2)
										lg_flash:render(bitmap, 0, x + dx, y + dy)
									end
									
									gfx_reset_blending()
								end
								break
							end
						end
					end					
				end
			end
		end
	end

--RIFLE LASERSIGHT
	function bindings.wormRender(x, y, worm, viewport, ownerPlayer)
		local player = worm:player()
		local ownViewport = ( ownerPlayer == worm:player() )
		local bitmap = viewport:bitmap()
		if ownViewport then
		if worm:current_weapon():type():name() == "Winchester" or worm:current_weapon():type():name() == "Rifle " then
			if worm:current_weapon():is_reloading() == false then
				local range = 650
				local decr = range
				local temp = 0
				local tx,ty = worm:pos()
				
					
			--THE LASERSIGHT ACTUAL LENGTH IS CHECKED JUST LIKE IN LIGHTNING GUN, EXCEPT IT DOESN'T CHECK WORM COLLISIONS
				while decr > 0 do
					local dx, dy = angle_vector(worm:angle(), decr)
						
					if map_is_blocked(tx,ty,tx+dx,ty+dy) then
						decr = decr - 5
					else			
						for i=1,60 do
							temp = randomint(8,range)
							if temp <= decr then
								dx, dy = angle_vector(worm:angle(), temp)
								gfx_set_add(255)
								bitmap:putpixelwu(x+dx,y+dy,color(laserColor.r,laserColor.g,laserColor.b))
								gfx_reset_blending()
							end
						end
							
						break
					end
				end
			end
		end
		end
	end

--FIRECONES

	function weaponeffects.firecone_winchester(worm)
		worm:player():data().draw_firecone = true
	end

	--MAIN RENDER FUNCTION (TURNED OFF BECAUSE RENDERED FIRECONES DON'T LOOK GOOD)
	--[[function bindings.wormRender(x, y, worm, viewport, ownerPlayer)
		local player = worm:player()
		local ownViewport = ( ownerPlayer == worm:player() )
		local bitmap = viewport:bitmap()
		local sx,sy,fx,fy,ex,ey
		local flames_amount = 5
		local start_distance = 4
		local end_distance = 15
		local end_distance2 = 20
		local width = 4
		local random = 0
		
		if player:data().draw_firecone then
			player:data().draw_firecone = false
			sx,sy = angle_vector(worm:angle(),start_distance)
			
			for i = 1,flames_amount do
				random = randomint(-width,width)
				
				fx,fy = angle_vector(worm:angle()+random,end_distance)			
				bitmap:linewu(x+sx,y+sy,x+fx,y+fy,color(255,255,255))
				
				ex,ey = angle_vector(worm:angle()+random,end_distance2)	
				bitmap:linewu(x+fx,y+fy,x+ex,y+ey,color(255,200,105))	
			end
			
		end
		
	end]]--
	
end
