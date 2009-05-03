#include "lights.h"
#include "level.h"
#include "engine.h"

bool obstacle;
BITMAP *lightbuff=NULL;
int x_origin;
int y_origin;
int color;
int fadeness;
int randomness;
int flashlight_color;
BITMAP *_where;
BITMAP *_material;
int _mask;


void do_collision(BITMAP *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(BITMAP *, int, int, int))
{
   int dx = x2-x1;
   int dy = y2-y1;
   int i1, i2;
   int x, y;
   int dd;

   /* worker macro */
   #define DO_COL(pri_sign, pri_c, pri_cond, sec_sign, sec_c, sec_cond)     \
   {                                                                         \
      if (d##pri_c == 0) {                                                   \
     proc(bmp, x1, y1, d);                                               \
     return;                                                             \
      }                                                                      \
                                         \
      i1 = 2 * d##sec_c;                                                     \
      dd = i1 - (sec_sign (pri_sign d##pri_c));                              \
      i2 = dd - (sec_sign (pri_sign d##pri_c));                              \
                                         \
      x = x1;                                                                \
      y = y1;                                                                \
                                         \
      while (pri_c pri_cond pri_c##2) {                                      \
     proc(bmp, x, y, d);                                                 \
     if (obstacle) {                    \
     return;                        \
     }                                  \
                                         \
     if (dd sec_cond 0) {                                                \
        sec_c sec_sign##= 1;                                             \
        dd += i2;                                                        \
     }                                                                   \
     else                                                                \
        dd += i1;                                                        \
                                         \
     pri_c pri_sign##= 1;                                                \
      }                                                                      \
   }

   if (dx >= 0) {
      if (dy >= 0) {
     if (dx >= dy) {
        DO_COL(+, x, <=, +, y, >=);
     }
     else {
        DO_COL(+, y, <=, +, x, >=);
     }
      }
      else {
     if (dx >= -dy) {
        DO_COL(+, x, <=, -, y, <=);
     }
     else {
        DO_COL(-, y, >=, +, x, >=);
     }
      }
   }
   else {
      if (dy >= 0) {
     if (-dx >= dy) {
        DO_COL(-, x, >=, +, y, >=);
     }
     else {
        DO_COL(+, y, <=, -, x, <=);
     }
      }
      else {
     if (-dx >= -dy) {
        DO_COL(-, x, >=, -, y, <=);
     }
     else {
        DO_COL(-, y, >=, -, x, <=);
     }
      }
   }
}

void check_obstacles(BITMAP *where, int x, int y, int d)
{
  int g=getpixel(where,x,y);
  //int g=where->line[y][x];
  if(map->mat[g+1].blocks_light) obstacle=true;
};

bool obs_line ( BITMAP*where, int x1 , int y1 , int x2 , int y2 )
{
  obstacle=false;
  do_collision(where, x1, y1, x2, y2, 0, check_obstacles);
  return obstacle;
};

void obs_light(BITMAP *where, int x, int y, int d)
{
  int dx,dy,i,col;
  int g=getpixel(where,x,y);
  //int g=where->line[y][x];
  if(map->mat[g+1].blocks_light) obstacle=true;
    
  if (map->paralax && getpixel(_where,x,y)==makecol(255,0,255));
  else
  if(getpixel(lightbuff,x,y)==0/* && !obstacle*/)
  {

    dx= abs(x_origin-x);
    dy= abs(y_origin-y);
    i=(fixtoi(fixhypot(itofix(dx),itofix(dy)))*fadeness)/100;
    if (randomness!=0)i+=rand()%randomness;
    col=255-i;
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    set_add_blender(0,0,0,col);
    //set_luminance_blender(0,0,0,col);
    //set_multiply_blender(0,0,0,col);
    //set_dodge_blender(0,0,0,col);
    //set_color_blender(0,0,0,col);
    //set_trans_blender(0,0,0,col);
    if(col<=255 && col>0)
    {
    putpixel(_where,x,y,color);
    //putpixel(map->background,x,y,color);
    };
    solid_mode();
    lightbuff->line[y][x]=1;
    //putpixel(lightbuff,x,y,1);
  };
};

void render_light( int x, int y, int r, int g, int b, int fade, int noise,BITMAP *where, BITMAP *material)
{
  int _x,_y,bx1,bx2,by1,by2,c;
  set_color_depth(8);
  //if(lightbuff==NULL)
  lightbuff=create_bitmap(material->w,material->h);
  set_color_depth(game->v_depth);
  //clear_to_color(lightbuff,0);
  
  _where=where;
  _material=material;
  
  color=makecol(r,g,b);
  fadeness=fade;
  randomness=noise;
  x_origin=x;
  y_origin=y;
  
  c=25500/fade;
  bx1=x-c;
  bx2=x+c;
  by1=y-c;
  by2=y+c;
  rectfill(lightbuff, bx1, by1, bx2, by2, 0);
  
  if(bx1<0)bx1=0;
  if(bx2>material->w)bx2=material->w-1;
  if(by1<0)by1=0;
  if(by2>material->h)by2=material->h-1;
  //bx1=0;
  //bx2=material->w;
  //by1=0;
  //by2=material->h;
  
  for(_x=bx1;_x<bx2;_x++)
  {
    obstacle=false;
    do_collision(material, x, y, _x, by1, 0, obs_light);
    obstacle=false;
    do_collision(material, x, y, _x, by2, 0, obs_light);
  };
  
  for(_y=by1;_y<=by2;_y++)
  {
    obstacle=false;
    do_collision(material, x, y, bx2, _y, 0, obs_light);
    obstacle=false;
    do_collision(material, x, y, bx1, _y, 0, obs_light);
  };
  
  destroy_bitmap(lightbuff);

};

void obs_light2(BITMAP *where, int x, int y, int d)
{
  int g=getpixel(where,x,y);
  //g=where->line[y][x]; 
  if(map->mat[g+1].blocks_light) obstacle=true;
    
  if (map->paralax && getpixel(map->mapimg,x,y)==makecol(255,0,255));
  else
  {
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    set_add_blender(0,0,0,255);
    putpixel(_where,x,y,flashlight_color);
    solid_mode();
  };
};

void flahslight_ray(BITMAP *where, int x, int y, int d)
{
  obstacle=false;
  do_collision(where, x_origin, y_origin, x, y, 0, obs_light2);
  //line(_where, x_origin, y_origin, x, y, 0);
};

void render_flashlight( int x, int y, int angle, int dir, BITMAP *where, BITMAP *material)
{
  int x1,y1,x2,y2;
  
  x_origin=x;
  y_origin=y;
  
  _where=where;
  _material=material;
  flashlight_color=makecol(20,20,20);
  
  
  x1=fixtoi(fixcos(ftofix(angle/1000+8))*200)*dir;
  y1=fixtoi(fixsin(ftofix(angle/1000+8))*200);
  x2=fixtoi(fixcos(ftofix(angle/1000-8))*200)*dir;
  y2=fixtoi(fixsin(ftofix(angle/1000-8))*200);
  
  do_line(material,x+x1,y-y1,x+x2,y-y2,0,flahslight_ray);
};

void obs_sunlight(BITMAP *where, int x, int y, int d)
{
  int col;
  int g=getpixel(where,x,y);
  //int g=where->line[y][x]; 
  if(map->mat[g+1].blocks_light) obstacle=true;
    
  if(getpixel(lightbuff,x,y)==0 && !obstacle)
  {
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    set_add_blender(0,0,0,255);
    
    col=40;
    putpixel(_where,x,y,makecol(col,col,col));
    putpixel(map->background,x,y,makecol(col,col,col));
    solid_mode();
    putpixel(lightbuff,x,y,1);
  };
};

void render_sunlight( BITMAP *where, BITMAP *material)
{
  int _x,_y;
  set_color_depth(8);
  lightbuff=create_bitmap(material->w,material->h);
  set_color_depth(game->v_depth);
  clear_to_color(lightbuff,0);
  _where=where;
  _material=material;
  
  
  
  for(_x=0;_x<material->w;_x++)
  {
    obstacle=false;
    do_collision(material, _x, 0, _x+material->h/3, material->h, 0, obs_sunlight);
  };
  
  for(_y=0;_y<material->h;_y++)
  {
    obstacle=false;
    do_collision(material, 0, _y, material->h/3, material->h+_y, 0, obs_sunlight);
  };
  
  /*for(_y=0;_y<material->h;_y++)
  {
    obstacle=false;
    do_collision(material, x, y, material->w, _y, 0, obs_light);
    obstacle=false;
    do_collision(material, x, y, 0, _y, 0, obs_light);
  };*/
  
  destroy_bitmap(lightbuff);

};

/*
void obs_dinsunlight(BITMAP *where, int x, int y, int d)
{
  int dx,dy,i,col;
  int g=getpixel(map->material,x,y);
  //int g=where->line[y][x]; 
  if(map->mat[g+1].blocks_light) obstacle=true;
    
  if(!obstacle && y_origin<=y)
  {
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    set_add_blender(0,0,0,255);
    
    col=40;
    putpixel(map->mapimg,x,y,makecol(col,col,col));
    putpixel(map->background,x,y,makecol(col,col,col));
    solid_mode();
  };
};

void check_sunlight( int x, int y)
{
  int _x,_y;
  _where=map->mapimg;
  _material=map->material;

  _x=(float)y*(((float)map->material->h/3.)/(float)map->material->h);
  y_origin=y;

  obstacle=false;
  do_collision(map->material, x-_x, 0, x-_x+map->material->h/3, map->material->h, 0, obs_dinsunlight);


};
*/

void obs_exp_light(BITMAP *where, int x, int y, int d)
{
  int dx,dy,i,col,wx,wy,_x,_y;
  //int g=getpixel(where,x,y);
  int g=_material->line[y][x];
  if(map->mat[g+1].blocks_light) obstacle=true;
    
  if (map->paralax==NULL || getpixel(map->mapimg,x,y)!=_mask)
  {
    wx=_where->w/2;
    wy=_where->h/2;
    _x=x-x_origin;
    _y=y-y_origin;
    if(lightbuff->line[_y+wy][_x+wx]==0)// && !obstacle)
    {
      dx= abs(_x);
      dy= abs(_y);
      i=(fixtoi(fixhypot(itofix(dx),itofix(dy)))*fadeness)/100;
      if (randomness)i+=rand()%randomness;
      col=255-i;
      drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
      set_add_blender(0,0,0,col);
      if(col<=1)obstacle=true;
      else putpixel(_where,_x+wx,_y+wy,color);
      solid_mode();
      lightbuff->line[_y+wy][_x+wx]=1;
    };
  };
};

void render_exp_light( int x, int y, int _color, int fade, int noise,int mask,BITMAP *where, BITMAP *material)
{
  int _x,_y,bx1,bx2,by1,by2,c;

  //clear_to_color(lightbuff,0);
  
  _mask=mask;
  _where=where;
  _material=material;
  
  color=_color;
  fadeness=fade;
  randomness=noise;
  x_origin=x;
  y_origin=y;
  
  c=(where->h-1)/2;
  bx1=x-c;
  bx2=x+c;
  by1=y-c;
  by2=y+c;
  //lightbuff=create_bitmap(bx2-bx1,by2-by1);
  set_color_depth(8);
  //if(lightbuff==NULL)
  lightbuff=create_bitmap(where->w,where->h);
  set_color_depth(game->v_depth);
  clear_to_color(lightbuff,0);
  //rectfill(lightbuff, bx1, by1, bx2, by2, 0);
  
  if(bx1<0)bx1=0;
  if(bx2>material->w-1)bx2=material->w-1;
  if(by1<0)by1=0;
  if(by2>material->h-1)by2=material->h-1;
  //bx1=0;
  //bx2=material->w;
  //by1=0;
  //by2=material->h;
  
  for(_x=bx1;_x<bx2;_x++)
  {
    obstacle=false;
    do_collision(material, x, y, _x, by1, 0, obs_exp_light);
    obstacle=false;
    do_collision(material, x, y, _x, by2, 0, obs_exp_light);
  };
  
  for(_y=by1;_y<=by2;_y++)
  {
    obstacle=false;
    do_collision(material, x, y, bx2, _y, 0, obs_exp_light);
    obstacle=false;
    do_collision(material, x, y, bx1, _y, 0, obs_exp_light);
  };
  destroy_bitmap(lightbuff);

};


/*void render_lens(int x, int y, int radius, BITMAP* where)
{
  float _x,_y,hyp,shyp,x2,y2,prop;
  for (_x=0;_x<radius;_x++)
  for (_y=0;_y<radius;_y++)
  {
    hyp=fixtof(fixhypot(ftofix(_x),ftofix(_y)));
    if (hyp<radius)
    {
      if(hyp!=0)
      {
        shyp=fixtof(fixsqrt(ftofix(radius*radius-hyp*hyp)));
        prop=((radius-shyp)+hyp)/2;
        x2=(_x*prop)/hyp;
        y2=(_y*prop)/hyp;
      } else
      {
        x2=_x;
        y2=_y;
      };
      putpixel(where,x+_x,y+_y,getpixel(map->mapimg,x+x2,y+y2));
      putpixel(where,x-_x,y+_y,getpixel(map->mapimg,x-x2,y+y2));
      putpixel(where,x+_x,y-_y,getpixel(map->mapimg,x+x2,y-y2));
      putpixel(where,x-_x,y-_y,getpixel(map->mapimg,x-x2,y-y2));
    }
  };
};*/


void render_lens(int x, int y, int radius, BITMAP* where, BITMAP* buf)
{
  float _x,_y,x1,y1,hyp,x2,y2,prop;
  for (_x=0;_x<=radius;_x++)
  for (_y=0;_y<=radius;_y++)
  {
    hyp=fixtof(fixhypot(ftofix(_x),ftofix(_y)));
    if (hyp<=radius)
    {
      if(hyp!=0)
      {
        prop=(hyp+(radius-hyp)*0)/radius;
        x2=_y;
        y2=-_x;
        x1=(x2-_x)*prop;
        y1=(y2-_y)*prop;
        putpixel(buf, (int)(radius + _x) , (int)(radius + _y) ,getpixel(where,(int)(x+x2-x1),(int)(y+y2-y1)));
        x2=-_y;
        y2=-_x;
        x1=(x2-_x)*prop;
        y1=(y2+_y)*prop;
        putpixel(buf,(int)(radius+_x),(int)(radius-_y),getpixel(where,(int)(x+x2-x1),(int)(y+y2-y1)));
        x2=-_y;
        y2=_x;
        x1=(x2+_x)*prop;
        y1=(y2+_y)*prop;
        putpixel(buf,(int)(radius-_x),(int)(radius-_y),getpixel(where,(int)(x+x2-x1),(int)(y+y2-y1)));
        x2=_y;
        y2=_x;
        x1=(x2+_x)*prop;
        y1=(y2-_y)*prop;
        putpixel(buf,(int)(radius-_x),(int)(radius+_y),getpixel(where,(int)(x+x2-x1),(int)(y+y2-y1)));
      } else
      {
        x2=_x;
        y2=_y;
        putpixel(buf,(int)(radius+_x),(int)(radius+_y),getpixel(where,(int)(x+x2),(int)(y+y2)));
      };
    } else
    {
      putpixel(buf, (int)(radius+_x) , (int)(radius+_y),getpixel(where,(int)(x+_x),(int)(y+_y)));
      putpixel(buf, (int)(radius+_x) , (int)(radius-_y),getpixel(where,(int)(x+_x),(int)(y-_y)));
      putpixel(buf, (int)(radius-_x) , (int)(radius-_y),getpixel(where,(int)(x-_x),(int)(y-_y)));
      putpixel(buf, (int)(radius-_x) , (int)(radius+_y),getpixel(where,(int)(x-_x),(int)(y+_y)));
    };
  };
  blit(buf,where,0,0,x-radius,y-radius,buf->w,buf->h);
};

void c_segments::create_segment(float x1,float x2,float y)
{
  
  seg[segcount].b1.x=x1;
  seg[segcount].b1.y=yorigin-y;
  seg[segcount].b1.xinc=(x1-xorigin)/y;
  seg[segcount].b2.x=x2;
  seg[segcount].b2.y=yorigin-y;
  seg[segcount].b2.xinc=(x2-xorigin)/y;
  segcount++;
};

/*
void c_segments::remove_segment(int index)
{
  segcount--;
  if (index!=segcount)seg[index]=seg[segcount];
};

void c_segments::move_segmentsup()
{
  int i;
  for (i=0;i<segcount;i++)
  {
    seg[i].b2.x+=seg[i].b2.xinc;
    seg[i].b1.x+=seg[i].b1.xinc;
  };
};
*/
/*
void render_seglight( int x, int y, int angle, int dir, BITMAP *where, BITMAP *material)
{
  int x1,y1,x2,y2,i,_y;
  

  
  _where=where;
  _material=material;
  

  //struct s_segment segment[100];
  c_segments segs;
  segs.xorigin=x;
  segs.yorigin=y;
  segs.seg[0].b1.x=x;
  segs.seg[0].b1.y=y;
  segs.seg[0].b1.xinc=-1;
  segs.seg[0].b2.x=x;
  segs.seg[0].b2.y=y;
  segs.seg[0].b2.xinc=1;
  segs.segcount=1;
  
  _y=0;
  while (_y<100)
  {
    //segs.move_segmentsup();
    //int p=segs.segcount;
    for (i=0;i<segs.segcount;i++)
    {
      int _x,lastx;
      bool collision,sthhappened=false;
      line(where,segs.seg[i].b1.x,y-_y,segs.seg[i].b2.x,y-_y,makecol(255,255,255));
      segs.seg[i].b1.x+=segs.seg[i].b1.xinc;
      segs.seg[i].b2.x+=segs.seg[i].b2.xinc;
      int g=getpixel(material,segs.seg[i].b1.x,y-_y);
      collision=map->mat[g+1].blocks_light;
      lastx=segs.seg[i].b1.x;
      for ( _x=segs.seg[i].b1.x ; _x<segs.seg[i].b2.x ; _x++)
      {
        g=getpixel(material,_x,y-_y);
        if(map->mat[g+1].blocks_light && !collision)
        {
          //line(where,lastx,y-_y,_x-1,y-_y,makecol(255,255,255));
          segs.create_segment(lastx,_x-1,_y);
          collision=true;
          sthhappened=true;
        };
        if(!map->mat[g+1].blocks_light && collision)
        {
          lastx=_x;
          collision=false;
          sthhappened=true;
        };
      };
      if (sthhappened)
      {
        g=getpixel(material,_x-1,y-_y);
        if (!map->mat[g+1].blocks_light);
          segs.create_segment(lastx,_x-1,_y);
          //segs.seg[segs.segcount-1].b2.xinc=segs.seg[i].b2.xinc;
          //line(where,lastx,y-_y,_x-1,y-_y,makecol(255,255,255));
        segs.remove_segment(i);
      }else
      {
        if(collision)
          segs.remove_segment(i);
        //line(where,segs.seg[i].b1.x,y-_y,segs.seg[i].b2.x,y-_y,makecol(255,255,255));
      };
    };
    _y++;
  };
  
  //do_line(material,x+x1,y-y1,x+x2,y-y2,0,flahslight_ray);
};
*/


