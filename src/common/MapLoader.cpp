/*
 *  MapLoader.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 03.05.09.
 *  code under LGPL
 *
 */

#include "MapLoader.h"
#include "PreInitVar.h"
#include "Cache.h"
#include "CMap.h"
#include "EndianSwap.h"
#ifndef DEDICATED_ONLY
#include <gd.h>
#endif
#include <zlib.h>
#include "FindFile.h"
#include "FileUtils.h"


struct ML_OrigLiero : MapLoader {
	static const long Width = 504, Height = 350;
	PIVar(bool,false) Powerlevel;
	
	std::string format() { return "Original Liero"; }
	
	bool parseHeader() {
		// Validate the liero level
		fseek(fp,0,SEEK_END);
		size_t length = ftell(fp);
		
		// check for powerlevel
		if(length != 176400 && length != 176402) {
			if(length == 177178)
				Powerlevel = true;
			else {
				// bad file
				return false;
			}
		}
		
		head.name = GetBaseFilename(filename);
		head.width = Width;
		head.height = Height;
		
		return true;
	}
	
	bool parseData(CMap* m) {
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
		LOCK_OR_FAIL(m->bmpBackImage);
		LOCK_OR_FAIL(m->bmpImage);
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
				
				PutPixel(m->bmpImage.get(),x,y, MakeColour(palette[p*3], palette[p*3+1], palette[p*3+2]));
				if(type == PX_EMPTY)
					PutPixel(m->bmpBackImage.get(),x,y, MakeColour(palette[p*3], palette[p*3+1], palette[p*3+2]));
				m->SetPixelFlag(x,y,type);
				//}
				n++;
			}
		}
		m->unlockFlags();
		UnlockSurface(m->bmpImage);
		UnlockSurface(m->bmpBackImage);
		
		delete[] palette;
		delete[] bytearr;
				
		return true;
	}
	
};


struct ML_LieroX : MapLoader {
	
	std::string id;
	PIVar(int,0) Type;
	std::string Theme_Name;
	PIVar(int,0) numobj;
	PIVar(bool,false) ctf;
	
	std::string format() { return id; }
	
