/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Worm skin class
// Created 16/6/08
// Karel Petranek

#include "CGameSkin.h"

#include "DeprecatedGUI/Graphics.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "LieroX.h" // for bDedicated
#include "Debug.h"

/////////////////////
// Constructors
CGameSkin::CGameSkin(const std::string &file, int fw, int fh, int fs, int sw, int sh)
{
	// TODO: share code with Change()
	
	if (bDedicated)  { // No skins in dedicated mode
		bmpSurface = NULL;
	} else {
		bmpSurface = LoadGameImage("skins/" + file, true);
		if (!bmpSurface.get()) { // Try to load the default skin if the given one failed
			warnings << "CGameSkin::Change: couldn't find skin " << file << endl;
			bmpSurface = LoadGameImage("skins/default.png", true);
		}
		if (bmpSurface.get())
			SetColorKey(bmpSurface.get());
	}
	bmpMirrored = NULL;
	bmpShadow = NULL;
	bmpMirroredShadow = NULL;
	bmpPreview = NULL;
	bmpNormal = NULL;

	sFileName = file;
	iDefaultColor = iColor = MakeColour(128, 128, 128);
	bColorized = false;
	iBotIcon = -1;
	iFrameWidth = fw;
	iFrameHeight = fh;
	iFrameSpacing = fs;
	iSkinWidth = sw;
	iSkinHeight = sh;

	GenerateShadow();
	GenerateMirroredImage();
	GenerateNormalSurface();
	GeneratePreview();
}

CGameSkin::CGameSkin(int fw, int fh, int fs, int sw, int sh)
{
	bmpSurface = NULL;
	bmpMirrored = NULL;
	bmpShadow = NULL;
	bmpMirroredShadow = NULL;
	bmpPreview = NULL;
	bmpNormal = NULL;
	sFileName = "";
	iDefaultColor = iColor = MakeColour(128, 128, 128);
	bColorized = false;
	iBotIcon = -1;
	iFrameWidth = fw;
	iFrameHeight = fh;
	iFrameSpacing = fs;
	iSkinWidth = sw;
	iSkinHeight = sh;
}

CGameSkin::CGameSkin(const CGameSkin& skin)
{
	operator=(skin);
}

//////////////////
// Destructor
CGameSkin::~CGameSkin()
{
}

////////////////////
// Change the skin
void CGameSkin::Change(const std::string &file)
{
	if (bDedicated)  {
		bmpSurface = NULL;
	} else {
		bmpSurface = LoadGameImage("skins/" + file, true);
		if (!bmpSurface.get()) { // Try to load the default skin if the given one failed
			warnings << "CGameSkin::Change: couldn't find skin " << file << endl;
			bmpSurface = LoadGameImage("skins/default.png", true);
		}
		if (bmpSurface.get())  {
			SetColorKey(bmpSurface.get());
			if (bmpSurface->w != 672 || bmpSurface->h != 36)
				notes << "The skin " << file << " has a non-standard size (" << bmpSurface->w << "x" << bmpSurface->h << ")" << endl;
		}
	}
	sFileName = file;

	GenerateNormalSurface();
	GenerateShadow();
	GenerateMirroredImage();

	// Colorize
	if (bColorized)  {
		bColorized = false; // To force the recolorization
		Colorize(iColor);
	}

	GeneratePreview();
}

/////////////////////
// Prepares the non-mirrored surface
bool CGameSkin::PrepareNormalSurface()
{
	// Check
	if (!bmpSurface.get() || bDedicated)
		return false;

	// Allocate
	if (!bmpNormal.get())  {
		bmpNormal = gfxCreateSurfaceAlpha(bmpSurface->w, iFrameHeight);
		if (!bmpNormal.get())
			return false;
	}

	FillSurfaceTransparent(bmpNormal.get());

	return true;	
}

///////////////////////
// Generates the normal surface, no colorization is done
void CGameSkin::GenerateNormalSurface()
{
	if (!PrepareNormalSurface())
		return;

	// Just copy the upper row
	CopySurface(bmpNormal.get(), bmpSurface.get(), 0, 0, 0, 0, bmpNormal->w, bmpNormal->h);
}

