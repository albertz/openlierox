#include "bonus.hpp"
#include "game.hpp"
#include "constants.hpp"
#include <iostream>

void Bonus::process()
{
	y += velY;
	
	int ix = ftoi(x), iy = ftoi(y);
	
	if(game.level.inside(ix, iy + 1)
	&& game.materials[game.level.pixel(ix, iy + 1)].background())
	{
		velY += C[BonusGravity];
	}
		
	int inewY = ftoi(y + velY);
	if(inewY < 0 || inewY >= game.level.height - 1
	|| game.materials[game.level.pixel(ix, inewY)].dirtRock())
	{
		velY = -(velY * C[BonusBounceMul]) / C[BonusBounceDiv];
		
		if(std::abs(velY) < 100) // TODO: Read from EXE
			velY = 0;
	}
	
	if(--timer <= 0)
	{
		game.sobjectTypes[game.bonusSObjects[frame]].create(ix, iy, 0);
		if(used)
			game.bonuses.free(this);
	}
}
