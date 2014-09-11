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
		w = h = 0;
		return;
	}
	
	{
		SDL_RendererInfo info;
		if(SDL_GetRendererInfo(m_renderer.get(), &info) == 0) {
			maxTextureWidth = info.max_texture_width;
			maxTextureHeight = info.max_texture_height;
		} else {
			errors << "SurfaceTexture: GetRendererInfo error: " << SDL_GetError() << endl;
			w = h = 0;
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
			w = h = 0;
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
	if(w <= 0) return; // not correctly initialized
	
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
			SDL_Rect texRect;
			texRect.x = sub.x - horizIdx * maxTextureWidth;
			texRect.y = sub.y - vertIdx * maxTextureHeight;
			texRect.w = sub.w;
			texRect.h = sub.h;
			uint8_t* pixels =
				(uint8_t*) m_surface->pixels
				+ sub.y * m_surface->pitch
				+ sub.x * m_surface->format->BytesPerPixel;
			SDL_UpdateTexture(m_textures[idx].get(), &texRect, pixels, m_surface->pitch);
			
			horizIdx++;
			sub.x = horizIdx * maxTextureWidth;
			sub.w = maxTextureWidth;
		}
		
		vertIdx++;
		sub.y = vertIdx * maxTextureHeight;
		sub.h = maxTextureHeight;
	}
}

void SurfaceTexture::render(const SDL_Rect* _srcrect, const SDL_Rect* _dstrect) {
	if(w <= 0) return; // not correctly initialized

	SDL_Rect rect;
	if(_srcrect) rect = *_srcrect;
	else {
		// full area
		rect.x = rect.y = 0;
		rect.w = w;
		rect.h = h;
	}
	SDL_Rect dstrect;
	if(_dstrect) dstrect = *_dstrect;
	else {
		// full area
		dstrect.x = dstrect.y = 0;
		dstrect.w = w;
		dstrect.h = h;
	}
	
	float scaleX = float(dstrect.w) / rect.w;
	float scaleY = float(dstrect.h) / rect.h;
	
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
			SDL_Rect rendSrcRect;
			rendSrcRect.x = sub.x - horizIdx * maxTextureWidth;
			rendSrcRect.y = sub.y - vertIdx * maxTextureHeight;
			rendSrcRect.w = sub.w;
			rendSrcRect.h = sub.h;
			SDL_Rect rendDstRect;
			rendDstRect.x = dstrect.x + int((rect.x - sub.x) * scaleX);
			rendDstRect.y = dstrect.y + int((rect.y - sub.y) * scaleY);
			rendDstRect.w = int(sub.w * scaleX);
			rendDstRect.h = int(sub.h * scaleY);			
			SDL_RenderCopy(m_renderer.get(), m_textures[idx].get(), &rendSrcRect, &rendDstRect);
			
			horizIdx++;
			sub.x = horizIdx * maxTextureWidth;
			sub.w = maxTextureWidth;
		}
		
		vertIdx++;
		sub.y = vertIdx * maxTextureHeight;
		sub.h = maxTextureHeight;
	}
}