//////////////////
// Prepare the mirrored surface
bool CGameSkin::PrepareMirrorSurface()
{
	// Check
	if (!bmpSurface.get() || bDedicated)
		return false;

	// Allocate
	if (!bmpMirrored.get())  {
		bmpMirrored = gfxCreateSurfaceAlpha(bmpSurface->w, iFrameHeight);
		if (!bmpMirrored.get())
			return false;
	}

	FillSurfaceTransparent(bmpMirrored.get());

	return true;
}

////////////////////
// Generate a mirrored image
void CGameSkin::GenerateMirroredImage()
{
	if (!PrepareMirrorSurface())
		return;

	DrawImageAdv_Mirror(bmpMirrored.get(), bmpSurface.get(), 0, 0, 0, 0, bmpMirrored->w, bmpMirrored->h);
}

//////////////////
// Prepares the preview surface, returns false when there was some error
bool CGameSkin::PreparePreviewSurface()
{
	// No surfaces in dedicated mode
	if (bDedicated)
		return false;

	// Allocate
	if (!bmpPreview.get())  {
		bmpPreview = gfxCreateSurfaceAlpha(iSkinWidth, iSkinHeight);
		if (!bmpPreview.get())
			return false;
	}

	// Fill with pink
	FillSurfaceTransparent(bmpPreview.get());

	return bmpNormal.get() != NULL;
}

/////////////////////
// Generates a preview image
void CGameSkin::GeneratePreview()
{
	if (!PreparePreviewSurface())
		return;

	// Worm image
	static const int preview_frame = 4;
	int sx = preview_frame * iFrameWidth + iFrameSpacing;
	CopySurface(bmpPreview.get(), bmpNormal.get(), sx, 0, 0, 0, bmpPreview->w, bmpPreview->h);

	// CPU image
	if (iBotIcon >= 0 && DeprecatedGUI::gfxGame.bmpAI.get())
		DrawImageAdv(bmpPreview.get(), DeprecatedGUI::gfxGame.bmpAI.get(),
			iBotIcon * CPU_WIDTH, 0, 0, iSkinHeight - DeprecatedGUI::gfxGame.bmpAI->h, CPU_WIDTH, DeprecatedGUI::gfxGame.bmpAI->h); 
}

//////////////////
// Prepares the shadow surface, returns false when there was some error
bool CGameSkin::PrepareShadowSurface()
{
	// Make sure we have something to create the shadow from
	if (!bmpSurface.get() || bDedicated)
		return false;
	if (bmpSurface->h < iFrameHeight)
		return false;

	// Allocate the shadow surface
	if (!bmpShadow.get())  {
		bmpShadow = gfxCreateSurface(bmpSurface.get()->w, iFrameHeight);
		if (!bmpShadow.get())
			return false;
	}

	// Allocate the shadow mirror image
	if (!bmpMirroredShadow.get())  {
		bmpMirroredShadow = gfxCreateSurface(bmpSurface->w, iFrameHeight);
		if (!bmpMirroredShadow.get())
			return false;
	}

	// Set the color key & alpha
	SetColorKey(bmpShadow.get());
	SetColorKey(bmpMirroredShadow.get());
	SetPerSurfaceAlpha(bmpShadow.get(), SHADOW_OPACITY);
	SetPerSurfaceAlpha(bmpMirroredShadow.get(), SHADOW_OPACITY);

	// Clear the shadow surface
	FillSurfaceTransparent(bmpShadow.get());
	FillSurfaceTransparent(bmpMirroredShadow.get());

	return true;
}

