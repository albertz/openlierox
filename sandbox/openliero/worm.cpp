#include "worm.hpp"
#include "game.hpp"
#include "sfx.hpp"
#include "gfx.hpp"
#include "viewport.hpp"
#include "constants.hpp"
#include "console.hpp"

#include <iostream>

struct Point
{
	int x, y;
};

void Worm::calculateReactionForce(int newX, int newY, int dir)
{
	static Point const colPoints[4][7] =
	{
		{ //DOWN reaction points
			{-1, -4},
			{ 0, -4},
			{ 1, -4},
			{ 0,  0},
			{ 0,  0},
			{ 0,  0},
			{ 0,  0}
		},
		{ //LEFT reaction points
			{1, -3},
			{1, -2},
			{1, -1},
			{1,  0},
			{1,  1},
			{1,  2},
			{1,  3}
		},
		{ //UP reaction points
			{-1, 4},
			{ 0, 4},
			{ 1, 4},
			{ 0, 0},
			{ 0, 0},
			{ 0, 0},
			{ 0, 0}
		},
		{ //RIGHT reaction points
			{-1, -3},
			{-1, -2},
			{-1, -1},
			{-1,  0},
			{-1,  1},
			{-1,  2},
			{-1,  3}
		}
		
	};

	static int const colPointCount[4] =
	{
		3,
		7,
		3,
		7
	};

	reacts[dir] = 0;
	
	// newX should be x + velX at the first call
	
	for(int i = 0; i < colPointCount[dir]; ++i)
	{
		int colX = newX + colPoints[dir][i].x;
		int colY = newY + colPoints[dir][i].y;
		
#if 0
		if(!game.level.inside(colX, colY) // TODO: Liero seems to not make any check here, checking garbage pixels
		|| !game.materials[game.level.pixel(colX, colY)].background())
#else
		// This should emulate Liero better
		PalIdx pix = game.level.checkedPixelWrap(colX, colY);
		if(!game.materials[pix].background())
#endif
		{
			++reacts[dir];
		}
	}
}

void Worm::processPhysics()
{
	if(reacts[RFUp] > 0)
	{
		velX = (velX * C[WormFricMult]) / C[WormFricDiv];
	}
	
	if(velX > 0)
	{
		if(reacts[RFLeft] > 0)
		{
			if(velX > C[MinBounceRight])
			{
				if(H[HFallDamage])
					health -= C[FallDamageRight];
				else
					sfx.play(14, 14);
				velX = -velX / 3;
			}
			else
				velX = 0;
		}
	}
	else if(velX < 0)
	{
		if(reacts[RFRight])
		{
			if(velX < C[MinBounceLeft])
			{
				if(H[HFallDamage])
					health -= C[FallDamageLeft];
				else
					sfx.play(14, 14);
				velX = -velX / 3;
			}
			else
				velX = 0;
		}
	}
	
	if(velY > 0)
	{
		if(reacts[RFUp] > 0)
		{
			if(velY > C[MinBounceDown])
			{
				if(H[HFallDamage])
					health -= C[FallDamageDown];
				else
					sfx.play(14, 14);
				velY = -velY / 3;
			}
			else
				velY = 0;
		}
	}
	else if(velY < 0)
	{
		if(reacts[RFDown])
		{
			if(velY < C[MinBounceUp])
			{
				if(H[HFallDamage])
					health -= C[FallDamageUp];
				else
					sfx.play(14, 14);
				velY = -velY / 3;
			}
			else
				velY = 0;
		}
	}
	
	if(reacts[RFUp] == 0)
	{
		velY += C[WormGravity];
	}
	
	if(velX >= 0)
	{
		if(reacts[RFLeft] < 2)
			x += velX;
	}
	else
	{
		if(reacts[RFRight] < 2)
			x += velX;
	}
	
	if(velY >= 0)
	{
		if(reacts[RFUp] < 2)
			y += velY;
	}
	else
	{
		if(reacts[RFDown] < 2)
			y += velY;
	}
}

