#include "gfx.h"
#include "text.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include "loadpng.h"

//Load image depending on format
BITMAP* load_image(const char* tmp, RGB* palette)
{
  std::string frmt,ufrmt;
  frmt=tmp;
  ufrmt=frmt.substr(frmt.length() - 3);
  std::transform(ufrmt.begin(), ufrmt.end(), ufrmt.begin(), toupper);
  BITMAP* ret;
  if (ufrmt=="BMP")
    {
      ret = load_bmp(tmp,palette);
    }
  else if (ufrmt=="PNG")
    {
      ret = load_png(tmp, palette);
    }
  return ret;
}

BITMAP* loadImage(const char* tmp, RGB* palette)
{
  std::string filename;
  
  filename = tmp;
  if ( exists( filename.c_str() ) )
  {
    return load_image( filename.c_str() , palette );
  }
  //filename = tmp;
  filename += ".png";
  if ( exists( filename.c_str() ) )
  {
    return load_png( filename.c_str() , palette );
  }
  filename = tmp;
  filename += ".bmp";
  if ( exists( filename.c_str() ))
  {
    return load_bmp( filename.c_str() , palette );
  }
  return NULL;
}
