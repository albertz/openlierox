//
//  PixelFunctors.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 28.12.11.
//  Copyright (c) 2011 Albert Zeyer. All rights reserved.
//

#include "PixelFunctors.h"
#include "GfxPrimitives.h"

PixelCopy& getPixelCopyFunc(const SDL_Surface *source_surf, const SDL_Surface *dest_surf)
{
	const bool issameformat = PixelFormatEqual(source_surf->format, dest_surf->format);
	const bool alphablend = Surface_HasBlendMode(source_surf);
	const bool colorkeycheck = Surface_HasColorKey(source_surf);
	const bool srchasalpha = source_surf->format->Amask != 0;
	const bool dsthasalpha = dest_surf->format->Amask != 0;
	const int srcbytespp = source_surf->format->BytesPerPixel;
	const int dstbytespp = dest_surf->format->BytesPerPixel;
	Color colorkey;
	if(colorkeycheck)
		colorkey = Color(source_surf->format, Surface_GetColorKey(source_surf));
	
#define _RET_PIXELCOPY(av1, av2, av3, av4, av5, av6, av7) \
	return PixelCopy_Class<av1,av2,av3,av4,av5,av6,av7>::getInstance(source_surf->format, dest_surf->format, colorkey);

#define _BRANCH7(av1, av2, av3, av4, av5, av6) \
	{ \
		if(dstbytespp == 4) _RET_PIXELCOPY(av1, av2, av3, av4, av5, av6, 4) \
		else if(dstbytespp == 3) _RET_PIXELCOPY(av1, av2, av3, av4, av5, av6, 3) \
		else if(dstbytespp == 2) _RET_PIXELCOPY(av1, av2, av3, av4, av5, av6, 2) \
		else if(dstbytespp == 1) _RET_PIXELCOPY(av1, av2, av3, av4, av5, av6, 1) \
		else assert(false); \
	}

#define _BRANCH6(av1,av2,av3,av4,av5) \
	{ \
		if(srcbytespp == 4) _BRANCH7(av1,av2,av3,av4,av5, 4) \
		else if(srcbytespp == 3) _BRANCH7(av1,av2,av3,av4,av5, 3) \
		else if(srcbytespp == 2) _BRANCH7(av1,av2,av3,av4,av5, 2) \
		else if(srcbytespp == 1) _BRANCH7(av1,av2,av3,av4,av5, 1) \
		else assert(false); \
	}

#define _BRANCH5(av1,av2,av3,av4) \
	{ \
		if(dsthasalpha) _BRANCH6(av1,av2,av3,av4, true) \
		else _BRANCH6(av1,av2,av3,av4, false); \
	}

#define _BRANCH4(av1,av2,av3) \
	{ \
		if(srchasalpha) _BRANCH5(av1,av2,av3, true) \
		else _BRANCH5(av1,av2,av3, false); \
	}
	
#define _BRANCH3(av1,av2) \
	{ \
		if(colorkeycheck) _BRANCH4(av1,av2, true) \
		else _BRANCH4(av1,av2, false); \
	}

#define _BRANCH2(av1) \
	{ \
		if(alphablend) _BRANCH3(av1, true) \
		else _BRANCH3(av1, false); \
	}
	
#define _BRANCH1() \
	{ \
		if(issameformat) _BRANCH2(true) \
		else _BRANCH2(false); \
	}
	
	_BRANCH1()
	
	// for stupid compilers
	return PixelCopy_Class<false,false,false,false,false,1,1>::getInstance(source_surf->format, dest_surf->format, colorkey);
}
