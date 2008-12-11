// Level Maker v1.1
// For Liero Xtreme
// By Jason Boettcher
// 29/7/03


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../corona.h"
#include "../zlib/zlib.h"

//template<class PIXEL>
typedef unsigned long PIXEL;
void ScaleRectAvg(PIXEL *Target, PIXEL *Source, int SrcWidth, int SrcHeight, int TgtWidth, int TgtHeight);

typedef unsigned long ulong;
typedef unsigned char uchar;

void BlitSurface(ulong *dest, int destwidth, ulong *src, int srcwidth, int dx, int dy, int sx, int sy, int w, int h);


#define		PX_EMPTY	0x01
#define		PX_DIRT		0x02
#define		PX_ROCK		0x04
#define		MAP_VERSION	0
#define		MPT_IMAGE	1

void	Shutdown(void);
ulong	GetPixel(corona::Image *pcImage, int x, int y);
void	Fill(corona::Image *dest, ulong nColour);

char	szOutFile[_MAX_PATH];
char	szFrontFile[_MAX_PATH];
char	szBackFile[_MAX_PATH];
char	szMaterialFile[_MAX_PATH];
char	Option = '\0';

corona::Image *pcFrontImage = NULL;
corona::Image *pcBackImage = NULL;
corona::Image *pcMaterialImage = NULL;