	bool parseHeader() {
		// Header
		id = freadfixedcstr(fp, 32);
		int	version = 0;
		fread_endian<int>(fp, version);
		
		// Check to make sure it's a valid level file
		if((id != "LieroX Level" && id != "LieroX CTF Level") || version != MAP_VERSION) {
			errors << "CMap::Load: " << filename << " is not a valid level file (" << id << ") or wrong version (" << version << ")" << endl;
			return false;
		}
		
		// CTF map?
		ctf = (id == "LieroX CTF Level"); // TODO: there's no CTF maps around, and it was a hack, remove it
		
		head.name = freadfixedcstr(fp, 64);		
		fread_endian<int>(fp, head.width);
		fread_endian<int>(fp, head.height);
		fread_endian<int>(fp, (int&)Type);
		Theme_Name = freadfixedcstr(fp, 32);
		fread_endian<int>(fp, (int&)numobj);
		
		return true;
	}
	
	
	///////////////////
	// Load the image format
	bool LoadImageFormat(CMap* m)
	{
		// Load the details
		Uint32 size = 0, destsize = 0;
		
		fread_compat(size, sizeof(Uint32), 1, fp);
		EndianSwap(size);
		fread_compat(destsize, sizeof(Uint32), 1, fp);
		EndianSwap(destsize);
		
		// Allocate the memory
		uchar *pSource = new uchar[size];
		uchar *pDest = new uchar[destsize];
		
		if(!pSource || !pDest) {
			errors << "CMap::LoadImageFormat: not enough memory" << endl;
			return false;
		}
		
		if(fread(pSource, sizeof(uchar), size, fp) < size) {
			errors << "CMap::LoadImageFormat: cannot read data" << endl;
			return false;
		}
		
		ulong lng_dsize = destsize;
		if( uncompress( pDest, &lng_dsize, pSource, size ) != Z_OK ) {
			errors("Failed decompression\n");
			delete[] pSource;
			delete[] pDest;
			return false;
		}
		destsize = lng_dsize;
		if( destsize < Uint32(head.width * head.height * 3 * 2) )
		{
			errors("CMap::LoadImageFormat(): image too small for Width*Height");
			delete[] pSource;
			delete[] pDest;
			return false;
		}
		
		delete[] pSource;  // not needed anymore
		
		//
		// Translate the data
		//
		
		// Lock surfaces
		LOCK_OR_FAIL(m->bmpBackImage);
		LOCK_OR_FAIL(m->bmpImage);
		
		Uint64 p=0;
		Uint32 curcolor=0;
		Uint8* curpixel = (Uint8*)m->bmpBackImage.get()->pixels;
		Uint8* PixelRow = curpixel;
		
		Uint8 bpp = m->bmpImage.get()->format->BytesPerPixel;
		// Load the back image
		for (Sint64 y = 0; y < head.height; y++, PixelRow += m->bmpBackImage.get()->pitch)  {
			curpixel = PixelRow;
			for (Sint64 x = 0; x < head.width; x++, curpixel += bpp)  {
				curcolor = MakeColour(pDest[p], pDest[p+1], pDest[p+2]);
				p += 3;
				PutPixelToAddr(curpixel, curcolor, bpp);
			}
		}
		
		// Load the front image
		curpixel = (Uint8 *)m->bmpImage.get()->pixels;
		PixelRow = curpixel;
		for (Sint64 y = 0; y < head.height; y++, PixelRow += m->bmpImage.get()->pitch)  {
			curpixel = PixelRow;
			for (Sint64 x = 0;x < head.width; x++, curpixel += bpp)  {
				curcolor = MakeColour(pDest[p], pDest[p+1], pDest[p+2]);
				p += 3;
				PutPixelToAddr(curpixel, curcolor, bpp);
			}
		}
		
		
		// Load the pixel flags and calculate dirt count
		Uint64 n=0;
		m->nTotalDirtCount = 0;
		
		curpixel = (Uint8 *)m->bmpImage.get()->pixels;
		PixelRow = curpixel;
		Uint8 *backpixel = (Uint8 *)m->bmpBackImage.get()->pixels;
		Uint8 *BackPixelRow = backpixel;
		
		m->lockFlags();
		
		for(Sint64 y=0; y< head.height; y++, PixelRow+=m->bmpImage.get()->pitch, BackPixelRow+=m->bmpBackImage.get()->pitch) {
			curpixel = PixelRow;
			backpixel = BackPixelRow;
			for(Sint64 x=0; x<head.width; x++, curpixel+=bpp, backpixel+=bpp) {
				m->PixelFlags[n] = pDest[p++];
				if(m->PixelFlags[n] & PX_EMPTY)
					memcpy(curpixel, backpixel, bpp);
				if(m->PixelFlags[n] & PX_DIRT)
					m->nTotalDirtCount++;
				n++;
			}
		}
		m->unlockFlags();
		
		// Unlock the surfaces
		UnlockSurface(m->bmpBackImage);
		UnlockSurface(m->bmpImage);
		
		// Load the CTF gametype variables
		if (ctf)  {
			warnings << "CMap::LoadImageFormat(): trying to load old-format CTF map, we do not support this anymore" << endl;
			short dummy;
			fread_endian<short>(fp, dummy);
			fread_endian<short>(fp, dummy);
			fread_endian<short>(fp, dummy);
			fread_endian<short>(fp, dummy);
			fread_endian<short>(fp, dummy);
			fread_endian<short>(fp, dummy);
		}
		
		//SDL_SaveBMP(pxf, "mat.bmp");
		//SDL_SaveBMP(bmpImage, "front.bmp");
		//SDL_SaveBMP(bmpBackImage, "back.bmp");
		
		// Delete the data
		delete[] pDest;
		
		// Try to load additional data (like hi-res images)
		LoadAdditionalLevelData(m);
				
		return true;
	}
	
