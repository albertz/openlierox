//
//  SurfaceTexture.h
//  OpenLieroX
//
//  Created by Albert Zeyer on 10.09.14.
//	code under LGPL
//

#ifndef __OpenLieroX__SurfaceTexture__
#define __OpenLieroX__SurfaceTexture__

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;
struct SDL_Rect;

/*
This represents a texture
(might be represented by several small textures - not implemented yet)
which is backed up by a surface.
So, you can update the surface, and then you need to upload the modified
area back into the texture by updateArea().
*/
class SurfaceTexture {
	SDL_Surface* m_surface;
	SDL_Texture* m_texture;

public:
	void updateArea(const SDL_Rect* rect);
	void render(SDL_Renderer* dest, const SDL_Rect * srcrect, const SDL_Rect * dstrect);
};

#endif /* defined(__OpenLieroX__SurfaceTexture__) */