void Worm::process()
{
	if(health > settings.health)
		health = settings.health;
	
	if(game.settings.gameMode != Settings::GMKillEmAll
	|| lives > 0)
	{
		if(visible)
		{
			// Liero.exe: 291C
			
			fixed nextX = x + velX;
			fixed nextY = y + velY;
			
			int iNextX = ftoi(nextX);
			int iNextY = ftoi(nextY);
			
			{ // Calculate reaction forces

				for(int i = 0; i < 4; i++)
				{
					calculateReactionForce(iNextX, iNextY, i);
					
					// Yes, Liero does this in every iteration. Keep it this way.
					
					
					if(iNextX < 4)
					{
						reacts[RFRight] += 5;
					}
					else if(iNextX > game.level.width - 5)
					{
						reacts[RFLeft] += 5;
					}

					if(iNextY < 5)
					{
						reacts[RFDown] += 5;
					}
					else
					{
						if(H[HWormFloat])
						{
							if(iNextY > C[WormFloatLevel])
								velY -= C[WormFloatPower];
						}
						else if(iNextY > game.level.height - 6)
						{
							reacts[RFUp] += 5;
						}
					}
				}

				if(reacts[RFDown] < 2)
				{
					if(reacts[RFUp] > 0)
					{
						if(reacts[RFLeft] > 0 || reacts[RFRight] > 0)
						{
							//Low or none push down,
							//Push up and
							//Push left or right

							y -= itof(1);
							nextY = y + velY;
							iNextY = ftoi(nextY);

							calculateReactionForce(iNextX, iNextY, RFLeft);
							calculateReactionForce(iNextX, iNextY, RFRight);
						}
					}
				}

				if(reacts[RFUp] < 2)
				{
					if(reacts[RFDown] > 0)
					{
						if(reacts[RFLeft] > 0 || reacts[RFRight] > 0)
						{
							//Low or none push up,
							//Push down and
							//Push left or right

							y += itof(1);
							nextY = y + velY;
							iNextY = ftoi(nextY);

							calculateReactionForce(iNextX, iNextY, RFLeft);
							calculateReactionForce(iNextX, iNextY, RFRight);
						}
					}
				}
			}
			
			int ix = ftoi(x);
			int iy = ftoi(y);

			for(Game::BonusList::iterator i = game.bonuses.begin(); i != game.bonuses.end(); ++i)
			{				
				if(ix + 5 > ftoi(i->x)
				&& ix - 5 < ftoi(i->x)
				&& iy + 5 > ftoi(i->y)
				&& iy - 5 < ftoi(i->y))
				{
					if(i->frame == 1)
					{
						if(health < settings.health)
						{
							game.bonuses.free(i);
							health += (game.rand(C[BonusHealthVar]) + C[BonusMinHealth]) * settings.health / 100; // TODO: Read from EXE
							if(health > settings.health)
								health = settings.health;
						}
					}
					else if(i->frame == 0)
					{
						if(game.rand(C[BonusExplodeRisk]) > 1) // TODO: Read from EXE
						{
							WormWeapon& ww = weapons[currentWeapon];
							
							if(!H[HBonusReloadOnly])
							{
								fireConeActive = false;
								fireCone = -1;
								
								ww.id = i->weapon;
								ww.ammo = game.weapons[ww.id].ammo;
							}
							
							sfx.play(24, 24);
							
							game.bonuses.free(i);
							
							ww.available = true;
							ww.loadingLeft = 0;
						}
						else
						{
							int bix = ftoi(i->x);
							int biy = ftoi(i->y);
							game.bonuses.free(i);
							game.sobjectTypes[0].create(bix, biy, this->index);
						}
					}
				}
			}

			processSteerables();
			
			if(!movable && !gfx.testKey(keyLeft()) && !gfx.testKey(keyRight())) // processSteerables sets movable to false, does this interfer?
			{
				movable = true;
			} // 2FB1
			
			processAiming();
			processTasks();
			processWeapons();
			
			if(gfx.testKey(keyFire()) && !gfx.testKey(keyChange())
			&& weapons[currentWeapon].available
			&& weapons[currentWeapon].delayLeft <= 0)
			{
				fire();
			}
			else
			{
				if(game.weapons[weapons[currentWeapon].id].loopSound)
					sfx.stop(game.weapons[weapons[currentWeapon].id].launchSound);
			}

			processPhysics();
			processSight();
			
			if(gfx.testKey(keyChange()))
			{
				 processWeaponChange();
			}
			else
			{
				keyChangePressed = false;
				processMovement();
			}


			if(health < settings.health / 4)
			{
				if(game.rand(health + 6) == 0)
				{
					if(game.rand(3) == 0)
					{
						if(!sfx.isPlaying(wormSoundID))
						{
							sfx.play(18 + game.rand(3), wormSoundID);
						}
					}
					
					game.nobjectTypes[6].create1(velX, velY, x, y, 0, this->index);
				}
			}
			
			if(health <= 0)
			{
				/* TODO
				//Kill him!
				if(worm->flag != 0)
				{
					//He got the flag!
					long flag;
					if(w == 0)
						flag = 21;
					else
						flag = 20;
    
					//Create the flag
					CreateObject1(
						worm->m_fXVel,
						worm->m_fYVel,
						worm->m_fX,
						worm->m_fY,
						0,
						flag,
    BYTE(w)
					);
					worm->flag = 0;
				} // 468D
				*/

				if(this->index == game.lastKilled)
				{
					game.gotChanged = false;
				}
				else
				{
					game.gotChanged = true;
				}
				
				leaveShellTimer = 0;
				makeSightGreen = false;
				// TODO: cGame::cWorm[w^1].makesightgreen = 0;
				viewport.bannerY = -8;
				
				Weapon& w = game.weapons[weapons[currentWeapon].id];
				if(w.loopSound)
				{
					sfx.stop(w.launchSound);
				}
				
				sfx.play(16 + game.rand(3), wormSoundID);
				
				fireConeActive = 0;
				ninjarope.out = false;
				--lives;
				game.lastKilled = this->index;
				
				if(lastKilledBy && lastKilledBy != this->index)
				{
					++ game.worms[lastKilledBy].kills;
				}
				
				visible = false;
				killedTimer = 150;
				
				int max = 120 * game.settings.blood / 100;
				
				if(max > 1)
				{
					for(int i = 1; i <= max; ++i)
					{
						game.nobjectTypes[6].create2(
							game.rand(128),
							velX / 3, velY / 3,
							x, y,
							0,
							this->index);
					}
				}
				
#if 1
				for(int i = 7; i <= 105; i += 14)
				{
					game.nobjectTypes[index].create2(
							i + game.rand(14),
							velX / 3, velY / 3,
							x, y,
							0,
							this->index);
				}
#endif
				/* TODO
				max = (120 * settings.blood) / 100;

				long c;
				if(max > 1)
				{
					for(c = 1; c <= max; c++)
					{
						//Blood mayhem!
						// ----- Changed when importing to OLX -----
						CreateObject2(game.rand(128), worm->m_fXVel/3, worm->m_fYVel/3, worm->m_fX, worm->m_fY, 0, 6, BYTE(w));
						// ----- Changed when importing to OLX -----
					} // 47E9
				}

				for(c = 7; c <= 105; c += 14)
				{
					// ----- Changed when importing to OLX -----
					CreateObject2(c + game.rand(14), worm->m_fXVel/3, worm->m_fYVel/3, worm->m_fX, worm->m_fY, 0, w, BYTE(w));
					// ----- Changed when importing to OLX -----
				} // 485D
				*/

				gfx.releaseKey(keyFire());				
			}
		}
		else
		{
			// Worm is dead
			// Press any key to respawn - if it will be Fire key the worm will fire immediately,
			// because testKeyOnce() works not as expected in OLX mods.
			if(gfx.testKeyOnce(keyFire()) || gfx.testKeyOnce(keyJump()) || gfx.testKeyOnce(keyChange()))
			{
				ready = true;
			}
			
			if(killedTimer > 0)
				--killedTimer;
				
			if(killedTimer == 0)
				beginRespawn();
				
			if(killedTimer < 0)
				doRespawning();
		}
	}
	
	if(settings.controller == 1)
		processLieroAI();
}