	///////////////////
	// Load the high-resolution images
	void LoadAdditionalLevelData(CMap* m)
	{
		while( !feof(fp) && !ferror(fp) )
		{
			std::string chunkName;
			if(!fread_fixedwidthstr<16>(chunkName, fp)) {
				errors << "CMap::LoadAdditionalLevelData: error while reading" << endl;
				break;
			}
			Uint32 size = 0;
			if(fread_endian<Uint32>(fp, size) == 0) {
				errors << "CMap::LoadAdditionalLevelData: error while reading (attribute " << chunkName << ")" << endl;
				break;
			}
			uchar *pSource = new uchar[size];
			if(pSource == NULL) {
				errors << "CMap::LoadAdditionalLevelData: not enough memory" << endl;
				break;
			}
			if(fread(pSource, sizeof(uchar), size, fp) < size) {
				delete[] pSource;
				errors << "CMap::LoadAdditionalLevelData: error while reading" << endl;
				break;
			}
			
			if( stringcaseequal( chunkName, "OLX hi-res data") )
				LoadLevelImageHiRes( m, pSource, size );
			else
				if( stringcaseequal( chunkName, "OLX level config") )
					LoadLevelConfig( m, pSource, size );
				else
					warnings << "Unknown additional data found in level file: " << chunkName << ", size " << size << endl;
			
			delete [] pSource;
		}
		
	}
	
	
	///////////////////
	// Load level config, such as CTF base spawnpoints
	void LoadLevelConfig(CMap* m, uchar *pSource, Uint32 size)
	{
		warnings << "CMap::LoadLevelConfig(): level config is not used yet in this version of OLX" << endl;
		return;
		
		// TODO: test if this code works
		
		Uint32 destsize = *(Uint32 *)(pSource);
		EndianSwap(destsize);
		
		// Allocate the memory
		uchar *pDest = new uchar[destsize];
		
		ulong lng_dsize = destsize;
		int ret = uncompress( pDest, &lng_dsize, pSource + sizeof(Uint32), size - sizeof(Uint32) );
		if( ret != Z_OK ) 
		{
			warnings << "CMap::LoadLevelConfig(): failed to load hi-res image, using low-res image" << endl;
			lng_dsize = 0;
		}
		destsize = lng_dsize;
		
		// Fill up additional data
		m->AdditionalData.clear();
		Uint32 pos = 0;
		Uint32 AdditionalDataSize = *(Uint32 *)(pDest + pos);
		EndianSwap(AdditionalDataSize);
		pos += 4;
		if( AdditionalDataSize + 4 > destsize )
		{
			warnings << "CMap::LoadLevelConfig(): wrong additional data size " << AdditionalDataSize << endl;
		}
		else
		{
			bool nameChunk = true;
			std::string nameChunkData;
			while( AdditionalDataSize + 4 > pos ) // 4 bytes of whole data size
			{
				Uint32 chunkSize = *(Uint32 *)(pDest + pos);
				EndianSwap(chunkSize);
				if( chunkSize + pos > AdditionalDataSize )
				{
					warnings << "CMap::LoadLevelConfig(): wrong additional data chunk size " << chunkSize << endl;
					break;
				}
				pos += 4;
				
				if( nameChunk )
					nameChunkData = std::string( (char *)(pDest + pos), chunkSize );
				else
					m->AdditionalData[nameChunkData] = std::string( (char *)(pDest + pos), chunkSize );
				
				nameChunk = !nameChunk;
				pos += chunkSize;
			}
		}
		delete[] pDest;
	}
	
