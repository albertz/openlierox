#include "sobject.hpp"

#include "worm.hpp"
#include "game.hpp"
#include "viewport.hpp"
#include "gfx.hpp"
#include "sfx.hpp"
#include "console.hpp"
#include "text.hpp"
#include <cmath>
#include <cassert>
#include <iostream> // TEMP

void SObjectType::create(int x, int y, int owner)
{
	SObject& obj = *game.sobjects.newObjectReuse();
	
	assert(numSounds < 10);
	
	if(startSound >= 0)
		sfx.play(game.rand(numSounds) + startSound, startSound);
		
	for(std::size_t i = 0; i < game.viewports.size(); ++i)
	{
		Viewport& v = game.worms[i].viewport;
		
		if(x > v.x
		&& x < v.x + v.rect.width()
		&& y > v.y
		&& y < v.y + v.rect.height())
		{
			if(itof(shake) > v.shake)
				v.shake = itof(shake);
		}
	}
	
	obj.id = id;
	obj.x = x - 8;
	obj.y = y - 8;
	obj.curFrame = 0;
	obj.animDelay = animDelay;
	
	if(flash > gfx.screenFlash)
	{
		gfx.screenFlash = flash;
	}
		
	if(damage > 0)
	{
		for(std::size_t i = 0; i < game.worms.size(); ++i)
		{
			Worm& w = game.worms[i];
			
			int wix = ftoi(w.x);
			int wiy = ftoi(w.y);
			
			if(wix < x + detectRange
			&& wix > x - detectRange
			&& wiy < y + detectRange
			&& wiy > y - detectRange)
			{
				int delta = wix - x;
				int power = detectRange - std::abs(delta);
				int powerSum = power;
				
				if(std::abs(w.velX) < itof(2)) // TODO: Read from EXE
				{
					if(delta > 0)
						w.velX += blowAway * power;
					else
						w.velX -= blowAway * power;
				}
				
				delta = wiy - y;
				power = detectRange - std::abs(delta);
				powerSum = (powerSum + power) / 2;
				
				if(std::abs(w.velY) < itof(2)) // TODO: Read from EXE
				{
					if(delta > 0)
						w.velY += blowAway * power;
					else
						w.velY -= blowAway * power;
				}
				
				int z = damage * powerSum;
				if(detectRange)
					z /= detectRange;
					
				if(w.health > 0)
				{
					w.health -= z;
					
					if(w.health <= 0)
						w.lastKilledBy = owner;
						
					int bloodAmount = game.settings.blood * powerSum / 100;
					
					if(bloodAmount > 0)
					{
						for(int i = 0; i < bloodAmount; ++i)
						{
							game.nobjectTypes[6].create2(
								game.rand(128),
								w.velX / 3, w.velY / 3,
								w.x, w.y,
								0,
								w.index);
						}
					}
					
					if(game.rand(3) == 0
					&& !sfx.isPlaying(w.wormSoundID))
					{
						sfx.play(18 + game.rand(3), w.wormSoundID);
					}
				}
			}
		} // for( ... worms ...
		
		int objBlowAway = blowAway / 3; // TODO: Read from EXE
		
		for(Game::WObjectList::iterator i = game.wobjects.begin(); i != game.wobjects.end(); ++i)
		{
			Weapon& weapon = game.weapons[i->id];
			
			if(weapon.affectByExplosions)
			{
				int ix = ftoi(i->x), iy = ftoi(i->y);
				if(ix < x + detectRange
				&& ix > x - detectRange
				&& iy < y + detectRange
				&& iy > y - detectRange)
				{
					int delta = ix - x;
					int power = detectRange - std::abs(delta);
					
					if(power > 0)
					{
						if(delta > 0)
							i->velX += objBlowAway * power;
						else if(delta < 0)
							i->velX -= objBlowAway * power;
					}
					
					delta = iy - y;
					power = detectRange - std::abs(delta);
					
					if(power > 0)
					{
						if(delta > 0)
							i->velY += objBlowAway * power;
						else if(delta < 0)
							i->velY -= objBlowAway * power;
					}
					
					// Is it a booby trap?
					if(i->id == 34) // TODO: Read from EXE
					{
						i->blowUpObject(owner);
					}
				}
			} // if( ... affectByExplosions ...
		} // for( ... wobjects ...
		
		for(Game::NObjectList::iterator i = game.nobjects.begin(); i != game.nobjects.end(); ++i)
		{
			NObjectType& t = game.nobjectTypes[i->id];
		
			if(t.affectByExplosions)
			{
				int ix = ftoi(i->x), iy = ftoi(i->y);
				if(ix < x + detectRange
				&& ix > x - detectRange
				&& iy < y + detectRange
				&& iy > y - detectRange)
				{
					int delta = ix - x;
					int power = detectRange - std::abs(delta);
					
					if(power > 0)
					{
						if(delta > 0)
							i->velX += objBlowAway * power;
						else if(delta < 0)
							i->velX -= objBlowAway * power;
					}
					
					delta = iy - y;
					power = detectRange - std::abs(delta);
					
					if(power > 0)
					{
						if(delta > 0)
							i->velY += objBlowAway * power;
						else if(delta < 0)
							i->velY -= objBlowAway * power;
					}
				}
			}
		}
				
		{
			int width = detectRange / 2;
			
			Rect rect(x - width, y - width, x + width + 1, y + width + 1);
			
			rect.intersect(game.level.rect());
			
			for(int y = rect.y1; y < rect.y2; ++y)
			for(int x = rect.x1; x < rect.x2; ++x)
			{
				PalIdx pix = game.level.pixel(x, y);
				if(game.materials[pix].anyDirt()
				&& game.rand(8) == 0)
				{
					game.nobjectTypes[2].create2(
						game.rand(128),
						0, 0,
						itof(x), itof(y),
						pix, owner);
				}
			}
		}
		
	} // if(damage ...
	
	if(dirtEffect >= 0)
	{
		drawDirtEffect(dirtEffect, x - 7, y - 7);
		
		correctShadow(Rect(x - 10, y - 10, x + 11, y + 11));
	}
	
	for(Game::BonusList::iterator i = game.bonuses.begin(); i != game.bonuses.end(); ++i)
	{
		int ix = ftoi(i->x), iy = ftoi(i->y);
		
		if(ix > x - detectRange
		&& ix < x + detectRange
		&& iy > y - detectRange
		&& iy < y + detectRange)
		{
			game.bonuses.free(i);
			game.sobjectTypes[0].create(ix, iy, owner);
		}
	} // for( ... bonuses ...
}

void SObject::process()
{
	SObjectType& t = game.sobjectTypes[id];
	
	
	if(--animDelay <= 0)
	{
		animDelay = t.animDelay;
		++curFrame;
		if(curFrame > t.numFrames)
			game.sobjects.free(this);
	}
}