int sqrVectorLength(int x, int y)
{
	return x*x + y*y;
}

void Worm::processLieroAI()
{
	Worm* target = 0;
	int minLen = 0;
	for(std::size_t i = 0; i < game.worms.size(); ++i)
	{
		Worm* w = & game.worms[i];
		if(w != this)
		{
			int len = sqrVectorLength(ftoi(x) - ftoi(w->x), ftoi(y) - ftoi(w->y));
			if(!target || len < minLen) // First or closer worm
			{
				target = w;
				minLen = len;
			}
		}
	}
	
	int maxDist;
	
	WormWeapon& ww = weapons[currentWeapon];
	Weapon& w = game.weapons[ww.id];
	
	if(w.timeToExplo > 0 && w.timeToExplo < 500)
	{
		maxDist = (w.timeToExplo - w.timeToExploV / 2) * w.speed / 130;
	}
	else
	{
		maxDist = w.speed - w.gravity / 10;
	} // 4D43
	
	if(maxDist < 90)
		maxDist = 90;
		
	fixed deltaX = target->x - x;
	fixed deltaY = target->y - y;
	int ideltaX = ftoi(deltaX);
	int ideltaY = ftoi(deltaY);
		
	int realDist = vectorLength(ideltaX, ideltaY);
	
	if(realDist < maxDist || !visible)
	{
		// The other worm is close enough
		bool fire = gfx.testKey(keyFire());
		if(game.rand(game.aiParams.k[fire][WormSettings::Fire]) == 0)
		{
			gfx.setKey(keyFire(), !fire);
		} // 4DE7
	}
	else if(visible)
	{
		gfx.releaseKey(keyFire());
	} // 4DFA
		
	// In Liero this is a loop with two iterations, that's better maybe
	bool jump = gfx.testKey(keyJump());
	if(game.rand(game.aiParams.k[jump][WormSettings::Jump]) == 0)
	{
		gfx.toggleKey(keyJump());
	}
	
	bool change = gfx.testKey(keyChange());
	if(game.rand(game.aiParams.k[change][WormSettings::Change]) == 0)
	{
		gfx.toggleKey(keyChange());
	}

//l_4E6B:
	// Moves up
   
// l_4EE5:
	if(realDist > 0)
	{
		deltaX /= realDist;
		deltaY /= realDist;
	}
	else
	{
		deltaX = 0;
		deltaY = 0;
	} // 4F2F
	
	int dir = 1;
	
	for(; dir < 128; ++dir)
	{
		if(std::abs(cosTable[dir] - deltaX) < 0xC00
		&& std::abs(sinTable[dir] - deltaY) < 0xC00) // The original had 0xC000, which is wrong
			break;
	} // 4F93
	
	fixed adeltaX = std::abs(deltaX);
	fixed adeltaY = std::abs(deltaY);

	if(dir >= 128)
	{
		if(deltaX > 0)
		{
			if(deltaY < 0)
			{
				if(adeltaY > adeltaX)
					dir = 64 + game.rand(16);
				else if(adeltaX > adeltaY)
					dir = 80 + game.rand(16);
				else
					dir = 80;
			}
			else // deltaY >= 0
			{
				if(adeltaX > adeltaY)
					dir = 96 + game.rand(16);
				else
					dir = 116;
			}
		}
		else
		{
			if(deltaY < 0)
			{
				
				if(adeltaY > adeltaX)
					dir = 48 + game.rand(16);
				else if(adeltaX > adeltaY)
					dir = 32 + game.rand(16);
				else
					dir = 48; // This was 56, but that seems wrong
			}
			else // deltaX <= 0 && deltaY >= 0
			{
				if(adeltaX > adeltaY)
					dir = 12 + game.rand(16);
				else
					dir = 12;
			}
		}
	} // 50FD
   
  
/* TODO (maybe)
   if(realdist < maxdist)
   {
    if(dir < 64)
    {
 l_510E:
     //What the hell is wrong with this code?
     //It is messed up totaly! Translating the correct code
     //NOTE! Something has to be done here!
     dir += ax; //What the hell is AX?
     if(dir > 64)
     {
      dir = 64;
     }
    } // 5167
    if(dir > 64)
    {
     //The same thing with this code! Is it encrypted or what?
     dir -= ax; //Again
     if(dir < 64)
     {
      dir = 64;
     }
    }
   } // 51C6
*/

	change = gfx.testKey(keyChange());
	
	if(change)
	{
		if(game.rand(game.aiParams.k[gfx.testKey(keyLeft())][WormSettings::Left]) == 0)
		{
			gfx.toggleKey(keyLeft());
		}
		
		if(game.rand(game.aiParams.k[gfx.testKey(keyRight())][WormSettings::Right]) == 0)
		{
			gfx.toggleKey(keyRight());
		}
		
		if(ninjarope.out && ninjarope.attached)
		{
// l_525F:
			bool up = gfx.testKey(keyUp());
			
			if(game.rand(game.aiParams.k[up][WormSettings::Up]) == 0)
			{
				gfx.toggleKey(keyUp());
			}
			
			bool down = gfx.testKey(keyDown());
			if(game.rand(game.aiParams.k[down][WormSettings::Down]) == 0)
			{
				gfx.toggleKey(keyDown());
			}
		}
		else
		{
// l_52D2:
			gfx.releaseKey(keyUp());
			gfx.releaseKey(keyDown());
		} // 52F8
	} // if(change)
	else
	{
	
		if(realDist > maxDist)
		{
			gfx.setKey(keyRight(), (deltaX > 0));
			gfx.setKey(keyLeft(), (deltaX <= 0));
		} // 5347
		else
		{
			gfx.releaseKey(keyRight());
			gfx.releaseKey(keyLeft());
		}

		if(direction != 0)
		{
			if(dir < 64)
				gfx.pressKey(keyLeft());
			// 5369
			gfx.setKey(keyUp(),   (dir + 1 < ftoi(aimingAngle)));
			// 5379
			gfx.setKey(keyDown(), (dir - 1 > ftoi(aimingAngle)));
		}
		else
		{
			if(dir > 64)
				gfx.pressKey(keyRight());
			// 53C6
			gfx.setKey(keyUp(),   (dir - 1 > ftoi(aimingAngle)));
			// 53E8
			gfx.setKey(keyDown(), (dir + 1 < ftoi(aimingAngle)));
			// 540A
		}
		
		if(gfx.testKey(keyLeft())
		&& reacts[RFRight])
		{
			if(reacts[RFDown] > 0)
				gfx.pressKey(keyRight());
			else
				gfx.pressKey(keyJump());
		} // 5454
		
		if(gfx.testKey(keyRight())
		&& reacts[RFLeft])
		{
			if(reacts[RFDown] > 0)
				gfx.pressKey(keyLeft());
			else
				gfx.pressKey(keyJump());
		} // 549E
	}
}