	///////////////////
	// Load the high-resolution images
	void LoadLevelImageHiRes(CMap* m, uchar *pSource, Uint32 size)
	{
#ifndef DEDICATED_ONLY // Hi-res images are not needed for dedicated server, it uses only material image which is in low-res data
		
		if(bDedicated) return;
		
		gdImagePtr gdImage = gdImageCreateFromPngPtr( size, pSource );
		
		if( !gdImage || gdImageSX(gdImage) != (int)head.width * 2 || gdImageSY(gdImage) != (int)head.height * 4 )
		{
			warnings << "CMap: hi-res image loading failed" << endl;
			if( gdImage )
				gdImageDestroy(gdImage);
			return;
		}
		
		m->bmpBackImageHiRes = gfxCreateSurface(head.width*2, head.height*2);
		if(m->bmpBackImageHiRes.get() == NULL) 
		{
			warnings << "CMap::LoadImageFormatHiRes(): bmpBackImageHiRes creation failed, using low-res image" << endl;
			gdImageDestroy(gdImage);
			return;
		}
		
		hints << "CMap: Loading high-res level images" << endl;
		// Lock surfaces
		LOCK_OR_QUIT(m->bmpDrawImage);
		LOCK_OR_QUIT(m->bmpBackImageHiRes);
		
		Uint32 curcolor=0;
		Uint8* curpixel = (Uint8*)m->bmpDrawImage.get()->pixels;
		Uint8* PixelRow = curpixel;
		Uint8 bpp = m->bmpDrawImage.get()->format->BytesPerPixel;
		
		// Load the front image
		for (Sint64 y = 0; y < head.height*2; y++, PixelRow += m->bmpDrawImage.get()->pitch)  {
			curpixel = PixelRow;
			for (Sint64 x = 0; x < head.width*2; x++, curpixel += bpp)  {
				curcolor = gdImageGetTrueColorPixel( gdImage, x, y ); // Maybe we can make direct memory access, but PNG may be palette-based, and I'm too lazy
				curcolor = MakeColour(gdTrueColorGetRed(curcolor), gdTrueColorGetGreen(curcolor), gdTrueColorGetBlue(curcolor));
				PutPixelToAddr(curpixel, curcolor, bpp);
			}
		}
		
		curpixel = (Uint8*)m->bmpBackImageHiRes.get()->pixels;
		PixelRow = curpixel;
		Sint64 HeightX2 = head.height*2;
		// Load the back image
		for (Sint64 y = 0; y < head.height*2; y++, PixelRow += m->bmpBackImageHiRes.get()->pitch)  {
			curpixel = PixelRow;
			for (Sint64 x = 0; x < head.width*2; x++, curpixel += bpp)  {
				curcolor = gdImageGetTrueColorPixel( gdImage, x, y + HeightX2 ); // Maybe we can make direct memory access, but PNG may be palette-based, and I'm too lazy
				curcolor = MakeColour(gdTrueColorGetRed(curcolor), gdTrueColorGetGreen(curcolor), gdTrueColorGetBlue(curcolor));
				PutPixelToAddr(curpixel, curcolor, bpp);
			}
		}
		
		// Update image according to the pixel flags
		Uint64 n=0;
		
		curpixel = (Uint8 *)m->bmpDrawImage.get()->pixels;
		PixelRow = curpixel;
		Uint8 *backpixel = (Uint8 *)m->bmpBackImageHiRes.get()->pixels;
		Uint8 *BackPixelRow = backpixel;
		
		m->lockFlags();
		Uint8 bppX2 = bpp*2;
		int pitch = m->bmpDrawImage.get()->pitch;
		for(Sint64 y=0; y<head.height; y++, PixelRow+=pitch*2, BackPixelRow+=pitch*2 ) 
		{
			curpixel = PixelRow;
			backpixel = BackPixelRow;
			for(Sint64 x=0; x<head.width; x++, curpixel+=bppX2, backpixel+=bppX2)
			{
				if(m->PixelFlags[n] & PX_EMPTY)
				{
					memcpy(curpixel, backpixel, bppX2);
					memcpy(curpixel+pitch, backpixel+pitch, bppX2);
				}
				n++;
			}
		}
		m->unlockFlags();
		UnlockSurface(m->bmpBackImageHiRes);
		UnlockSurface(m->bmpDrawImage);
		
		gdImageDestroy(gdImage);
		
#endif // DEDICATED_ONLY
		
	}
	
	
	bool parseData(CMap* m) {
		m->Name = head.name;
		m->Width = head.width;
		m->Height = head.height;
		m->Type = Type;
		const int Width = head.width, Height = head.height;
		
		
		/*
		 notes("Level info:\n");
		 notes("  id = %s\n", id);
		 notes("  version = %i\n", version);
		 notes("  Name = %s\n", Name);
		 notes("  Width = %i\n", Width);
		 notes("  Height = %i\n", Height);
		 notes("  Type = %i\n", Type);
		 notes("  Theme_Name = %s\n", Theme_Name);
		 notes("  numobj = %i\n", numobj);
		 */
		
		// Load the images if in an image format
		if(Type == MPT_IMAGE)
		{
			// Allocate the map
		createMap:
			if(!m->Create(Width, Height, Theme_Name, m->MinimapWidth, m->MinimapHeight)) {
				errors << "CMap::Load (" << filename << "): cannot allocate map" << endl;
				if(cCache.GetEntryCount() > 0) {
					hints << "current cache size is " << cCache.GetCacheSize() << ", we are clearing it now" << endl;
					cCache.Clear();
					goto createMap;
				}
				return false;
			}
			
			// Load the image format
			notes << "CMap::Load: level " << filename << " is in image format" << endl;
			return LoadImageFormat(m);
		} else if (ctf)  {
			errors("pixmap format is not supported for CTF levels\n");
			return false;
		}
		
		
		
		// Create a blank map
		if(!m->New(Width, Height, Theme_Name, m->MinimapWidth, m->MinimapHeight)) {
			errors << "CMap::Load (" << filename << "): cannot create map" << endl;
			return false;
		}
		
		// Lock the surfaces
		LOCK_OR_FAIL(m->bmpImage);
		LOCK_OR_FAIL(m->bmpBackImage);
		
		// Dirt map
		Uint8 *p1 = (Uint8 *)m->bmpImage.get()->pixels;
		Uint8 *p2 = (Uint8 *)m->bmpBackImage.get()->pixels;
		Uint8 *dstrow = p1;
		Uint8 *srcrow = p2;
		
		// Load the bitmask, 1 bit == 1 pixel with a yes/no dirt flag
		uint size = Width*Height/8;
		uchar *bitmask = new uchar[size];
		if (!bitmask)  {
			errors("CMap::Load: Could not create bit mask\n");
			return false;
		}
		if(fread(bitmask,sizeof(uchar),size,fp) < size) {
			errors << "CMap::Load: could not read bitmask" << endl;
			return false;
		}
		
		static const unsigned char mask[] = {1,2,4,8,16,32,64,128};
		
		m->nTotalDirtCount = Width*Height;  // Calculate the dirt count
		
		m->lockFlags();
		
		for(size_t n = 0, i = 0, x = 0; i < size; i++, x += 8) {
			if (x >= (size_t)Width)  {
				srcrow += m->bmpBackImage.get()->pitch;
				dstrow += m->bmpImage.get()->pitch;
				p1 = dstrow;
				p2 = srcrow;
				x = 0;
			}
			
			// 1 bit == 1 pixel with a yes/no dirt flag
			for(size_t j = 0; j < 8;
				j++,
				n++,
				p1 += m->bmpImage.get()->format->BytesPerPixel,
				p2 += m->bmpBackImage.get()->format->BytesPerPixel) {
				
				if(bitmask[i] & mask[j])  {
					m->PixelFlags[n] = PX_EMPTY;
					m->nTotalDirtCount--;
					memcpy(p1, p2, m->bmpImage.get()->format->BytesPerPixel);
				}
			}
		}
		
		m->unlockFlags();
		
		delete[] bitmask;
		
		// Unlock the surfaces
		UnlockSurface(m->bmpImage);
		UnlockSurface(m->bmpBackImage);
		
		// Objects
		object_t o;
		m->NumObjects = 0;
		for(int i = 0; i < numobj; i++) {
			fread_compat(o.Type,	sizeof(int),	1,	fp);
			EndianSwap(o.Type);
			fread_compat(o.Size,	sizeof(int),	1,	fp);
			EndianSwap(o.Size);
			fread_compat(o.X,	    sizeof(int),	1,	fp);
			EndianSwap(o.X);
			fread_compat(o.Y,	    sizeof(int),	1,	fp);
			EndianSwap(o.Y);
			
			// Place the object
			if(o.Type == OBJ_STONE)
				m->PlaceStone(o.Size, CVec((float)o.X, (float)o.Y));
			else if(o.Type == OBJ_MISC)
				m->PlaceMisc(o.Size, CVec((float)o.X, (float)o.Y));
		}
		
		return true;
	}
};


MapLoader* MapLoader::open(const std::string& filename, bool abs_filename) {
	FILE* fp = abs_filename ? fopen(filename.c_str(), "rb") : OpenGameFile(filename, "rb");
	if(fp == NULL) {
		warnings << "level " << filename << " does not exist" << endl;
		return NULL;
	}
	
	if( stringcasecmp(GetFileExtension(filename), "lev") == 0 )
		return (new ML_OrigLiero()) -> Set(filename, fp) -> parseHeaderAndCheck();
		
	MapLoader* loader = (new ML_LieroX()) -> Set(filename, fp) -> parseHeaderAndCheck();
	if(loader) return loader;
	
	// HINT: Other level formats could be added here
	
	return NULL;
}