///////////////////
// Load the files
int loadFiles(HWND hWnd, char *szFrontFile, char *szBackFile, char *szMaterialFile)
{
	pcFrontImage = corona::OpenImage(szFrontFile,corona::FF_AUTODETECT,corona::PF_R8G8B8);
	pcBackImage = corona::OpenImage(szBackFile,corona::FF_AUTODETECT,corona::PF_R8G8B8);
	pcMaterialImage = corona::OpenImage(szMaterialFile,corona::FF_AUTODETECT,corona::PF_R8G8B8);

	// Check if they loaded
	if( !pcFrontImage ) {
		MessageBox(hWnd, "Error opening front image", "Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	if( !pcBackImage ) {
		MessageBox(hWnd, "Error opening back image", "Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	if( !pcMaterialImage ) {
		MessageBox(hWnd, "Error opening material image", "Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}


	// Check that sizes match
	int Width = pcFrontImage->getWidth();
	int Height = pcFrontImage->getHeight();

	if(pcBackImage->getWidth() != Width || pcBackImage->getHeight() != Height) {        
        MessageBox(hWnd, "Image sizes do not match", "Error", MB_OK | MB_ICONEXCLAMATION);
		Shutdown();
		return false;
	}

	if(pcMaterialImage->getWidth() != Width || pcMaterialImage->getHeight() != Height) {
		MessageBox(hWnd, "Image sizes do not match", "Error", MB_OK | MB_ICONEXCLAMATION);
		Shutdown();
		return false;
	}

	return true;
}


///////////////////
// Write out the map file
int writeMap(HWND hWnd, char *szOutFile, char *szName)
{
	int n,p, back, front;
	int x,y;
	uchar r,g,b;
    char buf[_MAX_PATH];

	FILE *fp = fopen(szOutFile, "wb");
	if(!fp) {
		sprintf(buf, "Could not open %s for writing\n",szOutFile);
        MessageBox(hWnd, buf, "Error", MB_OK | MB_ICONEXCLAMATION);
		Shutdown();
		return false;
	}


	char	id[32];
	int		version = MAP_VERSION;
	char	theme[32];
	char	name[64];
	int		Type = MPT_IMAGE;
	int		Width = pcFrontImage->getWidth();
	int		Height = pcFrontImage->getHeight();

	strcpy(id,"LieroX Level");
	strcpy(name, szName);
	
	strcpy(theme,"dirt");
	int NumObjects = 0;

	fwrite(id,			sizeof(char),	32,	fp);
	fwrite(&version,	sizeof(int),	1,	fp);
	fwrite(name,		sizeof(char),	64,	fp);
	fwrite(&Width,		sizeof(int),	1,	fp);
	fwrite(&Height,		sizeof(int),	1,	fp);
	fwrite(&Type,		sizeof(int),	1,	fp);
	fwrite(theme,		sizeof(char),	32,	fp);
	fwrite(&NumObjects,	sizeof(int),	1,	fp);

	// The images are saved in a raw 24bit format.
	// 8 bits per r,g,b channel

	ulong size = (Width*Height * 3) * 2 + (Width*Height) + 1;
	ulong destsize = size + (size / 8) + 12;

	uchar *pSource = new uchar[size];
	uchar *pDest = new uchar[destsize];

	if(!pSource || !pDest) {
		MessageBox(hWnd, "Out of memory", "Error", MB_OK | MB_ICONEXCLAMATION);
		fclose(fp);
		Shutdown();
		return false;
	}

	// Save the back & front images
	memcpy( pSource, pcBackImage->getPixels(), Width*Height*3);
	memcpy( pSource+(Width*Height*3), pcFrontImage->getPixels(), Width*Height*3);

	p = (Width*Height*3) * 2;
	back = 0;
	front = (Width*Height*3);

	// Save the pixel flags		
	uchar *pixels = (uchar *)pcMaterialImage->getPixels();
	for(n=0;n<Width*Height*3;) {
		uchar t = PX_EMPTY;

		r = pixels[n++];
		g = pixels[n++];
		b = pixels[n++];

		if(r==0 && g==0 && b==0)
			t = PX_EMPTY;
		if(r==255 && g==255 && b==255)
			t = PX_DIRT;
		if(r==128 && g==128 && b==128)
			t = PX_ROCK;

		// Optimization
		if(t == PX_ROCK) {
			pSource[back++] = 0;
			pSource[back++] = 0;
			pSource[back++] = 0;
		} else
			back+=3;
		if(t == PX_EMPTY) {
			pSource[front++] = 0;
			pSource[front++] = 0;
			pSource[front++] = 0;
		} else
			front+=3;
		

		pSource[p++] = t;
	}

	// Compress it
	if( compress( pDest, &destsize, pSource, size) != Z_OK ) {
		MessageBox(hWnd, "Failed compressing", "Error", MB_OK | MB_ICONEXCLAMATION);
		fclose(fp);
		delete[] pSource;
		delete[] pDest;
		return false;
	}
	
	// Write out the details & the data
	fwrite(&destsize, sizeof(ulong), 1, fp);
	fwrite(&size, sizeof(ulong), 1, fp);
	fwrite(pDest, sizeof(uchar), destsize, fp);

	delete[] pSource;
	delete[] pDest;

	fclose(fp);
	return true;
}


/*-------------------------------
 * Create a preview image
 *-------------------------------*/
int createPreview(HWND hWnd, char *szPreviewFile)
{
	// Filename not required. Just don't create the preview if it doesn't exist
	if(strlen(szPreviewFile) == 0)
		return true;

	int nPrevWidth = 160;
	int nPrevHeight = 120;

	int nCompleteWidth = pcFrontImage->getWidth();
	int nCompleteHeight = pcFrontImage->getHeight();

	// Create the final image
	corona::Image *pcCompleteImage = corona::CreateImage(nCompleteWidth, nCompleteHeight, corona::PF_R8G8B8A8);
	if(pcCompleteImage == NULL) {
		MessageBox(hWnd, "Could not create final image for preview", "Warning", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	// Create the preview image
	corona::Image *pcPreviewImage = corona::CreateImage(nPrevWidth, nPrevHeight, corona::PF_R8G8B8A8);
	if(pcPreviewImage == NULL) {
		MessageBox(hWnd, "Could not create preview image", "Warning", MB_OK | MB_ICONEXCLAMATION);
		delete pcCompleteImage;
		return false;
	}

	Fill(pcPreviewImage, 0xFF000000);


	// Build the final image
	uchar *pMat = (uchar *)pcMaterialImage->getPixels();
	uchar *pFront = (uchar *)pcFrontImage->getPixels();
	uchar *pBack = (uchar *)pcBackImage->getPixels();
	uchar *pFinal = (uchar *)pcCompleteImage->getPixels();
	int nFinal=0;
	for(int n=0;n<nCompleteWidth*nCompleteHeight*3;) {
		uchar t = PX_EMPTY;
		int r = n;
		int g = n+1;
		int b = n+2;
		n+=3;

		int nColr = pMat[r];
		int nColg = pMat[g];
		int nColb = pMat[b];

		// Material type
		if(nColr==0 && nColg==0 && nColb==0)
			t = PX_EMPTY;
		if(nColr==255 && nColg==255 && nColb==255)
			t = PX_DIRT;
		if(nColr==128 && nColg==128 && nColb==128)
			t = PX_ROCK;

		if(t == PX_EMPTY) {
			pFinal[nFinal++] = pBack[r];
			pFinal[nFinal++] = pBack[g];
			pFinal[nFinal++] = pBack[b];
			pFinal[nFinal++] = 255;
		} else {
			pFinal[nFinal++] = pFront[r];
			pFinal[nFinal++] = pFront[g];
			pFinal[nFinal++] = pFront[b];
			pFinal[nFinal++] = 255;
		}
	}


	/*
	 * Create an image array for the resized image destination
	 */
	int nDestWidth = nCompleteWidth;
	int nDestHeight = nCompleteHeight;
	
	if(nCompleteHeight > nCompleteWidth) {
		float fRatio = (float)nCompleteWidth / (float)nCompleteHeight;
		nDestHeight = nPrevHeight;
		nDestWidth = (int)((float)nPrevHeight*fRatio);
	} else {
		float fRatio = (float)nCompleteHeight / (float)nCompleteWidth;
		nDestWidth = nPrevWidth;
		nDestHeight = (int)((float)nPrevWidth*fRatio);
	}

	PIXEL *pDest = new PIXEL[nDestWidth*nDestHeight*4];
	if(pDest == NULL) {
		MessageBox(hWnd, "Could not allocate memory for preview image", "Warning", MB_OK);
		return false;
	}

	PIXEL *pSource = (ulong *)pcCompleteImage->getPixels();

	ScaleRectAvg(pDest, pSource, 
		         pcCompleteImage->getWidth(), pcCompleteImage->getHeight(),
				 nDestWidth, nDestHeight);

	int x = nPrevWidth/2 - nDestWidth/2;
	int y = nPrevHeight/2 - nDestHeight/2;

	BlitSurface((ulong *)pcPreviewImage->getPixels(), pcPreviewImage->getWidth(),
				pDest, nDestWidth,
				x ,y,
				0, 0,
				nDestWidth, nDestHeight);


	bool bRet = corona::SaveImage(szPreviewFile, corona::FF_AUTODETECT, pcPreviewImage);
	if(!bRet)
		MessageBox(hWnd, "Could not save preview image (try a TGA or PNG format)", "Warning", MB_OK);

	// Free memory & images
	delete[] pDest;
	delete pcCompleteImage;
	delete pcPreviewImage;

	return true;
}


/*-------------------------------
 * Fill an image with a colour
 *-------------------------------*/
void Fill(corona::Image *dest, ulong nColour)
{
	int x, y;

	for(y=0; y<dest->getHeight(); y++) {
		for(x=0; x<dest->getWidth(); x++) {
			ulong *pixel = (ulong *)dest->getPixels() + (y*dest->getWidth() + x);

			*pixel = nColour;
		}
	}
}


/*-------------------------------
 * Blit an image from one array to another
 *-------------------------------*/
void BlitSurface(ulong *dest, int destwidth, ulong *src, int srcwidth, int dx, int dy, int sx, int sy, int w, int h)
{
	int x, y;

	// Warning: Does not perform clipping

	// Copy the character to the out image file
	for(y=0; y<h; y++) {
		for(x=0; x<w; x++) {
			ulong *pixel = src + ((y+sy)*srcwidth + (x+sx));

			ulong *opixel = dest + ((y+dy)*destwidth + (x+dx));

			*opixel = *pixel;
		}
	}
}



///////////////////
// Shutdown
void Shutdown(void)
{
	if(pcFrontImage) {
		delete pcFrontImage;
		pcFrontImage = NULL;
	}

	if(pcBackImage) {
		delete pcBackImage;
		pcBackImage = NULL;
	}

	if(pcMaterialImage) {
		delete pcMaterialImage;
		pcMaterialImage = NULL;
	}
}