void Worm::beginRespawn()
{
	int tempX = ftoi(x);
	int tempY = ftoi(y);
	
	int enemyX = tempX;
	int enemyY = tempY;
	
	if(game.worms.size() == 2)
	{
		enemyX = ftoi(game.worms[index ^ 1].x);
		enemyY = ftoi(game.worms[index ^ 1].y);
	}

	int trials = 0;
	do
	{
		x = itof(C[WormSpawnRectX] + game.rand(C[WormSpawnRectW]));
		y = itof(C[WormSpawnRectY] + game.rand(C[WormSpawnRectH]));

		// The original didn't have + 4 in both, which seems
		// to be done in the exe and makes sense.
		while(ftoi(y) + 4 < game.level.height
		&& game.materials[game.level.pixel(ftoi(x), ftoi(y) + 4)].background())
		{
			y += itof(1);
		}
		
		if(++trials >= 50000)
		{
			Console::writeWarning("Couldn't find a suitable spawn position in time");
			break;
		}
	}
	while(!checkRespawnPosition(enemyX, enemyY, tempX, tempY, ftoi(x), ftoi(y)));
			
	killedTimer = -1;
}

void Worm::doRespawning()
{
	int destX = ftoi(x) - viewport.centerX;
	if( destX < 0 )
		destX = 0;
	if( destX > viewport.maxX )
		destX = viewport.maxX;
		
	int destY = ftoi(y) - viewport.centerY;
	if( destY < 0 )
		destY = 0;
	if( destY > viewport.maxY )
		destY = viewport.maxY;

	if(viewport.x < destX + 5
	&& viewport.x > destX - 5
	&& viewport.y < destY + 5
	&& viewport.y > destY - 5
	&& ready)
	{
		int ix = ftoi(x), iy = ftoi(y);
		drawDirtEffect(0, ix - 7, iy - 7);
		correctShadow(Rect(ix - 10, iy - 10, ix + 11, iy + 11));
		
		ready = false;
		sfx.play(21, 21);
		
		visible = true;
		fireConeActive = 0;
		velX = 0;
		velY = 0;
		health = settings.health;
		
		// NOTE: This was done at death before, but doing it here seems to make more sense
		if(game.rand() & 1)
		{
			aimingAngle = itof(32);
			direction = 0;
		}
		else
		{
			aimingAngle = itof(96);
			direction = 1;
		}
	}
}

