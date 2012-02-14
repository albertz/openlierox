/*
 *  raytracing.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.10.
 *  code under GPL
 *
 */

#include <boost/bind.hpp>
#include "GfxPrimitives.h"
#include "raytracing.h"
#include "util/macros.h"
#include "game/Game.h"
#include "game/CMap.h"
#include "SafeVector.h"
#include "game/CWorm.h"
#include "Geometry.h"


/* some hacky code for projectiles and other LX only objects */
struct ObjectMap {
	SafeVector<CGameObject*> objects;
	AbsTime lastUpdate;
	
	void set(int x, int y, CGameObject* obj) {
		int width = game.gameMap()->GetWidth();
		if(x >= 0 && x < width && y >= 0 && (unsigned int)y < game.gameMap()->GetHeight())
			*objects[y * width + x] = obj;
	}
	
	void update() {
		lastUpdate = tLX->currentTime;
		int width = game.gameMap()->GetWidth();
		size_t size = width * game.gameMap()->GetHeight();
		if(objects.size() != size)
			objects.resize(size);
		else
			memset(objects[0], 0, objects.size() * sizeof(void*));
		
		for(Iterator<CProjectile*>::Ref i = cClient->getProjectiles().begin(); i->isValid(); i->next()) {
			const int y = (int)i->get()->pos().get().y - i->get()->size().y / 2;
			const int x = (int)i->get()->pos().get().x - i->get()->size().x / 2;
			for(int dy = 0; dy < i->get()->size().y; ++dy)
				for(int dx = 0; dx < i->get()->size().x; ++dx)
					set(x + dx, y + dy, i->get());
		}
		
		for_each_iterator(CWorm*, w, game.worms()) {
			if(w->get()->getNinjaRope()->isReleased()) {
				Line l(w->get()->pos(), w->get()->getNinjaRope()->pos());
				l.forEachPoint( boost::bind(&ObjectMap::set, this, _1, _2, w->get()->getNinjaRope()) );
			}
		}
	}
	
	void updateIfNeccessary() {
		if(tLX->currentTime > lastUpdate)
			update();		
	}
	
	CGameObject* at(int x, int y) {
		const int width = game.gameMap()->GetWidth();
		const int height = game.gameMap()->GetHeight();
		if(x < 0 || x >= width || y < 0 || y >= height) return NULL;
		const size_t size = width * height;
		if(objects.size() == size)
			return *objects[y * width + x];
		return NULL;
	}
	
};

static ObjectMap objectMap;


static GamePixelInfo getGamePixelInfo_MapOnly(int x, int y) {
	GamePixelInfo info;
	info.type = GamePixelInfo::GPI_Material;
	info.material = game.gameMap()->getMaterialIndex(x, y);

	{
		SDL_Surface* image = game.gameMap()->image->surf.get();
		if(LockSurface(image)) {
			Uint32 col = GetPixel(image, x, y);
			info.color = Color(image->format, col);
			UnlockSurface(image);
			if(IsTransparent(image, col)) info.color.a = SDL_ALPHA_TRANSPARENT;
		}
	}
	
	if(info.color.a != SDL_ALPHA_OPAQUE) {
		SDL_Surface* paralax = game.gameMap()->paralax->surf.get();
		if(LockSurface(paralax)) {
			// hardcoded w/h because we have no other option here
			const int w = 320;
			const int h = 240;
			int px = int(x * (paralax->w - w) / float( game.gameMap()->material->w - w ));
			int py = int(y * (paralax->h - h) / float( game.gameMap()->material->h - h ));
			// we cannot be sure that we have them valid, so do these extra checks
			if(px < 0) px = 0;
			else if(px >= paralax->w) px = paralax->w - 1;
			if(py < 0) py = 0;
			else if(py >= paralax->h) py = paralax->h - 1;
			
			// TODO: real alpha blending
			info.color = Color(paralax->format, GetPixel(paralax, px, py));
			UnlockSurface(paralax);
		}	
	}
	
	return info;
}

static GamePixelInfo infoForObject(CGameObject* object, int x, int y) {
	GamePixelInfo info;
	info.type = GamePixelInfo::GPI_Object;
	info.object.obj = &*object;
	info.object.relX = x - (int)object->pos().get().x;
	info.object.relY = y - (int)object->pos().get().y;
	
	info.color = info.object.obj->renderColorAt(info.object.relX, info.object.relY);
	if(info.color.a != SDL_ALPHA_OPAQUE)
		// TODO: real alpha blending
		info.color = getGamePixelInfo_MapOnly(x, y).color;
	
	return info;	
}

// basically, this is CClient::DrawViewport_Game backwards
GamePixelInfo getGamePixelInfo(int x, int y) {
	objectMap.updateIfNeccessary();
	CVec p((float)x, (float)y);
	
	if(CGameObject* object = objectMap.at(x, y))
		return infoForObject(object, x, y);
	
	forrange_bool(object, game.objects.beginArea(x, y, x+1, y+1, /* layer */ 0)) {
		if(object->isInside(x, y))
			return infoForObject(&*object, x, y);
	}

	return getGamePixelInfo_MapOnly(x, y);
}

Color getGamePixelColor(int x, int y) {
	return getGamePixelInfo(x, y).color;
}
