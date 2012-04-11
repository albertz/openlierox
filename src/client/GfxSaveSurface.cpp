/*
 *  GfxSaveSurface.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 03.12.09.
 *  code under LGPL
 *
 */

#include <SDL.h>
#include "GfxPrimitives.h"
#include "Options.h"
#include "FindFile.h"

#ifndef DEDICATED_ONLY
#include <gd.h>
#endif


#ifndef DEDICATED_ONLY
///////////////////////
// Converts the SDL_surface to gdImagePtr
static gdImagePtr SDLSurface2GDImage(SDL_Surface* src) {
	if(src->format->BitsPerPixel == 8) {
		gdImagePtr gd_image = gdImageCreatePalette(src->w, src->h);
		if(!gd_image) return NULL;
		// we must allocate 255 colors in the palette so that it becomes an 8bit gdImage
		for(int i = 0; i < 255; ++i)
			gdImageColorAllocate(gd_image, i, i, i);
		LockSurface(src);
		for(int y = 0; y < src->h; ++y) {
			for(int x = 0; x < src->w; ++x)
				gd_image->pixels[y][x] = GetPixel(src, x, y);
		}
		UnlockSurface(src);
		return gd_image;
	}

	gdImagePtr gd_image = gdImageCreateTrueColor(src->w,src->h);
	if(!gd_image)
		return NULL;
	
	Uint32 rmask, gmask, bmask;
	// format of gdImage
	rmask=0x00FF0000; gmask=0x0000FF00; bmask=0x000000FF;
	
	SmartPointer<SDL_Surface> formated = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, rmask, gmask, bmask, 0);
	if(!formated.get())
		return NULL;
#ifdef DEBUG
	//printf("SDLSurface2GDImage() %p\n", formated.get() );
#endif
	// convert it to the new format (32 bpp)
	CopySurface(formated.get(), src, 0, 0, 0, 0, src->w, src->h);
	
	if (!LockSurface(formated))
		return NULL;
	
	for(int y = 0; y < src->h; y++) {
		memcpy(gd_image->tpixels[y], (uchar*)formated.get()->pixels + y*formated.get()->pitch, formated.get()->pitch);
	}
	
	UnlockSurface(formated);
	
	return gd_image;
}
#endif //DEDICATED_ONLY

///////////////////////
// Saves the surface into the specified file with the specified format
bool SaveSurface(SDL_Surface * image, const std::string& FileName, int Format, const std::string& Data)
{
	//
	// BMP
	//
	
	// We use standard SDL function for saving BMPs
	if (Format == FMT_BMP)  {
		
		// Save the image
		std::string abs_fn = GetWriteFullFileName (FileName, true);  // SDL requires full paths
		SDL_SaveBMP(image, abs_fn.c_str());
		
		// Append any additional data
		if (!Data.empty())  {
			
			FILE *f = OpenGameFile (FileName, "ab");
			if (!f)
				return false;
			
			fwrite(Data.data(), 1, Data.size(), f);
			fclose (f);
		}
		
		return true;
	}
	
#ifdef DEDICATED_ONLY
	warnings << "SaveSurface: cannot use something else than BMP in dedicated-only-mode" << endl;
	return false;
	
#else
	
	//
	// JPG, PNG, GIF
	//
	
	// We use GD for saving these formats
	gdImagePtr gd_image = NULL;
	
	// Convert the surface
	gd_image = SDLSurface2GDImage ( image );
	if ( !gd_image )
		return false;
	
	// Save the image
	int s;
	char *data = NULL;
	FILE *out = OpenGameFile (FileName, "wb");
	if ( !out )
		return false;
	
	// Get the data depending on the format
	switch (Format)
	{
		case FMT_PNG:
			data = ( char * ) gdImagePngPtr ( gd_image, &s );
			break;
		case FMT_JPG:
			data = ( char * ) gdImageJpegPtr ( gd_image, &s,tLXOptions->iJpegQuality );
			break;
		case FMT_GIF:
			data = ( char * ) gdImageGifPtr ( gd_image, &s );
			break;
		default:
			data = ( char * ) gdImagePngPtr ( gd_image, &s );
			break;
	}
	
	// Check
	if (!data)
		return false;
	
	// Size of the data
	size_t size = s > 0 ? s : -s;
	
	// Write the image data
	if (fwrite(data, 1, size, out) != size)
		return false;
	
	// Write any additional data
	if (!Data.empty())
		fwrite(Data.data(), 1, Data.size(), out);
	
	// Free everything
	gdFree ( data );
	gdImageDestroy ( gd_image );
	
	// Close the file and quit
	return fclose(out) == 0;
#endif // !DEDICATED_ONLY
}


