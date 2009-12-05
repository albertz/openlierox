function doom.init()

	common.init({
		hideEnemyHealth = true,
		hideNames = true
	})
	end

print("Loading particle...")

local bouncing = load_particle("bounce.obj")
local bouncing2 = load_particle("bounce2.obj")
local hurt = load_particle("hurt.obj")
local bouncing_shell = load_particle("shell_bounce.obj")
local bouncing_shell_shotgun = load_particle("shell_bounce_shotgun.obj")
local bouncing_meat = load_particle("meat_bounce.obj")

---slimpack
local slimpack = load_particle("slimpack_heal.obj")
local slimpack_speed = load_particle("slimpack_speed.obj")
local slimpack_loop = load_particle("slimpack_loop.obj")

---plasmablade
local plasmablade = load_particle("PlasmaBlade.obj")
local plasmablade_swing = load_particle("PlasmaBlade_Swing.obj")
local plasmablade_block = load_particle("plasmablade_block.obj")
local plasmablade_swing_sound = load_particle("plasmasaber_swing_sound.obj")

---Grenade
local grenade = load_particle("grenade.obj")

---Assault Rifle
local assaultrifle = load_particle("assaultrifle1_bullet.obj")

---BFG blaster
local bfg_big = load_particle("BFG_b.obj")
local bfg = load_particle("BFG.obj")
local bfg_flash = load_particle("flash_bfg.obj")
local bfg_flash_small = load_particle("flash_bfg_small.obj")
local bfg_flash_smallest = load_particle("flash_bfg_smallest.obj")
local bfg_small = load_particle("BFG_s.obj")
local bfg_smallest = load_particle("BFG_st.obj")
local bfg_sound_1 = load_particle("bfg_sound_1.obj")
local bfg_sound_2 = load_particle("bfg_sound_2.obj")
local bfg_sound_3 = load_particle("bfg_sound_3.obj")
local bfg_sound_4 = load_particle("bfg_sound_4.obj")
local bfg_firetrail = load_particle("bfg_firetrail.obj")
local bfg_sound_firestop =  load_particle("bfg_sound_firestop.obj")

---Chaingun
local smoke = load_particle("chaingun_trail.obj")

---Flamethrower
local flames = load_particle("flamethrower_on.obj")
local flame_burner = load_particle("flamethrower_onfire.obj")

---Remote Bomb
local remotebomb = load_particle("remotebomb.obj")
local remotebomb_explode = load_particle("remotebomb_script_bang.obj")	
local remotebomb_select = load_particle("remotebomb_select.obj")
local remotebomb_button = load_particle("remotebomb_button.obj")
local remotebomb_click = load_particle("click.obj")

---IonCannon
local ioncannon1 = load_particle("ioncannon_1.obj")
local ioncannon2 = load_particle("ioncannon_2.obj")
local ioncannonflash = load_particle("flash_ioncannon.obj")
local ion_charge_sound = sounds["ion_charge.ogg"]

	function doom.bouncing(object)
		if vector_distance( object:spd() ) > 0.2 then
			object:shoot(bouncing, 1, 0, 0, 0, 0, 0, 0, 0)
	end
end

	function doom.bouncing_heavy(object)
		if vector_distance( object:spd() ) > 0.2 then
			object:shoot(bouncing2, 1, 0, 0, 0, 0, 0, 0, 0)
	end
end

	function doom.bouncing_shell(object)
		if vector_distance( object:spd() ) > 0.2 then
			object:shoot(bouncing_shell, 1, 0, 0, 0, 0, 0, 0, 0)
	end
end

	function doom.bouncing_meat(object)
		if vector_distance( object:spd() ) > 0.2 then
			object:shoot(bouncing_meat, 1, 0, 0, 0, 0, 0, 0, 0)
	end
end

	function doom.bouncing_shell_shotgun(object)
		if vector_distance( object:spd() ) > 0.2 then
			object:shoot(bouncing_shell_shotgun, 1, 0, 0, 0, 0, 0, 0, 0)
	end
end

	function doom.parts_worm(object)
		if vector_distance( object:spd() ) > 1.8 then
			object:shoot(hurt, 1, 0, 0, 0, 0, 0, 0, 0)
	end
