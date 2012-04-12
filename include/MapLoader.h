/*
 *  MapLoad.h
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
#include "SmartPointer.h"
#include "util/Result.h"
class CMap;

struct MapHeader {
	MapHeader() : width(-1), height(-1) {}
	std::string name;
	Sint64 width, height;
};

class MapLoad {
protected:
	std::string filename;
	bool abs_filename;
	FILE* fp;
	MapHeader head;
	SmartPointer<SDL_Surface> minimap;
	
public:
	// determines level format and creates the specific maploader
	static MapLoad* open(const std::string& filename, bool abs_filename = false, bool printErrors = true);
	
	MapLoad() : fp(NULL) {}
	MapLoad* Set(const std::string& fn, bool absfn, FILE* f) { filename = fn; abs_filename = absfn; fp = f; return this; }
	virtual ~MapLoad() { if(fp) fclose(fp); fp = NULL; }
	virtual std::string format() = 0;
	virtual std::string formatShort() = 0;
	const MapHeader& header() { return head; }
	
	virtual Result parseHeader(bool printErrors = true) = 0;
	virtual Result parseData(CMap* m) = 0; // this assumes that the header was already parsed successfully
	virtual SmartPointer<SDL_Surface> getMinimap(); // in some cases, this can be done independent from data parseData and much faster. this also assumes that the header was already parsed successfully
	
	MapLoad* parseHeaderAndCheck(bool printErrors = true) {
		if(parseHeader(printErrors)) return this;
		delete this;
		return NULL;
	}
	
	void parseDataFinalize(CMap* m);
};

#endif
