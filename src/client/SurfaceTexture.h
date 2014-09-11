//
//  SurfaceTexture.h
//  OpenLieroX
//
//  Created by Albert Zeyer on 10.09.14.
//	code under LGPL
//

#ifndef __OpenLieroX__SurfaceTexture__
#define __OpenLieroX__SurfaceTexture__

#include <vector>
#include "CodeAttributes.h"
#include "SmartPointer.h"

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;
struct SDL_Rect;

/*
This represents a big virtual texture, composed of multiple smaller textures,
which is backed up by a surface.
So, you can update the surface, and then you need to upload the modified
area back into the texture by updateArea().
*/
class SurfaceTexture : DontCopyTag {
	SmartPointer<SDL_Renderer> m_renderer;
	SmartPointer<SDL_Surface> m_surface;
	std::vector<SmartPointer<SDL_Texture> > m_textures;

	int w, h;
	int maxTextureWidth, maxTextureHeight;
	int numTexturesHoriz, numTexturesVert;
	
	void _init();
	
public:
	SurfaceTexture(const SmartPointer<SDL_Renderer>& renderer, const SmartPointer<SDL_Surface>& surf);
	~SurfaceTexture();

	int width() const { return w; }
	int height() const { return h; }
	
	void updateArea(const SDL_Rect* rect);
	void render(const SDL_Rect * srcrect, const SDL_Rect * dstrect);
};

#endif /* defined(__OpenLieroX__SurfaceTexture__) */