end

----------------------------slimpack---------------------------
	function doom.slimpack_speed_active(worm)
	player = worm:player() 	
	player:data().slimpack_speed = 1
	player:data().slimpack_speed_sound = 1
	player:data().slimpack_speed_posx, player:data().slimpack_speed_posy = player:worm():pos()
	end

function doom.slimpack_speed_reset(worm)
	player = worm:player() 
	player:data().slimpack_speed = nil
	player:data().slimpack_speed_sound = nil
end

	function doom.slimpack_speed(worm)
	player = worm:player() 
	if player:data().slimpack_speed then
		 player:data().slimpack_speed = player:data().slimpack_speed + 1
		 player:data().slimpack_speed_sound = player:data().slimpack_speed_sound + 1
		 if player:is_local() then
		  local newx, newy = player:worm():pos()
		  local deltax = newx - player:data().slimpack_speed_posx
		  local deltay = newy - player:data().slimpack_speed_posy
		  player:worm():set_pos(newx + deltax + deltax , newy + deltay + deltay)
		 player:data().slimpack_speed_posx, player:data().slimpack_speed_posy = player:worm():pos()
		 end
		 if player:is_local()== false then
	        player:data().slimpack_speed = nil
		  player:data().slimpack_speed_sound = nil
		 end
		 if player:data().slimpack_speed % 5 == 0 then
		  worm:shoot(slimpack_speed, 1, -0.1, 0, 0, 0.5, 0, 0, -2)
		 end 	
		 if player:data().slimpack_speed_sound == 55 then
		  worm:shoot(slimpack_loop, 1, 0, 0, 0, 0, 0, 0, 0)
 		  player:data().slimpack_speed_sound = 1
		 end
		 if player:data().slimpack_speed == 1200 then
		  player:data().slimpack_speed = nil
		  player:data().slimpack_speed_sound = nil
		 end
end
		end		

function doom.slimpack(object,target)
		local healthexcess = target:health() - 125
			while healthexcess > 0 do
				healthexcess = healthexcess - 1
					target:shoot(slimpack,1)
	end
end

----------------------------needler----------------------------
function doom.lgrSetTarg(object)
	local x,y = object:pos()
	local player = object:player()
	if player then
		object:player():data().lgrTargX = x
		object:player():data().lgrTargY = y
	end
end

function doom.lgrTurnToTarg(object)
	local player = object:player()
	if player then
		local x1,y1 = object:pos()
		local x2 = player:data().lgrTargX
		local y2 = player:data().lgrTargY

		local a = object:get_angle()

		--object:set_angle(vector_direction(x1, y1, x2, y2))
		if ( angle_diff(a,vector_direction(x1, y1, x2, y2)) > 0 ) then
			object:set_angle(a + 25)
		else
			object:set_angle(a - 25)
		end
	end
end



--------------------------------Plasmablade---------------------------
-------------------------------block--------------------------
print("Loading Plasmablade. . .")	
	function doom.plasmablade_block_trigger(worm)
		worm:player():data().plasmablade_trigger_block = 1
		worm:player():data().plasmablade_aid = 1
	end


	function doom.plasmablade_loop(worm)
		
-------------------------------block--------------------------
		if 	worm:player():data().plasmablade_aid 
		then

		if worm:player():data().plasmablade_trigger_block 
		then
		worm:shoot(plasmablade_block, 1, 1, 0, 0, 0, 0, worm:player():data().plasmablade_trigger_block - 60, 8)
		worm:player():data().plasmablade_trigger_block = worm:player():data().plasmablade_trigger_block + 12
		
		if worm:player():data().plasmablade_trigger_block == 1
		then
		worm:shoot(plasmablade_swing_sound,1)
		end

		if worm:player():data().plasmablade_trigger_block == 360 
		then
		worm:player():data().plasmablade_trigger_block = 1
		end
		end
		
-------------------------------single strike--------------------------
	

