/*
 *  LXMapFlags.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 09.12.09.
 *  code under LGPL
 *
 */

#ifndef __LXMAPFLAGS_H__
#define __LXMAPFLAGS_H__


#define		MAP_VERSION	0

// Map types
#define		MPT_PIXMAP	0
#define		MPT_IMAGE	1

// Pixel flags
#define		PX_EMPTY	0x01
#define		PX_DIRT		0x02
#define		PX_ROCK		0x04
#define		PX_SHADOW	0x08
#define		PX_WORM		0x10

// Object types
#define		OBJ_HOLE	0
#define		OBJ_STONE	1
#define		OBJ_MISC	2

#define		MAX_OBJECTS	8192

// Antialiasing blur
#define		MINIMAP_BLUR	10.0f

// Shadow drop
#define		SHADOW_DROP	3




#endif