//////////////////
// Generate a shadow surface for the skin
void CGameSkin::GenerateShadow()
{
	if (!PrepareShadowSurface())
		return;

	// Lock the surface & get the pixels
	LOCK_OR_QUIT(bmpSurface);
	LOCK_OR_QUIT(bmpShadow);
	LOCK_OR_QUIT(bmpMirroredShadow);
	Uint8 *srcrow = (Uint8 *)bmpSurface.get()->pixels;
	Uint8 *srcpix = NULL;
	int srcbpp = bmpSurface->format->BytesPerPixel;

	const Uint32 black = SDL_MapRGB(bmpShadow->format, 0, 0, 0);
	int width = MIN(MIN(bmpSurface->w, bmpShadow->w), bmpMirroredShadow->w);

	// Go through the pixels and put the non-transparent ones to the shadow surface
	for (int y = 0; y < iFrameHeight; ++y)  {
		srcpix = srcrow;

		for (int x = 0; x < width; ++x)  {
			if (!IsTransparent(bmpSurface.get(), GetPixelFromAddr(srcpix, srcbpp)))  {
				PutPixel(bmpShadow.get(), x, y, black);
				PutPixel(bmpMirroredShadow.get(), bmpMirroredShadow->w - x - 1, y, black);
			}
			srcpix += srcbpp;
		}

		srcrow += bmpSurface->pitch;
	}

	// Unlock
	UnlockSurface(bmpSurface);
	UnlockSurface(bmpShadow);
	UnlockSurface(bmpMirroredShadow);
}

//////////////////
// Helper function for the assignment operator
static void copy_surf(SmartPointer<SDL_Surface>& to, const SmartPointer<SDL_Surface>& from)
{
	// NULL check
	if (!from.get())  {
		to = NULL;
		return;
	}

	// Create if doesn't exist
	if (!to.get())
		to = gfxCreateSurfaceAlpha(from->w, from->h);

	// Alloc error
	if (!to.get())
		return;

	// Copy
	CopySurface(to.get(), from.get(), 0, 0, 0, 0, to->w, to->h);
	SetColorKey(to.get());
}

///////////////////
// Assignment operator
CGameSkin& CGameSkin::operator =(const CGameSkin &oth)
{
	if (this != &oth)  { // Check for self-assignment
		if (!bDedicated)  {
			// NOTE: the assignment is safe because of smartpointer
			// HINT: the bmpSurface never changes, so it's safe to assign it like this
			bmpSurface = oth.bmpSurface;

			// Other surfaces
			copy_surf(bmpShadow, oth.bmpShadow);
			copy_surf(bmpMirrored, oth.bmpMirrored);
			copy_surf(bmpMirroredShadow, oth.bmpMirroredShadow);
			copy_surf(bmpPreview, oth.bmpPreview);
			copy_surf(bmpNormal, oth.bmpNormal);
		}
		
		// Other details
		sFileName = oth.sFileName;
		iColor = oth.iColor;
		iDefaultColor = oth.iDefaultColor;
		bColorized = oth.bColorized;
		iBotIcon = oth.iBotIcon;
		iFrameWidth = oth.iFrameWidth;
		iFrameHeight = oth.iFrameHeight;
		iFrameSpacing = oth.iFrameSpacing;
		iSkinWidth = oth.iSkinWidth;
		iSkinHeight = oth.iSkinHeight;
	}
	return *this;
}

////////////////////
// Comparison operator
bool CGameSkin::operator ==(const CGameSkin &oth)
{
	if (sFileName.size())
		return stringcaseequal(sFileName, oth.sFileName);
	else
		return bmpSurface.get() == oth.bmpSurface.get();
}

///////////////////////
// Draw the worm skin at the specified coordinates
void CGameSkin::Draw(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored)
{
	// No skins in dedicated mode
	if (bDedicated)
		return;

	// Get the correct frame
	int sx = frame * iFrameWidth + iFrameSpacing;
	int sy = (iFrameHeight - iSkinHeight);

	// Draw the skin
	if (mirrored)  {
		if (bmpMirrored.get())
			DrawImageAdv(surf, bmpMirrored.get(), bmpMirrored->w - sx - iSkinWidth - 1, sy, x, y, iSkinWidth, iSkinHeight);
	} else {
		if (bmpNormal.get())
			DrawImageAdv(surf, bmpNormal.get(), sx, sy, x, y, iSkinWidth, iSkinHeight);
	}

	// Bot icon?
	if (iBotIcon >= 0 && draw_cpu && DeprecatedGUI::gfxGame.bmpAI.get())  {
		DrawImageAdv(surf, DeprecatedGUI::gfxGame.bmpAI.get(),
		iBotIcon * CPU_WIDTH, 0, 0, iSkinHeight - DeprecatedGUI::gfxGame.bmpAI->h, CPU_WIDTH, DeprecatedGUI::gfxGame.bmpAI->h); 
	}
}

