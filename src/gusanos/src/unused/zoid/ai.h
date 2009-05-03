// AI Player
// nym 16.10.04 - Created.

#ifndef AI_H
#define AI_H

#include "player.h"

#define DIR_LEFT -1
#define DIR_RIGHT 1

#define WALK_ACCEL 100
#define WALK_MAX 500

#define PI 3.14159
#define TODEG(x) (x)/(PI/180)

//Worm Difficulty
#define AI_EASY 0
#define AI_NORMAL 1
#define AI_HARD 2
#define AI_DEFAULT 0

//
class wormai : public worm
{
  int target;//Index of targetted worm
  int difficulty;//AI difficulty level
 public:
  wormai();
  void update();//Update worm state, position etc.
  //Set difficulty level
  void setDifficulty(int d)
  {
    if (d >= AI_EASY && d <= AI_HARD)
      difficulty = d;
  }
};

#endif

