/*
 *  MapLoad.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 03.05.09.
 *  code under LGPL
 *
 */

#include "MapLoader.h"
#include "MapLoader_common.h"
#include "game/CMap.h"
#include "gusanos/loaders/gusanos.h" // for ...::canLoad
#include "FindFile.h"


void CMap::lxflagsToGusflags() {
	if(!image)
		image = create_bitmap_from_sdl(bmpDrawImage);
	if(!background)
		background = create_bitmap_from_sdl(bmpBackImageHiRes);

	loaderSucceeded();
}


static void setMinimap(CMap* m, SmartPointer<SDL_Surface>& minimap) {
	if(m->GetMiniMap().get()) {
		minimap = m->GetMiniMap();
	} else
		setMinimapErrorGraphic(minimap);
}

void MapLoad::parseDataFinalize(CMap* m) {
	if(m->GetMiniMap().get() && m->image)
		m->UpdateMiniMap(true);
	setMinimap(m, minimap);
}




MapLoad* createMapLoad_OrigLiero();
MapLoad* createMapLoad_LieroX();
MapLoad* createMapLoad_CK123();
MapLoad* createMapLoad_Gusanos(ResourceLocator<CMap>::BaseLoader* l, const std::string& name);
MapLoad* createMapLoad_Teeworlds();


MapLoad* MapLoad::open(const std::string& filename, bool abs_filename, bool printErrors) {
	
	if(IsDirectory(filename, abs_filename)) {
		// TODO: abs filename
		std::string name;
		if(GusanosLevelLoader::instance.canLoad(filename, name))
			return (createMapLoad_Gusanos(&GusanosLevelLoader::instance, name)) -> Set(filename, abs_filename, NULL) -> parseHeaderAndCheck(printErrors);;
	}
	else { // regular file
		FILE *fp = abs_filename ? OpenAbsFile(filename, "rb") : OpenGameFile(filename, "rb");
		if(fp == NULL) {
			if(printErrors) errors << "level " << filename << " does not exist" << endl;
			return NULL;
		}

		std::string fileext = GetFileExtension(filename); stringlwr(fileext);
		
		if( fileext == "lxl" )
			return (createMapLoad_LieroX()) -> Set(filename, abs_filename, fp) -> parseHeaderAndCheck(printErrors);;
		
		if( fileext == "lev" )
			return (createMapLoad_OrigLiero()) -> Set(filename, abs_filename, fp) -> parseHeaderAndCheck(printErrors);
			
		if( fileext == "ck1" || fileext == "ck2" || fileext == "ck3" )
			return (createMapLoad_CK123()) -> Set(filename, abs_filename, fp) -> parseHeaderAndCheck(printErrors);

		if( fileext == "map" )
			return (createMapLoad_Teeworlds()) -> Set(filename, abs_filename, fp) -> parseHeaderAndCheck(printErrors);
	}
	
	if(printErrors) errors << "level format of file " << filename << " unknown" << endl;
	return NULL;
}

SmartPointer<SDL_Surface> MapLoad::getMinimap() {
	if(minimap.get()) return minimap;

	// Stupid and dumb way but we cannot do better in general.
	// parseData should set the minimap.
	
	// In special cases (for example for Gusanos) where it is possible to do easier,
	// we have overloaded this getMinimap function.
	// In case for LieroX for example, it doesn't really make sense. We could add
	// lazyness behaviour to shadows+addition stuff so that not needed stuff
	// is not calculated. That would be a much cleaner way instead of coding
	// a special handler for LieroX maps.

	CMap map;
	if(map.LoadFromCache(filename)) {
		setMinimap(&map, minimap);
		
	} else {
		// TODO: remove that as soon as we do the map loading in a seperate thread
		ScopedBackgroundLoadingAni backgroundLoadingAni(320, 280, 50, 50, Color(128,128,128), Color(64,64,64));

		// parseData should set minimap
		parseData(&map);
	}
	
	return minimap;
}