void Worm::processWeapons()
{
	for(int i = 0; i < game.settings.selectableWeapons; ++i)
	{
		if(weapons[i].delayLeft >= 0)
			--weapons[i].delayLeft;
	}
	
	WormWeapon& ww = weapons[currentWeapon];
	Weapon& w = game.weapons[ww.id];
	
	if(ww.ammo <= 0)
	{
		ww.available = false;
		ww.loadingLeft = w.computedLoadingTime;
		ww.ammo = w.ammo;
	}
	
	if(ww.loadingLeft > 0) // NOTE: computedLoadingTime is never 0, so this works
	{
		--ww.loadingLeft;
		if(ww.loadingLeft <= 0)
		{
			if(w.playReloadSound)
				sfx.play(24, 24);
				
			ww.available = true;
		}
	}
	
	if(fireCone >= 0)
	{
		--fireCone;
		if(fireCone == 0)
			fireConeActive = false;
	}
	
	if(leaveShellTimer > 0)
	{
		if(--leaveShellTimer <= 0)
		{
			game.nobjectTypes[7].create1(game.rand(16000) - 8000, -int(game.rand(20000)), x, y, 0, this->index);
		}
	}
}

void Worm::processMovement()
{
	if(movable)
	{
		bool left = gfx.testKey(keyLeft());
		bool right = gfx.testKey(keyRight());
		
		if(left && !right)
		{
			if(velX > C[MaxVelLeft])
				velX -= C[WalkVelLeft];
				
			if(direction != 0)
			{
				aimingSpeed = 0;
				if(aimingAngle >= itof(64))
					aimingAngle = itof(128) - aimingAngle;
				direction = 0;
			}
			
			animate = true;
		}
		
		if(!left && right)
		{
			if(velX < C[MaxVelRight])
				velX += C[WalkVelRight];
				
			if(direction != 1)
			{
				aimingSpeed = 0;
				if(aimingAngle <= itof(64))
					aimingAngle = itof(128) - aimingAngle;
				direction = 1;
			}
			
			animate = true;
		}
		
		if(left && right)
		{
			if(ableToDig)
			{
				ableToDig = false;
				
				fixed dirX = cosTable[ftoi(aimingAngle)];
				fixed dirY = sinTable[ftoi(aimingAngle)];
				
				fixed posX = dirX * 2 + x;
				fixed posY = dirY * 2 + y;

				/* TODO
				long iDigx = ftoi(fTempx) - 4;
				if(iDigx < 0)    iDigx = 0;
				if(iDigx >= levwidth) iDigx = levwidth-1;

				long iDigenx = ftoi(fTempx) + 4;
				if(iDigenx < 0)    iDigenx = 0;
				if(iDigenx >= levwidth) iDigenx = levwidth-1;

				long iDigy;

				long iDigsty = ftoi(fTempy) - 4;
				if(iDigsty < 0)    iDigsty = 0;
				if(iDigsty >= levheight) iDigsty = levheight-1;

				long iDigeny = ftoi(fTempy) + 4;
				if(iDigeny < 0)    iDigeny = 0;
				if(iDigeny >= levheight) iDigeny = levheight-1;

				for(; iDigx <= iDigenx; iDigx++)
				{
					for(iDigy = iDigsty; iDigy <= iDigeny; iDigy++)
					{
						// ----- Changed when importing to OLX -----
						//Throw away every third pixel
						if(materials.Dirt[lev(iDigx, iDigy)] && game.rand(3) == 0)
						{
							CreateObject2(game.rand(128), 0, 0, itof(iDigx), itof(iDigy), lev(iDigx, iDigy), 2, BYTE(w));
						} // 419A
						// ----- Changed when importing to OLX -----
					} // 41A9
				} // 41BB
*/

				posX -= itof(7);
				posY -= itof(7);
				
				int ix = ftoi(posX), iy = ftoi(posY);
				drawDirtEffect(7, ix, iy);
				correctShadow(Rect(ix - 3, iy - 3, ix + 18, iy + 18));
				
				posX += dirX << 1;
				posY += dirY << 1;

//l_43EB:
				ix = ftoi(posX);
				iy = ftoi(posY);
				drawDirtEffect(7, ix, iy);
				correctShadow(Rect(ix - 3, iy - 3, ix + 18, iy + 18));
				
				//NOTE! Maybe the shadow corrections can be joined into one? Mmm?
			} // 4552
		}
		else
		{
			ableToDig = true;
		}
		
		if(!left && !right)
		{
			animate = false; //Don't animate the this unless he is moving
		} // 458C
	}
}

