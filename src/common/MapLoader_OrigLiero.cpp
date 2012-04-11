/*
 *  MapLoader_OrigLiero.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 11.4.12.
 *  code under LGPL
 *
 */

#include "MapLoader.h"
#include "MapLoader_common.h"
#include "PreInitVar.h"
#include "FindFile.h"
#include "game/CMap.h"
#include "Cache.h"
#include "StringUtils.h"
#include "Color.h"
#include "FileUtils.h"
#include "PixelFunctors.h"

class ML_OrigLiero : public MapLoad {
public:
	static const long Width = 504, Height = 350;
	PIVar(bool,false) Powerlevel;
	
	std::string format() { return "Original Liero"; }
	std::string formatShort() { return "Liero"; }
	
	bool parseHeader(bool printErrors) {
		// Validate the liero level
		fseek(fp,0,SEEK_END);
		size_t length = ftell(fp);
		
		// check for powerlevel
		if(length != 176400 && length != 176402) {
			if(length == 177178)
				Powerlevel = true;
			else {
				// bad file
				if(printErrors) errors << "OrigLiero loader: bad file: " << filename << endl;
				return false;
			}
		}
		
		head.name = GetBaseFilename(filename);
		head.width = Width;
		head.height = Height;
		
		return true;
	}
	
	bool parseData(CMap* m) {
		ParseFinalizer finalizer(this, m);
		
		fseek(fp,0,SEEK_SET);
		
		// Default is a dirt theme for the background & dirtballs
		if( !m->New(504,350,"dirt") ) {
			return false;
		}
		
		// Image type of map
		m->Type = MPT_IMAGE;
		
		uchar *palette = new uchar[768];
		if( palette == NULL) {
			errors << "CMap::LoadOriginal: ERROR: not enough memory for palette" << endl;
			return false;
		}
		
		// Load the palette
		if(!Powerlevel) {
			FILE *fpal = OpenGameFile("data/lieropal.act","rb");
			if(!fpal) {
				return false;
			}
			
			if(fread(palette,sizeof(uchar),768,fpal) < 768) {
				return false;
			}
			fclose(fpal);
		}
		
		// Load the image map
	imageMapCreate:
		uchar *bytearr = new uchar[Width*Height];
		if(bytearr == NULL) {
			errors << "CMap::LoadOriginal: ERROR: not enough memory for bytearr" << endl;
			if(cCache.GetEntryCount() > 0) {
				hints << "current cache size is " << cCache.GetCacheSize() << ", we are clearing it now" << endl;
				cCache.Clear();
				goto imageMapCreate;
			}
			delete[] palette;
			return false;
		}
		
		if(fread(bytearr,sizeof(uchar),Width*Height,fp) < Width*Height) {
			errors << "CMap::LoadOriginal: cannot read file" << endl;
			return false;
		}
		
		// Load the palette from the same file if it's a powerlevel
		if(Powerlevel) {
			std::string id;
			// Load id
			fread_fixedwidthstr<10>(id,fp);
			if(!stringcaseequal(id,"POWERLEVEL")) {
				delete[] palette;
				delete[] bytearr;
				return false;
			}
			
			// Load the palette
			if(fread(palette,sizeof(uchar),768,fp) < 768) {
				return false;
			}
			
			// Convert the 6bit colours to 8bit colours
			for(short n=0;n<768;n++) {
				float f = (float)palette[n] / 63.0f * 255.0f;
				palette[n] = (int)f;
			}
		}
		
		// Set the image
		LOCK_OR_FAIL(m->bmpBackImageHiRes);
		LOCK_OR_FAIL(m->bmpDrawImage);
		m->lockFlags();
		uint n=0;
		for(long y=0;y<Height;y++) {
			for(long x=0;x<Width;x++) {
				uchar p = bytearr[n];
				uchar type = PX_EMPTY;
				//if(p >= 0 && p <= 255) {
				
				// Dirt
				if( (p >= 12 && p <= 18) ||
				   (p >= 55 && p <= 58) ||
				   (p >= 82 && p <= 84) ||
				   (p >= 94 && p <= 103) ||
				   (p >= 120 && p <= 123) ||
				   (p >= 176 && p <= 180))
					type = PX_DIRT;
				
				// Rock
				else if( (p >= 19 && p <= 29) ||
						(p >= 59 && p <= 61) ||
						(p >= 85 && p <= 87) ||
						(p >= 91 && p <= 93) ||
						(p >= 123 && p <= 125) ||
						p==104)
					type = PX_ROCK;
				
				PutPixel2x2(m->bmpDrawImage.get(),x*2,y*2, Pack(Color(palette[p*3], palette[p*3+1], palette[p*3+2]), m->bmpDrawImage->format));
				if(type == PX_EMPTY)
					PutPixel2x2(m->bmpBackImageHiRes.get(),x*2,y*2, Pack(Color(palette[p*3], palette[p*3+1], palette[p*3+2]), m->bmpBackImageHiRes->format));
				m->SetPixelFlag(x,y,type);
				//}
				n++;
			}
		}
		m->unlockFlags();
		UnlockSurface(m->bmpDrawImage);
		UnlockSurface(m->bmpBackImageHiRes);
		
		delete[] palette;
		delete[] bytearr;
		
		m->lxflagsToGusflags();
		return true;
	}
	
};


MapLoad* createMapLoad_OrigLiero() {
	return new ML_OrigLiero();
}
