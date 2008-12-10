




 ---------------        ---         ---------------
            ---  Ze end of ze worms  ---
 ---------------   -    ---    -    ---------------

                    WORM WAR II(TM)

                                Worm War II Mod for Liero Xtreme
                                               by AnubisBlessing


| --------------- Version history ---

0.5 b
 First release for public testing 
 20080626

0.55 b

 20080527
 
 * Completed and enhanced weapon descriptions. Reading should now be more
   plesant.
 * Made some cool layout stuff for this README

 * Increased Tommy gun fire rate and damage (8-10)(150-130)
 * Decreased Tommy gun loading time (2.5-3)
 * Increased Heavy Machinegun fire rate and damage (15-17)(200-170)
 * Increased Strumgewehr damage (11-13)
 * Decreased Mauser fire rate (6-5)

 * Mortar now doesn't load
 * Made incendiary bomb a bit more effective
 * Changed Rocket launcher name

 * Medkit takes longer to load
 * Faster damage delay projectiles
   - Probably worsened the wallshooting, but lessens the effect of the moving
   shooter on the projectiles.
 * More flamer damage and load

0.66 b

 20080528

 * New weapon: Demolition Charge
 
 - Tested online

0.666 b

 * Demolition charge now has and audible and visible timer
 * Demolition charge explosion radius has been made smaller
 * Trace of the blast bits has been made clearer, so learning
   to control the blast is easier

 * Rocketlauncher now has two charges which can be fired
   in a pretty fast sequence
 * Damage delay for barbed wire
 
 * Physics overdone
   - Air and ground movement is faster
   - Rope is a bit weaker
   - Aim is slightly faster
0.77 b
  
  * Added Soviet side

0.78 b
  
  * Scrapped Soviet

0.88 b
  
  20080623

  * Damage delay no longer allows bullets togo through thick walls.
    - They still go through some walls, but it's not nearly as bad.
  * Demolition charge leaves some hot bits behind.
  * The mortar now does more damage.
    - Not quite as strong as in MW, though ;)
  * Rebalanced the machineguns
  * Garamand now has a bigger clip and loads a bit faster.
  * Grenades, mortars, and rocket launcher now have less delay
  * Heavy machinegun also fires into walls at short range now to avoid
    further confusion.
  
  * Physics overdone again. At least I'm happy with them now.
  * Minor tweaks on some projectile physics.
  
  * Added casing sound and smoke.
  * New sounds.

v0.1
  20080624
 
  * Grenade graphics and pin thing
  
  * Grand release!



| --------------- Current issues ---

 Current issues:
 * Damage delay still fire enable firearms to shoot through some walls.
 - Fix wall detection

 * Gas grenade might still cause some lag because of high paricle amount
 - Possible fix: Spawn particles less often, but let them live longer.
 - Keep particles down without damaging aestetics.

| --------------- Todo ---

 * GFX
 - Different casings?

| ---------------        ---
| --------------- Ze mod ---
| ---------------        ---

Theme:
The mod is set in World war two, in a battle between the allied forces and the
axis. Both sides have specific weapons which give them advantages or
disadvantages in different scenarios. The mod is of course also designed to be u
sed in normal FFA games with mixed weapons sets.


Design discussion:

* Standing still while firing?

 You need to be stationary in order to successfully fire the rocketlauncher or
 the heavy machinegun. Why? Because they are really strong, so it's matter of a
 trade-off. The tide of the battle can be completely changed by risking a bit of
 health.

* It's so easy to fire through walls with firearms!

 Damage delay is a technique used to prevent players with higher latency from
 damaging themselves by having the actual damaging projectile spawing a bit ahead
 of the user. This, poorly implemented means that it enables the projectile to
 spawn behind a wall as well. This is being worked upon.

 This because the first projectile, the actual damage delay gets too far before  detonating, even with a very short timer. This can be fixed by either lowering
 the initial speed of the projectile, or add dampening to it.
 The first of the option has the effect that the damaging projectiles speed will
 be more noticably affected by the worms speed, while the other will give
 absurd angles even while stationary, so needless to say, it's a painstaking
 job to find the balance between them. And in my opinion, the damage delay is
 worth it.


Side description:

| --------------- Axis ---
  
 Think blitzkrieg. Hit 'em fast, hard and precise. Most weapons 
 are typical distance weapons with high damage but low fire rate.

 Weapons:

  Mauser 98k
  The most powerful and precise weapon, it boasts a bayonette attack at close
  range. However, you have to load between each shot.
  Damage: 37 (67)
  Clip size: 1

  Sturmgewehr 44
  A powerful assault rifle, which serves best at medium to long range.
  Damage: 12
  Clip size: 20

  Gas grenade (x 2)
  A canister that starts leaking out a liquid upon impact which evaporates into a 
  toxic cloud of gas. You are only equipped with one, so use it wisely

| --------------- Allied forces ---

 Keep your allied close, but your enemies closer. Charge into the enemy and cause
 damage up close.

 Weapons:

  Garamand M1
  A semi automatic rifle, capable of killing an opponent with one clip. Do you
  have the aim, though?
  Damage: 32
  Clip size: 5

  Thompson M1928
  The spammy sammy of the weapons. Causes less damage than the StG and is 
  slightly less accurate. But it sure makes up for it in firing rate.
  Damage: 10
  Clip size: 25

  Mortar launcher (x 3)
  Three long trajectory explosive charges that will explode on impact 
  and produce shrapnel to tear your enemies up.


| --------------- Other equipment ---

 Apart from the side specific weapons, both sides have an arsenal of general use
 equipment in common.

 Firearms:

  Heavy Machinegun
  A heavy, stationary machinegun which with a huge clip and inflicts heavy damage
  with the drawback that you have to be still while using it.
  Damage: 17
  Clip size: 50

 Explosives:
 
  Hand grenade    (x 4)
  Incendiary bomb (x 2)
  Scorch the earth with these death sticks. Detonating on impact you will see
  you enemies engulfed in flames in no time.

  Rocket launcher (x 2)
  Rocketlaunchers were originally designed to destroy tanks and stuff, but
  that just makes it more fun to use on poor worms. You are only equipped with
  two shells per life, and you have to stand still when using it or you waste it.

  Demolition charge
  The mods most powerful explosive, capable of wiping out anything visible on the
  screen. However, the charge makes a clear noise to warn your enemies and has a
  10 second timer, so place it stealthily or chase your enemy into the trap.
  Keep your eyes on how the white smoke is shaped after the explosion and
  you might learn how to control direction of the blast. Do you know your
  binary digits? Then take a look at the timer. Oh, and be careful not to run into
  the bits not detonated in the first blast!

 Other:
   
  Flamethrower
  When is the wrong time for barbecue? That's right! Never! So bring your
  incinerator buddy on with you out on the battle field and spend some quality
  time with your friends.
  
  Barbed wire
  Tuck your enemies in in a cozy blanket of barbed wire, or why not attach the
  roll itself to their faces? The Barbed wire roll is the most damaging
  projectile in the mod.
  
  Sandbags
  Provides perfect protection from firearms, and can save your life from a nearby
  grenade if deployed in time.

  Medkit
  Heal a teammate. Or an enemy worm for that matter!
 

| --------------- Thanks ---
  
  You, for reading this README
  You for testing my mod

  Cizin (RIP ;P) for testing and feedback
  Alexis and Alexis for testing and testing and feedback and feedback
  Asrack for wanting to test it.
  Gyoghurt and all you other guys that night a month ago :P

  Rye for LMS
  JB, DC and all you other guys making OLX possible
  
  Everyone who has given feedback on my work.
  Anyone who made graphics or soundeffects used in this mod.

| --------------- Resources ---

  Graphics:
    Liero 1.11 graphics used in:
    Casing, Fire and smoke resources
   
    The rest are my making or included in Classic. w00t.
  
  Sound:
    MSF gun sounds used in betas up until 0.77
    Thanks sounddog.com for the rest of them.

  
  Any material may be used either with my permission or
  the real author being mentioned in the mod documentation.

  In case of future need of updating, I'd be glad to either 
  take any requests for it or, if deemed necessary, hand
  over the source.

 
| --------------- Contact ---

 anubisblessing@hotmail.com

 OR

 http:\\www.lxalliance.net\

 PM AnubisBlessing

 LXA thread coming up

 LXA wiki article coming up
 

| --------------- Legal info ---

 I'm not a lawyer.
 
 Distributed by your local internets.
 All trademarks and logos might be protected. Made in EU 
 2008 (C) Marco Juntunen