void Worm::processTasks()
{
	if(gfx.testKey(keyChange()))
	{
		if(ninjarope.out)
		{
			if(gfx.testKey(keyUp()))
				ninjarope.length -= C[NRPullVel]; 
			if(gfx.testKey(keyDown()))
				ninjarope.length += C[NRReleaseVel];
				
			if(ninjarope.length < C[NRMinLength])
				ninjarope.length = C[NRMinLength];
			if(ninjarope.length > C[NRMaxLength])
				ninjarope.length = C[NRMaxLength];
		}
		
		if(gfx.testKeyOnce(keyJump()))
		{
			ninjarope.out = true;
			ninjarope.attached = false;
			
			sfx.play(5, 5);
			
			ninjarope.x = x;
			ninjarope.y = y;
			
			ninjarope.velX = cosTable[ftoi(aimingAngle)] << C[NRThrowVelX];
			ninjarope.velY = sinTable[ftoi(aimingAngle)] << C[NRThrowVelY];
									
			ninjarope.length = C[NRInitialLength];
		}
	}
	else
	{
		//Jump = remove ninjarope, jump
		if(gfx.testKey(keyJump()))
		{
			ninjarope.out = false;
			ninjarope.attached = false;
			
			if(reacts[RFUp] > 0 && ableToJump)
			{
				velY -= C[JumpForce];
				ableToJump = false;
			}
		}
		else
			ableToJump = true;
	}
}

