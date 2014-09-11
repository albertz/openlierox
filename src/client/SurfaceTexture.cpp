//
//  SurfaceTexture.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 10.09.14.
//	code under LGPL
//

#include <SDL.h>
#include <assert.h>
#include "SurfaceTexture.h"
#include "Debug.h"

SurfaceTexture::SurfaceTexture(const SmartPointer<SDL_Renderer>& renderer, const SmartPointer<SDL_Surface>& surf)
: w(0), h(0), maxTextureWidth(0), maxTextureHeight(0), numTexturesHoriz(0), numTexturesVert(0)
{
	m_renderer = renderer;
	m_surface = surf;
	assert(m_renderer.get());
	assert(m_surface.get());
	_init();
}

void SurfaceTexture::_init() {
	w = m_surface->w;
	h = m_surface->h;
	if(w <= 0 || h <= 0) {
		errors << "SurfaceTexture: invalid surface size" << endl;
		return;
	}
	
	{
		SDL_RendererInfo info;
		if(SDL_GetRendererInfo(m_renderer.get(), &info) == 0) {
			maxTextureWidth = info.max_texture_width;
			maxTextureHeight = info.max_texture_height;
		} else {
			errors << "SurfaceTexture: GetRendererInfo error: " << SDL_GetError() << endl;
			return;
		}
	}

	numTexturesHoriz = (w - 1) / maxTextureWidth + 1;
	numTexturesVert = (h - 1) / maxTextureHeight + 1;
	assert(numTexturesHoriz >= 1 && numTexturesVert >= 1);

	const int numTextures = numTexturesHoriz * numTexturesVert;
	assert(numTextures >= 1);
	m_textures.resize(numTextures);
	for(int i = 0; i < numTextures; ++i) {
		const int horizIdx = i % numTexturesHoriz;
		const int vertIdx = i / numTexturesHoriz;
		
		int textureWidth, textureHeight;
		if(horizIdx < numTexturesHoriz - 1) textureWidth = maxTextureWidth;
		else textureWidth = w % maxTextureWidth;
		if(vertIdx < numTexturesVert - 1) textureHeight = maxTextureHeight;
		else textureHeight = w % maxTextureHeight;
		
		SDL_Texture* texture = SDL_CreateTexture
		(
			m_renderer.get(),
			m_surface->format->format,
			SDL_TEXTUREACCESS_STREAMING,
			textureWidth, textureHeight
		);
		if(!texture) {
			errors << "SurfaceTexture: could not create texture: " << SDL_GetError() << endl;
			return;
		}
		
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		m_textures[i] = texture;
	}
}

SurfaceTexture::~SurfaceTexture() {
	// all are automatically freed via SmartPointer
}

void SurfaceTexture::updateArea(const SDL_Rect* _rect) {
	SDL_Rect rect;
	if(_rect) rect = *_rect;
	else {
		// full area
		rect.x = rect.y = 0;
		rect.w = w;
		rect.h = h;
	}
	
	int vertIdx = rect.y / maxTextureHeight;
	SDL_Rect sub;
	sub.y = rect.y;
	sub.h = (vertIdx + 1) * maxTextureHeight - rect.y;
	while(vertIdx < numTexturesVert) {
		if(sub.y + sub.h > rect.y + rect.h) sub.h = rect.y + rect.h - sub.y;
		if(sub.h <= 0) break;
		
		int horizIdx = rect.x / maxTextureWidth;
		sub.x = rect.x;
		sub.w = (horizIdx + 1) * maxTextureWidth - rect.x;
		while(horizIdx < numTexturesHoriz) {
			if(sub.x + sub.w > rect.x + rect.w) sub.w = rect.x + rect.w - sub.x;
			if(sub.w <= 0) break;
			
			int idx = vertIdx * numTexturesHoriz + horizIdx;
			SDL_Rect srcRect;
			srcRect.x = sub.x - horizIdx * maxTextureWidth;
			srcRect.y = sub.y - vertIdx * maxTextureHeight;
			srcRect.w = sub.w;
			srcRect.h = sub.h;
			SDL_RenderCopy(m_renderer.get(), m_textures[idx].get(), &srcRect, &sub);
			
			horizIdx++;
			sub.x = horizIdx * maxTextureWidth;
			sub.w = maxTextureWidth;
		}
		
		vertIdx++;
		sub.y = vertIdx * maxTextureHeight;
		sub.h = maxTextureHeight;
	}
}

void SurfaceTexture::render(const SDL_Rect * srcrect, const SDL_Rect * dstrect) {
	
}