if worm:player():data().plasmablade_trigger
		then
		worm:shoot(plasmablade_swing, 1, 1, 0, 0, 0, 0, worm:player():data().plasmablade_trigger - 60, 4)
		worm:player():data().plasmablade_trigger = worm:player():data().plasmablade_trigger + 9
		worm:shoot(plasmablade_swing_sound,1)
		 		 
		if worm:player():data().plasmablade_trigger > 120
		then
		worm:player():data().plasmablade_trigger_aid = worm:player():data().plasmablade_trigger_aid + 1
		end

		if worm:player():data().plasmablade_trigger_aid == 2
		then
		worm:player():data().plasmablade_trigger = worm:player():data().plasmablade_trigger - 18
		end	

		if worm:player():data().plasmablade_trigger < 0
		then 
		worm:player():data().plasmablade_trigger = nil
		worm:player():data().plasmablade_trigger_aid = nil
		worm:player():data().plasmablade_aid = nil
		end
		end
		else
		 worm:shoot(plasmablade, 1, 1, 0, 0, 0, 0, -60, 4)
		end
	end


	function doom.plasmablade_strike_trigger(worm)
		worm:player():data().plasmablade_trigger = 1
		worm:player():data().plasmablade_trigger_aid = 1
		worm:player():data().plasmablade_trigger_block = nil
	end

	function doom.plasmablade_reset(worm)
		worm:player():data().plasmablade_trigger = nil
		worm:player():data().plasmablade_trigger_aid = nil
		worm:player():data().plasmablade_trigger_block = nil
		worm:player():data().plasmablade_trigger_block = nil
		worm:player():data().plasmablade_aid = nil
	end
--------------------------------Plasmablade---------------------------

-----------------------------------Grenade----------------------------
print("Loading Grenade. . .")

function doom.grenade_reset(worm)
worm:player():data().grenade_hold = 1
worm:player():data().grenade_reload = 2
end

function doom.grenade_loop(worm)
if worm:player():data().grenade_hold then
worm:player():data().grenade_hold = worm:player():data().grenade_hold + 0.05
end
if worm:player():data().grenade_hold > 3.5 then
worm:player():data().grenade_hold = worm:player():data().grenade_hold - 0.05
end
end

function doom.grenade_release(worm)
if worm:player():data().grenade_reload then
worm:shoot (grenade,1, worm:player():data().grenade_hold , 1 ,0.1 , 0.5, 0, 10, 0, 6)
worm:player():data().grenade_hold = 1
end
end

function doom.grenade_out_of_ammo(worm)
worm:player():data().grenade_reload = nil
end

function doom.grenade_reload(worm)
worm:player():data().grenade_reload = 1
end
-----------------------------------Grenade----------------------------

---------------------------------AssaultRifle-------------------------
print("Loading Assault Rifle. . .")

function doom.ar_reset(worm)
worm:player():data().ar_fire = 0
worm:player():data().ar_timer = 0
end

function doom.ar_fire_start(worm)
worm:player():data().ar_fire = 1
worm:player():data().ar_timer = 1
end

function doom.ar_fire_loop(worm)
if worm:player():data().ar_fire < 9 then
worm:player():data().ar_fire = worm:player():data().ar_fire + 0.4
worm:shoot (assaultrifle, 1 , 0.9, 0, 0, 0, worm:player():data().ar_fire + 1, 0, 6)
end

if worm:player():data().ar_fire > 9 then
worm:player():data().ar_fire = worm:player():data().ar_fire - 0.4
end
end
---------------------------------AssaultRifle-------------------------

-------------------------------------BFG------------------------------
print("Loading BFG Blaster. . .")

function doom.bfg_reset(worm)
worm:player():data().bfg_fire = 0
worm:player():data().bfg_timer = 0
end

function doom.bfg_fire_start(worm)
worm:player():data().bfg_fire = 1
worm:player():data().bfg_timer = 1
end

function doom.bfg_fire_loop(worm)
if worm:player():data().bfg_fire < 255 and worm:player():data().bfg_timer == 1 then
worm:player():data().bfg_fire = worm:player():data().bfg_fire + 1
end

if worm:player():data().bfg_fire == 255 then
worm:player():data().bfg_timer = 0
worm:player():data().bfg_fire = 300
worm:shoot (bfg_sound_firestop,1,0,0,0,0,0,0,5)
end

--firetrail--
if worm:player():data().bfg_fire > 30 and worm:player():data().bfg_timer == 1 then
worm:shoot (bfg_firetrail,1,0,0,0,0,0,0,5)
end

