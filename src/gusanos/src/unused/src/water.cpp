// -Water algorithms-
// For reasons of optimization I have used two bitmaps for water. One is the map->material bitmap
// which serves as the collision map for particles, The otherone is the map->water_buffer bitmap
// which is used to store the stagnated water.

#include "water.h"

struct s_water *water;
struct s_water_spawn water_spawn[1000];
int water_spawn_count;

void stagnate_water(int index)
{
  // puts a pixel in the water_buffer bitmap indicating that there is stagnated water there 
  // useing direct memory writting
  
  map->water_buffer->line[water[index].y][water[index].x]=3;
  
  // brings the last particle on the array to the position of the particle to remove
  // and decrases the water count variable
  
  water[index]=water[game->water_count-1];
  game->water_count--;
};

void remove_water(int index)
{
  // puts a pixel in the water_buffer bitmap indicating that there is stagnated water there 
  // useing direct memory writting
  check_hole_sides(water[index].x,water[index].y);
  map->material->line[water[index].y][water[index].x]=1;//water[index].matunder;
  putpixel(map->mapimg,water[index].x,water[index].y,getpixel(map->background,water[index].x,water[index].y));
  // brings the last particle on the array to the position of the particle to remove
  // and decrases the water count variable

  water[index]=water[game->water_count-1];
  game->water_count--;
};

void create_water(int x,int y,int dir)
{
  // It writes the water_buffer bitmap indicating that the water stagnated in there is not stagnated any more
  
  map->water_buffer->line[y][x]=1;
  
  // creates a new particle in the array and increases the water count
  
  water[game->water_count].x=x;
  water[game->water_count].y=y;
  water[game->water_count].dir=dir;
  water[game->water_count].time=0;
  water[game->water_count].matunder=1;
  water[game->water_count].color=/*makecol(90,90,255);//*/((short *)map->mapimg->line[y])[x];
  game->water_count++;
};

void check_water_sides(int index)
{
  // It checks the sides and the above of the water particle looking for stagnated water if it finds 
  // stagnated water it creates a new particle to reactivate that stagnated water
  
  if (map->mat[getpixel(map->water_buffer,water[index].x+1,water[index].y)+1].flows) create_water(water[index].x+1,water[index].y,water[index].dir);
  if (map->mat[getpixel(map->water_buffer,water[index].x-1,water[index].y)+1].flows) create_water(water[index].x-1,water[index].y,water[index].dir);
  if (map->mat[getpixel(map->water_buffer,water[index].x,water[index].y-1)+1].flows) create_water(water[index].x,water[index].y-1,water[index].dir);
};

void check_hole_sides(int x,int y)
{
  // Does the same that the check_water_sides function but from a given coordinate.
  // It is used when a dirt pixel of the map->material bitmap is removed.
  
  if (map->mat[getpixel(map->water_buffer,x+1,y)+1].flows) create_water(x+1,y,1);
  if (map->mat[getpixel(map->water_buffer,x-1,y)+1].flows) create_water(x-1,y,1);
  if (map->mat[getpixel(map->water_buffer,x,y-1)+1].flows) create_water(x,y-1,1);
};

