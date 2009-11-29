// AI Player

#include "ai.h"
#include "engine.h"
#include "console.h"
#include "network.h"
#include "player.h"
#include "weapons.h"
#include "level.h"
#include "sounds.h"
#include "particles.h"
#include "water.h"
#include "render.h"
#include "explosions.h"
#include "sprites.h"
#include "text.h"

#include <math.h>

wormai::wormai() : target(0), difficulty(AI_DEFAULT) { }

void wormai::update()
{
  if (!active)
    return;
  //Make sure target is active and that target isn't itself
  if (!player[target]->active || player[target] == this)
  {
    //Get new target
    target = rand()%player_count;
    return;
  }

  //Move towards target
  int d = (int)sqrt((float)((x - player[target]->x)*(x - player[target]->x) + (y - player[target]->y)*(y - player[target]->y)));
  if (abs(d/1000) > 50)
  {
    if (x > player[target]->x)
    {
      walk(DIR_LEFT, *game->ACELERATION, *game->MAX_SPEED);
      dir = DIR_LEFT;
    } else
    {
      walk(DIR_RIGHT, *game->ACELERATION, *game->MAX_SPEED);
      dir = DIR_RIGHT;
    }
  }

  //Update aiming angle
  int dx = player[target]->x/1000 - x/1000;
  int dy = player[target]->y/1000 - y/1000;
  float newAngle = atan2((float)dy, (float)dx);
  while (newAngle > 2*PI)
  {
    newAngle -= 2*PI;
  }
  while (newAngle < 0)
  {
    newAngle += 2*PI;
  }
  //0 straight down
  //128 straight up
  //AIMS PERFECT WHEN TARGET WORM IS IN QUADRANT 1 OR 4
  int newAim = (int)(64 - (TODEG(newAngle) * (256.0/360.0)))%256 * 1000;
  if (player[target]->x < x)
  {
    newAim = 128000 + (128000 - newAim);
  }
  newAim %= 256000;
  if (newAim < 0)
    newAim += 256000;
  
  //Difficulty  
  switch (difficulty)
  {
  //Default to easy :>
  default:
  case AI_EASY:
    //Aiming...
    if (newAim > aim)
      aim_speed += 100;
    else if (newAim < aim)
      aim_speed -= 100;
    break;
  case AI_NORMAL:
    //Faster aiming
    if (newAim > aim)
      aim_speed += 200;
    else if (newAim < aim)
      aim_speed -= 200;
    break;
  case AI_HARD:
    //Perfect aiming
    aim = newAim;
    break;
  }

  //printf("ANGLE: %f, AIM: %i, DX: %i, DY: %i\n", TODEG(newAngle), aim / 1000, dx, dy);

  //Fire at target
  shoot();

  //Change weapon
  if (rand() % 100 == 0)
    curr_weap = rand()%5;

  //Jump
  if (rand() % 50 == 0)
    jump(1000);

  //Rope
  if (rand() % 200 == 0)
  {
    aim = 96000;
    shootrope();
  } else if (rand() % 150 == 0)
  {
    destroyrope();
  }
}