if worm:player():data().bfg_fire > 60 and worm:player():data().bfg_timer == 1 then
worm:shoot (bfg_firetrail,1,0,0,0,0,0,0,5)
end

if worm:player():data().bfg_fire > 90 and worm:player():data().bfg_timer == 1 then
worm:shoot (bfg_firetrail,1,0,0,0,0,0,0,5)
end

if worm:player():data().bfg_fire > 120 and worm:player():data().bfg_timer == 1 then
worm:shoot (bfg_firetrail,1,0,0,0,0,0,0,5)
end

if worm:player():data().bfg_fire > 150 and worm:player():data().bfg_timer == 1 then
worm:shoot (bfg_firetrail,1,0,0,0,0,0,0,5)
end

if worm:player():data().bfg_fire > 180 and worm:player():data().bfg_timer == 1 then
worm:shoot (bfg_firetrail,1,0,0,0,0,0,0,5)
end

--firetrail--

--sound--
if worm:player():data().bfg_fire == 17 then
worm:shoot (bfg_sound_1,1)
end

if worm:player():data().bfg_fire == 74 then
worm:shoot (bfg_sound_2,1)
end

if worm:player():data().bfg_fire == 132 then
worm:shoot (bfg_sound_3,1)
end

if worm:player():data().bfg_fire == 200 then
worm:shoot (bfg_sound_4,1)
end
end
--sound--

function doom.bfg_fire_end(worm)
if worm:player():data().bfg_fire == 300 then
worm:player():data().bfg_timer = 0
worm:player():data().bfg_fire = 0
end

if worm:player():data().bfg_fire > 200 and worm:player():data().bfg_timer == 1 then
worm:player():data().bfg_timer = 0
worm:player():data().bfg_fire = 0
worm:shoot (bfg_big,1 , 2.5, 0, 0, 0, 0, 0, 5 )
worm:shoot (bfg_flash,1,0,0,0,0,0,0,5)
end

if worm:player():data().bfg_fire < 200 and worm:player():data().bfg_fire > 132 and worm:player():data().bfg_timer == 1 then
worm:player():data().bfg_timer = 0
worm:player():data().bfg_fire = 0
worm:shoot (bfg,1 , 2.5, 0, 0, 0, 0, 0, 5 )
worm:shoot (bfg_flash,1,0,0,0,0,0,0,5)
end

if worm:player():data().bfg_fire < 132 and worm:player():data().bfg_fire > 74 and worm:player():data().bfg_timer == 1  then
worm:player():data().bfg_timer = 0
worm:player():data().bfg_fire = 0
worm:shoot (bfg_small,1 , 2.5, 0, 0, 0, 0, 0, 5 )
worm:shoot (bfg_flash_small,1,0,0,0,0,0,0,5)
end

if worm:player():data().bfg_fire < 74 and worm:player():data().bfg_fire > 2 and worm:player():data().bfg_timer == 1 then
worm:player():data().bfg_timer = 0
worm:player():data().bfg_fire = 0
worm:shoot (bfg_smallest,1 , 2.5, 0, 0, 0, 0, 0, 5 )
worm:shoot (bfg_flash_smallest,1,0,0,0,0,0,0,5)
end
end

-------------------------------------BFG------------------------------

----------------------------------chaingun----------------------------
print("Loading Chaingun. . .")

function doom.chaingun_smoke_reset(worm)
worm:player():data().out = 0
worm:player():data().chaingun = 0
end

function doom.chaingun_smoke_in(worm)
worm:player():data().out = 1
worm:player():data().chaingun = 1
end

function doom.chaingun_smoke_out(worm)
worm:player():data().out = 0
end

function doom.chaingun_smoke_loop(worm)
if worm:player():data().chaingun < 500 and worm:player():data().out == 1 then
worm:player():data().chaingun = worm:player():data().chaingun + 1
end

if worm:player():data().out == 0  and worm:player():data().chaingun > 0 then
worm:player():data().chaingun = worm:player():data().chaingun - 1.5
end

if worm:player():data().chaingun < 500 and worm:player():data().chaingun > 150 and worm:player():data().out == 0 then 
worm:shoot (smoke,1,0,0.05,0,0,10,0,6)
end
end