void calc_water()
{
  int o,i,c;
  
  // If there is active some water check all particles from the end of the array to the begining,
  // in that way I avoid calculating all the particles created in the process
  
  if (game->water_count>0)
  for(o=game->water_count-1;o>=0;o--)
  {
    
    c=o;
    
    // It skips some of the particles to give the water a more realistic effect
    i=rand()%20;
    if(i!=0)
    {
      // If there is no water below
      if (water[c].y+1>map->material->h-1) i=-1;
      else i=map->material->line[water[c].y+1][water[c].x];
      if(map->mat[i+1].destroys_water)
      {
        remove_water(c);
      }else if(map->mat[i+1].particle_pass && !map->mat[i+1].flows)
      {
        // move the particle down
        map->material->line[water[c].y+1][water[c].x]=map->material->line[water[c].y][water[c].x];
        map->material->line[water[c].y][water[c].x]=water[c].matunder;
        water[c].matunder=i;
        
        /*drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
        set_trans_blender(0,0,0,128);*/
        putpixel(map->mapimg,water[c].x,water[c].y+1,water[c].color);
        //solid_mode();
        putpixel(map->mapimg,water[c].x,water[c].y,getpixel(map->background,water[c].x,water[c].y));

        // check if it left some stagnated water free
        check_water_sides(c);
        
        water[c].y++;
        
        // reset the stagnated time counter
        water[c].time=0;
      } else
      {
        // If there is no water in the direction in which its moving
        if (water[c].x+water[c].dir>map->material->w-1) i=-1;
        else if (water[c].x+water[c].dir < 0) i=-1;
        else i=map->material->line[water[c].y][water[c].x+water[c].dir];
        if(map->mat[i+1].destroys_water)
        {
          remove_water(c);
        }else if(map->mat[i+1].particle_pass && !map->mat[i+1].flows)
        {
          // move the particle towards its direction
          map->material->line[water[c].y][water[c].x+water[c].dir]=map->material->line[water[c].y][water[c].x];
          map->material->line[water[c].y][water[c].x]=water[c].matunder;
          water[c].matunder=i;
          /*drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
          set_trans_blender(0,0,0,128);*/
          putpixel(map->mapimg,water[c].x+water[c].dir,water[c].y,water[c].color);
          //solid_mode();
          putpixel(map->mapimg,water[c].x,water[c].y,getpixel(map->background,water[c].x,water[c].y));
          
          // check if it left some stagnated water free
          check_water_sides(c);
          
          water[c].x+=water[c].dir;
          
          // reset the stagnated time counter
          water[c].time=0; 
        } else
        {
          // If it was not able to move in any direction it means it is stagnated
          
          // Start counting the time being stagnated
          
          
          // Inverse its direction
          water[c].dir=water[c].dir*-1;
          water[c].time++;
          
          
          if(water[c].time>10)
          {
            int g;
            i=0;
            g=getpixel(map->material,water[c].x+1,water[c].y);
            if (map->mat[g+1].particle_pass && !map->mat[g+1].flows) i=1;
            g=getpixel(map->material,water[c].x-1,water[c].y);
            if (map->mat[g+1].particle_pass && !map->mat[g+1].flows) i=1;

            if (i==0)
            stagnate_water(c); // If it has been stagnated for 10 cycles remove it.
            else water[c].time=0;
          };
        };
      };
    };
  };
  for(o=water_spawn_count-1;o>=0;o--)
  {
    if(map->material->line[water_spawn[o].y][water_spawn[o].x]==1)
    {
      putpixel(map->material,water_spawn[o].x,water_spawn[o].y,3);
      putpixel(map->water_buffer,water_spawn[o].x,water_spawn[o].y,3);
      create_water(water_spawn[o].x,water_spawn[o].y,1);
    };
  };
};

void create_waterlist()
{
  int x,y,g;
  game->water_count=0;
  water_spawn_count=0;
  for(x=0;x<map->material->w;x++)
  for(y=0;y<map->material->h;y++)
  {
    g=getpixel(map->material,x,y);
    if(map->mat[g+1].flows)
    {
      if(map->mat[getpixel(map->material,x+1,y)+1].flows && map->mat[getpixel(map->material,x-1,y)+1].flows && map->mat[getpixel(map->material,x,y+1)+1].flows)
      {
        putpixel(map->water_buffer,x,y,g);
      } else
      {
        water[game->water_count].x=x;
        water[game->water_count].y=y;
        water[game->water_count].dir=1;
        water[game->water_count].matunder=1;
        //water[game->water_count].color=makecol(0,0,200+rand()%55);
        water[game->water_count].color=getpixel(map->mapimg,x,y);
        //putpixel(map->mapimg,x,y,water[game->water_count].color);
        game->water_count++;
      };
      map->has_water=true;
    };
    if(map->mat[g+1].creates_water)
    {
      water_spawn[water_spawn_count].x=x;
      water_spawn[water_spawn_count].y=y;
      putpixel(map->material,x,y,1);
      putpixel(map->background,x,y,getpixel(map->mapimg,x,y));
      water_spawn_count++;
      map->has_water=true;
    };
  };
};
