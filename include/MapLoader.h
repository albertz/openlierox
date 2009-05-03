/*
 *  MapLoader.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 03.05.09.
 *  code under LGPL
 *
 */

#ifndef __OLX__MAPLOADER_H__
#define __OLX__MAPLOADER_H__

#include <string>
#include <cstdio>
#include <SDL.h>
class CMap;

struct MapHeader {
	MapHeader() : width(-1), height(-1) {}
	std::string name;
	Sint64 width, height;
};

class MapLoader {
protected:
	std::string filename;
	FILE* fp;
	MapHeader head;
	
public:
	// determines level format and creates the specific maploader
	static MapLoader* open(const std::string& filename, bool abs_filename = false);
	
	MapLoader() : fp(NULL) {}
	MapLoader* Set(const std::string& fn, FILE* f) { filename = fn; fp = f; return this; }
	virtual ~MapLoader() { if(fp) fclose(fp); fp = NULL; }
	virtual std::string format() = 0;
	const MapHeader& header() { return head; }
	
	virtual bool parseHeader() = 0;
	virtual bool parseData(CMap* m) = 0;
	
	MapLoader* parseHeaderAndCheck() {
		if(parseHeader()) return this;
		delete this;
		return NULL;
	}
	
};

#endif