void Worm::processAiming()
{
	bool up = gfx.testKey(keyUp());
	bool down = gfx.testKey(keyDown());
	
	if(aimingSpeed != 0)
	{
		aimingAngle += aimingSpeed;
				
		if(!up && !down)
		{
			aimingSpeed = (aimingSpeed * C[AimFricMult]) / C[AimFricDiv];
		}
		
		if(direction == 1)
		{
			if(ftoi(aimingAngle) > C[AimMaxRight])
			{
				aimingSpeed = 0;
				aimingAngle = itof(C[AimMaxRight]);
			}
			if(ftoi(aimingAngle) < C[AimMinRight])
			{
				aimingSpeed = 0;
				aimingAngle = itof(C[AimMinRight]);
			}
		}
		else
		{
			if(ftoi(aimingAngle) < C[AimMaxLeft])
			{
				aimingSpeed = 0;
				aimingAngle = itof(C[AimMaxLeft]);
			}
			if(ftoi(aimingAngle) > C[AimMinLeft])
			{
				aimingSpeed = 0;
				aimingAngle = itof(C[AimMinLeft]);
			}
		}
	}
	
	if(movable && (!ninjarope.out || !gfx.testKey(keyChange())))
	{
		if(up)
		{
			if(direction == 0)
			{
				if(aimingSpeed < C[MaxAimVelLeft])
					aimingSpeed += C[AimAccLeft];
			}
			else
			{
				if(aimingSpeed > C[MaxAimVelRight])
					aimingSpeed -= C[AimAccRight];
			}
		}
		
		if(down)
		{
			if(direction == 1)
			{
				if(aimingSpeed < C[MaxAimVelLeft])
					aimingSpeed += C[AimAccLeft];
			}
			else
			{
				if(aimingSpeed > C[MaxAimVelRight])
					aimingSpeed -= C[AimAccRight];
			}
		}
	}
}

void Worm::processWeaponChange()
{
	if(!keyChangePressed)
	{
		gfx.releaseKey(keyLeft());
		gfx.releaseKey(keyRight());
		
		keyChangePressed = true;
	}
	
	fireConeActive = 0;
	animate = false;
	
	if(game.weapons[weapons[currentWeapon].id].loopSound)
	{
		sfx.stop(game.weapons[weapons[currentWeapon].id].launchSound);
	}
	
	if(weapons[currentWeapon].available || game.settings.loadChange)
	{
		if(gfx.testKeyOnce(keyLeft()))
		{
			if(--currentWeapon < 0)
				currentWeapon = game.settings.selectableWeapons - 1;
				
			hotspotX = ftoi(x);
			hotspotY = ftoi(y);
		}
		
		if(gfx.testKeyOnce(keyRight()))
		{
			if(++currentWeapon >= game.settings.selectableWeapons)
				currentWeapon = 0;
				
			hotspotX = ftoi(x);
			hotspotY = ftoi(y);
		}
	}
}

