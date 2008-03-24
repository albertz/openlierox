#include "worm.hpp"
#include "constants.hpp"
#include "game.hpp"
#include "colour.hpp"
#include "math.hpp"
//#include <cmath>

void Ninjarope::process(Worm& owner)
{
	if(out)
	{
		x += velX;
		y += velY;
		
		int ix = ftoi(x), iy = ftoi(y);
		
		anchor = 0;
		for(std::size_t i = 0; i < game.worms.size(); ++i)
		{
			Worm& w = *game.worms[i];
			
			if(&w != &owner
			&& checkForSpecWormHit(ix, iy, 1, w))
			{
				anchor = &w;
				break;
			}
		}
		
		fixed forceX, forceY;
		
		fixed diffX = x - owner.x;
		fixed diffY = y - owner.y;
		
		forceX = (diffX << C[NRForceShlX]) / C[NRForceDivX];
		forceY = (diffY << C[NRForceShlY]) / C[NRForceDivY];
		
		curLen = (vectorLength(ftoi(diffX), ftoi(diffY)) + 1) << C[NRForceLenShl];
		
		if(ix <= 0
		|| ix >= game.level.width - 1
		|| iy <= 0
		|| iy >= game.level.height - 1
		|| game.materials[game.level.pixel(ix, iy)].dirtRock())
		{
			if(!attached)
			{
				length = C[NRAttachLength];
				attached = true;
				
				if(game.level.inside(ix, iy))
				{
					PalIdx pix = game.level.pixel(ix, iy);
					
					if(game.materials[pix].anyDirt())
					{
						for(int i = 0; i < 11; ++i) // TODO: Check 11 and read from exe
						{
							game.nobjectTypes[2].create2(
								game.rand(128),
								0, 0,
								x, y,
								pix,
								&owner);
						}
					}
				}
			}
			
			
			velX = 0;
			velY = 0;
		}
		else if(anchor)
		{
			if(!attached)
			{
				length = C[NRAttachLength]; // TODO: Should this value be separate from the non-worm attaching?
				attached = true;
			}
			
			if(curLen > length)
			{
				anchor->velX -= forceX / curLen;
				anchor->velY -= forceY / curLen;
			}
			
			velX = anchor->velX;
			velY = anchor->velY;
			x = anchor->x;
			y = anchor->y;
		}
		else
		{
			attached = false;
		}
		
		if(attached)
		{
			// curLen can't be 0

			if(curLen > length)
			{
				owner.velX += forceX / curLen;
				owner.velY += forceY / curLen;
			}
		}
		else
		{
			velY += C[NinjaropeGravity];

			if(curLen > length)
			{
				velX -= forceX / curLen;
				velY -= forceY / curLen;
			}
		}
	}
}
