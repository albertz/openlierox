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

#include "CWormSkin.h"
#include "debug.h"
#include "DeprecatedGUI/Graphics.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "LieroX.h" // for bDedicated

#define FRAME_WIDTH 32
#define FRAME_HEIGHT 18
#define FRAME_SPACING 4

/////////////////////
// Constructors
CWormSkin::CWormSkin(const std::string &file)
{
	if (bDedicated)  { // No skins in dedicated mode
		bmpSurface = NULL;
	} else {
		bmpSurface = LoadGameImage("skins/" + file, true);
		if (!bmpSurface.get()) // Try to load the default skin if the given one failed
			bmpSurface = LoadGameImage("skins/default.png", true);
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

	GenerateShadow();
	GenerateMirroredImage();
	GenerateNormalSurface();
	GeneratePreview();
}

CWormSkin::CWormSkin()
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
}

//////////////////
// Destructor
CWormSkin::~CWormSkin()
{
	// Free the surfaces
	bmpShadow = NULL;
	bmpNormal = NULL;
	bmpMirrored = NULL;
	bmpMirroredShadow = NULL;
	bmpPreview = NULL;
}

////////////////////
// Change the skin
void CWormSkin::Change(const std::string &file)
{
	if (bDedicated)  {
		bmpSurface = NULL;
	} else {
		bmpSurface = LoadGameImage("skins/" + file, true);
		if (!bmpSurface.get()) // Try to load the default skin if the given one failed
			bmpSurface = LoadGameImage("skins/default.png", true);
		if (bmpSurface.get())
			SetColorKey(bmpSurface.get());
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
bool CWormSkin::PrepareNormalSurface()
{
	// Check
	if (!bmpSurface.get() || bDedicated)
		return false;

	// Allocate
	if (!bmpNormal.get())  {
		bmpNormal = gfxCreateSurfaceAlpha(bmpSurface->w, FRAME_HEIGHT);
		if (!bmpNormal.get())
			return false;
	}

	FillSurfaceTransparent(bmpNormal.get());

	return true;	
}

///////////////////////
// Generates the normal surface, no colorization is done
void CWormSkin::GenerateNormalSurface()
{
	if (!PrepareNormalSurface())
		return;

	// Just copy the upper row
	CopySurface(bmpNormal.get(), bmpSurface.get(), 0, 0, 0, 0, bmpNormal->w, bmpNormal->h);
}

//////////////////
// Prepare the mirrored surface
bool CWormSkin::PrepareMirrorSurface()
{
	// Check
	if (!bmpSurface.get() || bDedicated)
		return false;

	// Allocate
	if (!bmpMirrored.get())  {
		bmpMirrored = gfxCreateSurfaceAlpha(bmpSurface->w, FRAME_HEIGHT);
		if (!bmpMirrored.get())
			return false;
	}

	FillSurfaceTransparent(bmpMirrored.get());

	return true;
}

////////////////////
// Generate a mirrored image
void CWormSkin::GenerateMirroredImage()
{
	if (!PrepareMirrorSurface())
		return;

	DrawImageAdv_Mirror(bmpMirrored.get(), bmpSurface.get(), 0, 0, 0, 0, bmpMirrored->w, bmpMirrored->h);
}

//////////////////
// Prepares the preview surface, returns false when there was some error
bool CWormSkin::PreparePreviewSurface()
{
	// No surfaces in dedicated mode
	if (bDedicated)
		return false;

	// Allocate
	if (!bmpPreview.get())  {
		bmpPreview = gfxCreateSurfaceAlpha(SKIN_WIDTH, SKIN_HEIGHT);
		if (!bmpPreview.get())
			return false;
	}

	// Fill with pink
	FillSurfaceTransparent(bmpPreview.get());

	return bmpNormal.get() != NULL;
}

/////////////////////
// Generates a preview image
void CWormSkin::GeneratePreview()
{
	if (!PreparePreviewSurface())
		return;

	// Worm image
	static const int preview_frame = 4;
	int sx = preview_frame * FRAME_WIDTH + FRAME_SPACING;
	CopySurface(bmpPreview.get(), bmpNormal.get(), sx, 0, 0, 0, bmpPreview->w, bmpPreview->h);

	// CPU image
	if (iBotIcon >= 0 && DeprecatedGUI::gfxGame.bmpAI.get())
		DrawImageAdv(bmpPreview.get(), DeprecatedGUI::gfxGame.bmpAI.get(),
			iBotIcon * CPU_WIDTH, 0, 0, SKIN_HEIGHT - DeprecatedGUI::gfxGame.bmpAI->h, CPU_WIDTH, DeprecatedGUI::gfxGame.bmpAI->h); 
}

//////////////////
// Prepares the shadow surface, returns false when there was some error
bool CWormSkin::PrepareShadowSurface()
{
	// Make sure we have something to create the shadow from
	if (!bmpSurface.get() || bDedicated)
		return false;
	if (bmpSurface->h < FRAME_HEIGHT)
		return false;

	// Allocate the shadow surface
	if (!bmpShadow.get())  {
		bmpShadow = gfxCreateSurface(bmpSurface.get()->w, FRAME_HEIGHT);
		if (!bmpShadow.get())
			return false;
	}

	// Allocate the shadow mirror image
	if (!bmpMirroredShadow.get())  {
		bmpMirroredShadow = gfxCreateSurface(bmpSurface->w, FRAME_HEIGHT);
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
void CWormSkin::GenerateShadow()
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

	// Go through the pixels and put the non-transparent ones to the shadow surface
	for (int y = 0; y < FRAME_HEIGHT; ++y)  {
		srcpix = srcrow;

		for (int x = 0; x < bmpSurface->w; ++x)  {
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
		to = gfxCreateSurface(from->w, from->h);

	// Alloc error
	if (!to.get())
		return;

	// Copy
	CopySurface(to.get(), from.get(), 0, 0, 0, 0, to->w, to->h);
	SetColorKey(to.get());
}

///////////////////
// Assignment operator
CWormSkin& CWormSkin::operator =(const CWormSkin &oth)
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
	}
	return *this;
}

////////////////////
// Comparison operator
bool CWormSkin::operator ==(const CWormSkin &oth)
{
	if (sFileName.size())
		return stringcaseequal(sFileName, oth.sFileName);
	else
		return bmpSurface.get() == oth.bmpSurface.get();
}

///////////////////////
// Draw the worm skin at the specified coordinates
void CWormSkin::Draw(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored)
{
	// No skins in dedicated mode
	if (bDedicated)
		return;

	// Get the correct frame
	int sx = frame * FRAME_WIDTH + FRAME_SPACING;
	int sy = (FRAME_HEIGHT - SKIN_HEIGHT);

	// Draw the skin
	if (mirrored)  {
		if (bmpMirrored.get())
			DrawImageAdv(surf, bmpMirrored.get(), bmpMirrored->w - sx - SKIN_WIDTH - 1, sy, x, y, SKIN_WIDTH, SKIN_HEIGHT);
	} else {
		if (bmpNormal.get())
			DrawImageAdv(surf, bmpNormal.get(), sx, sy, x, y, SKIN_WIDTH, SKIN_HEIGHT);
	}

	// Bot icon?
	if (iBotIcon >= 0 && draw_cpu && DeprecatedGUI::gfxGame.bmpAI.get())  {
		DrawImageAdv(surf, DeprecatedGUI::gfxGame.bmpAI.get(),
		iBotIcon * CPU_WIDTH, 0, 0, SKIN_HEIGHT - DeprecatedGUI::gfxGame.bmpAI->h, CPU_WIDTH, DeprecatedGUI::gfxGame.bmpAI->h); 
	}
}

/////////////////////
// Draw the worm skin shadow
void CWormSkin::DrawShadow(SDL_Surface *surf, int x, int y, int frame, bool mirrored)
{
	// No skins in dedicated mode
	if (bDedicated)
		return;

	// Get the correct frame
	int sx = frame * FRAME_WIDTH + FRAME_SPACING;
	int sy = (FRAME_HEIGHT - SKIN_HEIGHT);

	// Draw the shadow
	if (mirrored)  {
		if (bmpMirroredShadow.get())
			DrawImageAdv(surf, bmpMirroredShadow.get(), bmpMirroredShadow->w - sx - SKIN_WIDTH - 1, sy, x, y, SKIN_WIDTH, SKIN_HEIGHT);
	} else {
		if (bmpShadow.get())
			DrawImageAdv(surf, bmpShadow.get(), sx, sy, x, y, SKIN_WIDTH, SKIN_HEIGHT);
	}
}

////////////////////////
// Colorize the skin
void CWormSkin::Colorize(Uint32 col)
{
	// No skins in dedicated mode
	if (bDedicated)
		return;

	// Check if we need to change the color
	if (bColorized && col == iColor)
		return;
	if (!bmpSurface.get() || !bmpNormal.get() || !bmpMirrored.get())
		return;
	if (bmpSurface->h < 2 * FRAME_HEIGHT || bmpNormal->w != bmpSurface->w || bmpMirrored->w != bmpSurface->w)
		return;

	bColorized = true;
	iColor = col;

	// Lock
	LOCK_OR_QUIT(bmpSurface);
	LOCK_OR_QUIT(bmpNormal);
	LOCK_OR_QUIT(bmpMirrored);

	// Get the color
	Uint8 colR, colG, colB;
	GetColour3(col, getMainPixelFormat(), &colR, &colG, &colB);

    // Set the colour of the worm
	const Uint32 black = SDL_MapRGB(bmpSurface->format, 0, 0, 0);

	for (int y = 0; y < FRAME_HEIGHT; ++y) {
		for (int x = 0; x < bmpSurface->w; ++x) {

			// Use the mask to check what colours to ignore
			Uint32 pixel = GetPixel(bmpSurface.get(), x, y);
			Uint32 mask = GetPixel(bmpSurface.get(), x, y + FRAME_HEIGHT);
            
            // Black means to just copy the colour but don't alter it
            if (EqualRGB(mask, black, bmpSurface.get()->format)) {
				PutPixel(bmpNormal.get(), x, y, pixel);
				PutPixel(bmpMirrored.get(), MAX(0, bmpMirrored->w - x - 1), y, pixel);
                continue;
            }

            // Pink means just ignore the pixel completely
            if( IsTransparent(bmpSurface.get(), mask) )
                continue;

            // Must be white (or some over unknown colour)

			float dr = (float)GetR(pixel, bmpSurface->format) / 96.0f;
			float dg = (float)GetG(pixel, bmpSurface->format) / 156.0f;
			float db = (float)GetB(pixel, bmpSurface->format) / 252.0f;

			float r2 = (float)colR * dr;
			float g2 = (float)colG * dg;
			float b2 = (float)colB * db;

			r2 = MIN(255.0f, r2);
			g2 = MIN(255.0f, g2);
			b2 = MIN(255.0f, b2);


			// Bit of a hack to make sure it isn't completey pink (see through)
			if((int)r2 == 255 && (int)g2 == 0 && (int)b2 == 255) {
				r2 = 240;
				b2 = 240;
			}

            // Put the colourised pixel
			PutPixel(bmpNormal.get(), x, y, 
				SDL_MapRGBA(bmpNormal->format, (int)r2, (int)g2, (int)b2, GetA(pixel, bmpSurface->format)));
			PutPixel(bmpMirrored.get(), MAX(0, bmpMirrored->w - x - 1), y, 
				SDL_MapRGBA(bmpNormal->format, (int)r2, (int)g2, (int)b2, GetA(pixel, bmpSurface->format)));
		}
	}

	UnlockSurface(bmpNormal);
	UnlockSurface(bmpMirrored);
	UnlockSurface(bmpSurface);

	// Regenerate the preview
	GeneratePreview();
}