/////////////////////
// Draw the worm skin shadow
void CGameSkin::DrawShadow(SDL_Surface *surf, int x, int y, int frame, bool mirrored)
{
	// No skins in dedicated mode
	if (bDedicated)
		return;

	// Get the correct frame
	int sx = frame * iFrameWidth + iFrameSpacing;
	int sy = (iFrameHeight - iSkinHeight);

	// Draw the shadow
	if (mirrored)  {
		if (bmpMirroredShadow.get())
			DrawImageAdv(surf, bmpMirroredShadow.get(), bmpMirroredShadow->w - sx - iSkinWidth - 1, sy, x, y, iSkinWidth, iSkinHeight);
	} else {
		if (bmpShadow.get())
			DrawImageAdv(surf, bmpShadow.get(), sx, sy, x, y, iSkinWidth, iSkinHeight);
	}
}

////////////////////////
// Colorize the skin
void CGameSkin::Colorize(Color col)
{
	// No skins in dedicated mode
	if (bDedicated)
		return;

	// Check if we need to change the color
	if (bColorized && col == iColor)
		return;
	if (!bmpSurface.get() || !bmpNormal.get() || !bmpMirrored.get())
		return;
	if (bmpSurface->h < 2 * iFrameHeight)
		return;

	bColorized = true;
	iColor = col;

	// Lock
	LOCK_OR_QUIT(bmpSurface);
	LOCK_OR_QUIT(bmpNormal);
	LOCK_OR_QUIT(bmpMirrored);

	// Get the color
	// TODO: cleanup
	const Uint8 colR = col.r, colG = col.g, colB = col.b;

    // Set the colour of the worm
	const Uint32 black = SDL_MapRGB(bmpSurface->format, 0, 0, 0);
	int width = MIN(MIN(bmpSurface->w, bmpNormal->w), bmpMirrored->w);

	for (int y = 0; y < iFrameHeight; ++y) {
		for (int x = 0; x < width; ++x) {

			// Use the mask to check what colours to ignore
			Uint32 pixel = GetPixel(bmpSurface.get(), x, y);
			Uint32 mask = GetPixel(bmpSurface.get(), x, y + iFrameHeight);
            
            // Black means to just copy the colour but don't alter it
            if (EqualRGB(mask, black, bmpSurface.get()->format)) {
				PutPixel(bmpNormal.get(), x, y, pixel);
				PutPixel(bmpMirrored.get(), MAX(0, bmpMirrored->w - x - 1), y, pixel);
                continue;
            }

            // Pink means just ignore the pixel completely
            if (IsTransparent(bmpSurface.get(), mask))
                continue;

            // Must be white (or some over unknown colour)
			unsigned int r2 = colR * GetR(pixel, bmpSurface->format) / 96;
			unsigned int g2 = colG * GetG(pixel, bmpSurface->format) / 156;
			unsigned int b2 = colB * GetB(pixel, bmpSurface->format) / 252;

			r2 = MIN(255, r2);
			g2 = MIN(255, g2);
			b2 = MIN(255, b2);


			// Bit of a hack to make sure it isn't completey pink (see through)
			if((int)r2 == 255 && (int)g2 == 0 && (int)b2 == 255) {
				r2 = 240;
				b2 = 240;
			}

            // Put the colourised pixel
			Uint8 a = GetA(pixel, bmpSurface->format);
			PutPixel(bmpNormal.get(), x, y, 
				SDL_MapRGBA(bmpNormal->format, r2, g2, b2, a));
			PutPixel(bmpMirrored.get(), MAX(0, bmpMirrored->w - x - 1), y, 
				SDL_MapRGBA(bmpMirrored->format, r2, g2, b2, a));
		}
	}

	UnlockSurface(bmpNormal);
	UnlockSurface(bmpMirrored);
	UnlockSurface(bmpSurface);

	// Regenerate the preview
	GeneratePreview();
}

////////////////////
// Returns number of frames in the skin animation
int CGameSkin::getFrameCount() const
{
	if (bmpSurface.get())
		return bmpSurface->w / (iFrameWidth + iFrameSpacing);
	else
		return 0;
}
