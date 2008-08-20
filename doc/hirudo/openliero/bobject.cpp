#include "bobject.hpp"

#include "colour.hpp"
#include "game.hpp"
#include "constants.hpp"

void createBObject(fixed x, fixed y, fixed velX, fixed velY)
{
	BObject& obj = *game.bobjects.newObjectReuse();
	
	obj.colour = game.rand(C[NumBloodColours]) + C[FirstBloodColour];
	obj.x = x;
	obj.y = y;
	obj.velX = velX;
	obj.velY = velY;
}

void BObject::process()
{
	x += velX;
	y += velY;
	
	int ix = ftoi(x);
	int iy = ftoi(y);
	
	if(!game.level.inside(ix, iy))
	{
		game.bobjects.free(this);
	}
	else
	{
		PalIdx c = game.level.pixel(ix, iy);
		Material m = game.materials[c];
		
		if(m.background())
			velY += C[BObjGravity];
			
		if((c >= 1 && c <= 2)
		|| (c >= 77 && c <= 79)) // TODO: Read from EXE
		{
			game.level.pixel(ix, iy) = 77 + game.rand(3);
			game.bobjects.free(this);
		}
		/* This can't happen!
		else if(iy >= game.level.height)
		{
			// Nothing
		}*/
		else if(m.anyDirt())
		{
			game.level.pixel(ix, iy) = 82 + game.rand(3);
			game.bobjects.free(this);
		}
		else if(m.rock())
		{
			game.level.pixel(ix, iy) = 85 + game.rand(3);
			game.bobjects.free(this);
		}
	}
}
