/*
 *  MapLoader_Gusanos.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 11.4.12.
 *  code under LGPL
 *
 */

#include <string>
#include "MapLoader.h"
#include "MapLoader_common.h"
#include "gusanos/loaders/gusanos.h"
#include "gusanos/level.h"
#include "gusanos/gusgame.h"
#include "game/CMap.h"
#include "Cache.h"


struct ML_Gusanos : public MapLoad {
public:
	CMap* curMap;
	ResourceLocator<CMap>::BaseLoader* loader;
	ML_Gusanos(ResourceLocator<CMap>::BaseLoader* l, const std::string& name) : curMap(NULL), loader(l) { head.name = name; }
	
	std::string format() { return loader->format(); }
	std::string formatShort() { return loader->formatShort(); }
	
	virtual Result parseHeader(bool printErrors) {
		return true;
	}
	
	SmartPointer<SDL_Surface> getMinimap() {
		if(minimap.get()) return minimap;
		
		// We use the allegro loader function because that assures that we don't have alpha - because alpha makes problem for the blitting functions.
		SmartPointer<SDL_Surface> image = load_bitmap__allegroformat(filename + "/level.png", false);
		if(!image.get()) {
			setMinimapErrorGraphic(minimap);
			return minimap;
		}		
		SetColorKey(image.get());
		SmartPointer<SDL_Surface> foreground = LoadGameImage(filename + "/foreground.png", true);
		SmartPointer<SDL_Surface> paralax = load_bitmap__allegroformat(filename + "/paralax.png", false);
		
		const int w = image->w, h = image->h;		
		bool doubleRes = false;
		float resFactor = doubleRes ? 0.5f : 1.0f;
		minimap = gfxCreateSurface(128, 96);
		CMap::gusUpdateMinimap(minimap, foreground, image, paralax, 0, 0, w, h, resFactor);
		
		return minimap;
	}	
	
	virtual Result parseData(CMap* m) {
		ParseFinalizer finalizer(this, m);
		
		curMap = m;
		m->Shutdown();
		
		std::string f = filename;
		notes << "Gusanos level loader: using " << loader->getName() << " for " << f << endl;
		
		// TODO: abs filename
		if(!gusGame.changeLevel(loader, f, m))
			return false;
		
		m->Name = head.name;
		
		// Allocate the map
	createMap:
		if(!m->MiniNew(m->material->w, m->material->h)) {
			errors << "Gus lvl loader (" << filename << "): cannot allocate map" << endl;
			if(cCache.GetEntryCount() > 0) {
				hints << "current cache size is " << cCache.GetCacheSize() << ", we are clearing it now" << endl;
				cCache.Clear();
				goto createMap;
			}
			return false;
		}		
		
		if(!m->material || !m->bmpDrawImage.get())
			return false;
		
		SetColorKey(m->bmpDrawImage.get());
		
		curMap = NULL;
		return true;
	}
};

MapLoad* createMapLoad_Gusanos(ResourceLocator<CMap>::BaseLoader* l, const std::string& name) {
	return new ML_Gusanos(l, name);
}