----------------------------------chaingun----------------------------

--------------------------------Flamethrower--------------------------
print("Loading Flamethrower. . .")

function doom.flame_loop(worm)
if worm:player():data().flame == 1 then 
worm:shoot (flames,1,0.5,0,0,0,30,0,5)
end
end

function doom.flame_reset(worm)
worm:player():data().flame = 1
flamethrowertimer = 0
end

function doom.flame_set(worm)
worm:player():data().flame = 0
end

function doom.burner(worm)
		if flamethrowertimer == 10 then
		worm:shoot(flame_burner,1,1,0,0,0,11,0,5)
		flamethrowertimer = 0
	else
		flamethrowertimer = flamethrowertimer + 1
	end	
end

function doom.burnpos(object,worm)
	wx,wy = worm:pos()
	object:set_pos(wx,wy)
end


--------------------------------Flamethrower--------------------------

---------------------------------RemoteBomb--------------------------
print("Loading RemoteBomb. . .")
function doom.remotebomb_reset(worm)
worm:player():data().remotebomb = 0
worm:player():data().remotebomb_mode = 1
end

function doom.remotebomb_set(worm)
worm:player():data().remotebomb = 1
end

function doom.remotebomb_loop(worm)
if worm:player():data().remotebomb < 20 then
worm:player():data().remotebomb = worm:player():data().remotebomb + 1
end
end

function doom.remotebomb_loping(worm)
if worm:player():data().remotebomb_mode == 2 then
worm:shoot (remotebomb_button,1,0,0,0,0,0,0,0)
end
end

function doom.remotebomb_select(worm)
if worm:player():data().remotebomb > 10 and worm:player():data().remotebomb_mode == 1 then
worm:shoot (remotebomb,1,worm:player():data().remotebomb/20, 0, 0, 0, 0, 0, 6)
end

if worm:player():data().remotebomb > 10 and worm:player():data().remotebomb_mode == 2 then
remotebomb_boom = 1
worm:shoot (remotebomb_click,1)
end

if worm:player():data().remotebomb < 10 then
worm:shoot (remotebomb_select,1)
worm:player():data().remotebomb_mode = worm:player():data().remotebomb_mode + 1
worm:player():data().remotebomb = 0
end

if worm:player():data().remotebomb_mode == 3 then
worm:player():data().remotebomb_mode = 1
remotebomb_boom = nil
end
end

function doom.remotebomb_active (object)
if remotebomb_boom then
object:shoot (remotebomb_explode,1)
remotebomb_boom = nil
end
end

---------------------------------RemoteBomb--------------------------

---------------------------------IonCannon---------------------------
print("Loading Ion Cannon. . .")


function doom.ion_reset(worm)
worm:player():data().ion_fire = 0
worm:player():data().ion_timer = 0
end

function doom.ion_fire_start(worm)
worm:player():data().ion_fire = 1
worm:player():data().ion_timer = 1
end

function doom.ion_fire_loop(worm)
local player = worm:player()
if worm:player():data().ion_fire and worm:player():data().ion_timer == 1 then
worm:player():data().ion_fire = worm:player():data().ion_fire + 1
end
if worm:player():data().ion_fire == 50 then
ion_charge_sound:play(worm, nil, 100, 1, 0)
end
if worm:player():data().ion_fire == 120 then
worm:player():data().ion_timer = 0
worm:player():data().ion_fire = 0
worm:shoot (ioncannon2,1,1,0,0,0,0,0,5)
worm:shoot (ioncannonflash,1,0,0,0,0,0,0,5)
worm:shoot (ioncannonflash,1,0,0,0,0,0,0,5)
worm:shoot (ioncannonflash,1,0,0,0,0,0,0,5)
end
end

function doom.ion_fire_end(worm)
if worm:player():data().ion_fire < 150 and worm:player():data().ion_fire > 5 and worm:player():data().ion_timer == 1 then
worm:player():data().ion_timer = 0
worm:player():data().ion_fire = 0
worm:shoot (ioncannon1,1,1,0,0,0,0,0,5)
worm:shoot (ioncannonflash,1,0,0,0,0,0,0,5)
end
end

---------------------------------IonCannon---------------------------