void Worm::fire()
{
	WormWeapon& ww = weapons[currentWeapon];
	Weapon& w = game.weapons[ww.id];
	
	--ww.ammo;
	ww.delayLeft = w.delay;
	
	fireCone = w.fireCone;
	if(fireCone)
		fireConeActive = true; // TODO: Consider removing fireConeActive since fireCone seems to imply it's state
		
	fixed firingX = cosTable[ftoi(aimingAngle)] * (w.detectDistance + 5) + x;
	fixed firingY = sinTable[ftoi(aimingAngle)] * (w.detectDistance + 5) + y - itof(1);
	
	if(w.leaveShells > 0)
	{
		if(game.rand(w.leaveShells) == 0)
		{
			leaveShellTimer = w.leaveShellDelay;
		}
	}
	
	if(w.launchSound >= 0)
	{
		if(w.loopSound)
		{
			if(!sfx.isPlaying(w.launchSound))
			{
				sfx.play(w.launchSound, w.launchSound, -1);
			}
			/* TODO
			if(FSOUND_IsPlaying(weapsettings.launchsound[this->weapons[this->currentweapon].id]))
			{
				playsound(
					weapsettings.loopsound[this->weapons[this->currentweapon].id],
					weapsettings.launchsound[this->weapons[this->currentweapon].id],
					soundpointers[weapsettings.launchsound[this->weapons[this->currentweapon].id]]
				);
			}
			*/
		}
		else
		{
			sfx.play(w.launchSound, w.launchSound);
		}
	}
		
	if(w.affectByWorm)
	{
		int speed = w.speed;
		if(speed < 100)
			speed = 100;
		int parts = w.parts;
		
		if(parts > 0)
		{
			fixed firingVelX = velX * 100 / speed;
			fixed firingVelY = velY * 100 / speed;
			
			for(int i = 0; i < parts; ++i)
			{
				w.fire(
					ftoi(aimingAngle),
					firingVelX,
					firingVelY,
					speed,
					firingX,
					firingY,
					this->index);
			}
		}
	}
	else
	{
		int parts = w.parts;
		
		if(parts > 0)
		{
			for(int i = 0; i < parts; ++i)
			{
				w.fire(
					ftoi(aimingAngle),
					0,
					0,
					w.speed,
					firingX,
					firingY,
					this->index);
			}
		}
	}
	
	velX -= (cosTable[ftoi(aimingAngle)] * w.recoil) / 100; // TODO: Check for signed recoil hack
	velY -= (sinTable[ftoi(aimingAngle)] * w.recoil) / 100;
}

bool checkForWormHit(int x, int y, int dist, Worm* ownWorm)
{
	for(std::size_t i = 0; i < game.worms.size(); ++i)
	{
		Worm& w = game.worms[i];
		
		if(&w != ownWorm)
		{
			return checkForSpecWormHit(x, y, dist, w);
		}
	}
	
	return false;
}

bool checkForSpecWormHit(int x, int y, int dist, Worm& w)
{
	if(!w.visible)
		return false;
		
	PalIdx* wormSprite = gfx.wormSprite(w.currentFrame, w.direction, 0);
			
	int deltaX = x - ftoi(w.x) + 7;
	int deltaY = y - ftoi(w.y) + 5;
	
	Rect r(deltaX - dist, deltaY - dist, deltaX + dist + 1, deltaY + dist + 1);
	
	r.intersect(Rect(0, 0, 16, 16));
	
	for(int cy = r.y1; cy < r.y2; ++cy)
	for(int cx = r.x1; cx < r.x2; ++cx)
	{
		if(game.materials[wormSprite[cy*16 + cx]].worm())
			return true;
	}
	
	return false;
}

void Worm::processSight()
{
	WormWeapon& ww = weapons[currentWeapon];
	Weapon& w = game.weapons[ww.id];
	
	if(ww.available
	&& (w.laserSight || ww.id == C[LaserWeapon] - 1))
	{
		fixed dirX = cosTable[ftoi(aimingAngle)];
		fixed dirY = sinTable[ftoi(aimingAngle)];
		fixed tempX = x + dirX * 6;
		fixed tempY = y + dirY * 6 - itof(1);
		
		do
		{
			tempX += dirX;
			tempY += dirY;
			makeSightGreen = checkForWormHit(ftoi(tempX), ftoi(tempY), 0, this);
		}
		while(
			tempX >= 0 &&
			tempY >= 0 &&
			tempX < itof(game.level.width) &&
			tempY < itof(game.level.height) &&
			game.materials[game.level.pixel(ftoi(tempX), ftoi(tempY))].background() &&
			!makeSightGreen);
			
		hotspotX = ftoi(tempX);
		hotspotY = ftoi(tempY);
	}
	else
		makeSightGreen = false;
}

void Worm::processSteerables()
{
	WormWeapon& ww = weapons[currentWeapon];
	if(game.weapons[ww.id].shotType == Weapon::STSteerable)
	{
		for(Game::WObjectList::iterator i = game.wobjects.begin(); i != game.wobjects.end(); ++i)
		{
			if(i->id == ww.id && i->owner == this->index)
			{
				if(gfx.testKey(keyLeft()))
					i->curFrame -= (game.cycles & 1) + 1;
					
				if(gfx.testKey(keyRight()))
					i->curFrame += (game.cycles & 1) + 1;
					
				i->curFrame &= 127; // Wrap
				movable = false;
			}
		}
	}
}
