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

#include <typeinfo>

#include "CGameSkin.h"
#include "DeprecatedGUI/Graphics.h"
#include "StringUtils.h"
#include "MathLib.h"
#include "LieroX.h" // for bDedicated
#include "Debug.h"
#include "Mutex.h"
#include "Condition.h"
#include "game/CMap.h" // for CMap::DrawObjectShadow
#include "PixelFunctors.h"

// global mutex to force only one execution at time
static Mutex skinActionHandlerMutex;

struct Skin_Action : Action {
	bool breakSignal;
	CGameSkin* skin;
	Skin_Action(CGameSkin* s) : breakSignal(false), skin(s) {}
};

struct GameSkinPreviewDrawer : DynDrawIntf {
	Mutex mutex;
	CGameSkin* skin;
	GameSkinPreviewDrawer(CGameSkin* s) : DynDrawIntf(s->iSkinWidth,s->iSkinHeight), skin(s) {}
	void draw(SDL_Surface* surf, int x, int y);
};

struct CGameSkin::Thread {
	Mutex mutex;
	Condition signal;
	bool ready;
	typedef std::list<Skin_Action*> ActionList;
	ActionList actionQueue;
	Skin_Action* curAction;
	
	GameSkinPreviewDrawer* skinPreviewDrawerP;
	SmartPointer<DynDrawIntf> skinPreviewDrawer;
	
	Thread(CGameSkin* s) : ready(true), curAction(NULL), skinPreviewDrawerP(NULL) {
		skinPreviewDrawer = skinPreviewDrawerP = new GameSkinPreviewDrawer(s);
	}
	~Thread() {
		forceStopThread();
		if(actionQueue.size() > 0) {
			// Note: We should normally not get here. If we would, it means that we add some actions
			// at some point in the code but we don't assure that the thread is running after.
			// The thread itself always assures that at the exit of the thread, no further action is in the queue.
			warnings << "CGameSkin::Thread uninit: still actions in queue" << endl;
			removeActions__unsafe();
		}
		Mutex::ScopedLock lock(skinPreviewDrawerP->mutex);
		skinPreviewDrawerP->skin = NULL;
	}
	
	void pushAction__unsafe(Skin_Action* a) {
		actionQueue.push_back(a);
	}
	
	static void cleanAction__unsafe(Skin_Action*& a) {
		delete a;
		a = NULL;
	}
	
	void removeActions__unsafe(const std::type_info& actionType) {
		for(ActionList::iterator i = actionQueue.begin(); i != actionQueue.end(); ) {
			if(typeid(**i) == actionType) {
				cleanAction__unsafe(*i);
				i = actionQueue.erase(i);
			} else
				++i;
		}
	}
	
	void removeActions__unsafe() {
		for(ActionList::iterator i = actionQueue.begin(); i != actionQueue.end(); ++i)
			cleanAction__unsafe(*i);			
		actionQueue.clear();
	}
	
	// run this after you added something to actionQueue to be sure that it will get handled
	void startThread__unsafe(CGameSkin* skin) {
		if(!ready) return; // !ready -> thread already running
		struct SkinActionHandler : Action {
			CGameSkin* skin;
			SkinActionHandler(CGameSkin* s) : skin(s) {}
			Result handle() {
				Result lastRet(true);
				Mutex::ScopedLock lock(skin->thread->mutex);
				while(skin->thread->actionQueue.size() > 0) {
					skin->thread->curAction = skin->thread->actionQueue.front();
					skin->thread->actionQueue.pop_front();
					
					skin->thread->mutex.unlock();
					skinActionHandlerMutex.lock(); // just to force only one action at a time globally
					lastRet = skin->thread->curAction->handle();
					skinActionHandlerMutex.unlock();
					skin->thread->mutex.lock();
					CGameSkin::Thread::cleanAction__unsafe(skin->thread->curAction);
				}
				skin->thread->ready = true;
				skin->thread->signal.broadcast();
				return lastRet;
			}
		};
		threadPool->start(new SkinActionHandler(skin), "CGameSkin handler", true);
		ready = false;
	}
	
	void startThread(CGameSkin* skin) {
		Mutex::ScopedLock lock(mutex);
		startThread__unsafe(skin);
	}
	
	void forceStopThread() {
		Mutex::ScopedLock lock(mutex);
		while(!ready) {
			removeActions__unsafe();
			if(curAction) curAction->breakSignal = true;
			signal.wait(mutex);
		}
	}
};

void CGameSkin::init(int fw, int fh, int fs, int sw, int sh) {	
	bmpSurface = NULL;
	bmpMirrored = NULL;
	bmpShadow = NULL;
	bmpMirroredShadow = NULL;
	bmpPreview = NULL;
	bmpNormal = NULL;
	sFileName = "";
	iDefaultColor = iColor = Color(128, 128, 128);
	bColorized = false;
	iBotIcon = -1;
	
	iFrameWidth = fw;
	iFrameHeight = fh;
	iFrameSpacing = fs;
	iSkinWidth = sw;
	iSkinHeight = sh;
	
	thread = new Thread(this);	
}

void CGameSkin::uninit() {
	if(thread) {
		delete thread;
		thread = NULL;
	}
	// all other stuff have its own destructors
}

CGameSkin::CGameSkin(int fw, int fh, int fs, int sw, int sh) : thread(NULL)
{
	init(fw,fh,fs,sw,sh);
}

CGameSkin::CGameSkin(const CGameSkin& skin) : thread(NULL)
{
	// we will init the thread also there
	operator=(skin);
}

CGameSkin::~CGameSkin()
{
	uninit();
}


struct SkinAction_Load : Skin_Action {
	bool genPreview;
	SkinAction_Load(CGameSkin* s, bool p) : Skin_Action(s), genPreview(p) {}
	Result handle() {
		skin->Load_Execute(breakSignal);
		if(breakSignal) return true;
		if(genPreview) skin->GeneratePreview();	
		return true;
	}
};


void CGameSkin::Load_Execute(bool& breakSignal) {
	bmpSurface = LoadGameImage("skins/" + sFileName, true);
	if(breakSignal) return;

	if (!bmpSurface.get()) { // Try to load the default skin if the given one failed
		warnings << "CGameSkin::Change: couldn't find skin " << sFileName << endl;
		bmpSurface = LoadGameImage("skins/default.png", true);
	}
	
	if (bmpSurface.get())  {
		SetColorKey(bmpSurface.get());
		if (bmpSurface->w % iFrameWidth != 0 || bmpSurface->h != 2 * iFrameHeight) {
			notes << "The skin " << sFileName << " has a non-standard size (" << bmpSurface->w << "x" << bmpSurface->h << ")" << endl;
			SmartPointer<SDL_Surface> old = bmpSurface;
			bmpSurface = gfxCreateSurfaceAlpha( old->w - (old->w % iFrameWidth), 2 * iFrameHeight );
			if(bmpSurface.get()) {
				CopySurface(bmpSurface.get(), old.get(), 0, 0, 0, 0, bmpSurface->w, MIN(old->h, 2 * iFrameHeight));
				SetColorKey(bmpSurface.get());
			}
			else
				warnings << "CGameSkin: Cannot create fixed surface" << endl;
		}
	}
	
	if (bmpSurface.get())  {
		if(getFrameCount() < 5) {
			// GeneratePreview would crash in this case
			warnings << "CGameSkin: skin " << sFileName << " too small: only " << getFrameCount() << " frames" << endl;
			bmpSurface = NULL;
		}
	}
	
	if(breakSignal) return;
	GenerateNormalSurface();
	if(breakSignal) return;
	GenerateShadow();
	if(breakSignal) return;
	GenerateMirroredImage();
}

////////////////////
// Change the skin
void CGameSkin::Change(const std::string &file)
{
	if(stringcaseequal(sFileName, file))
		return;
	
	thread->forceStopThread(); // also removes all actions
	// We are assuming here that no other thread is accessing the skin right now in this moment.
	
	sFileName = file;
	if(bDedicated) return;

	thread->pushAction__unsafe(new SkinAction_Load(this, /* generatePreview = */ !bColorized));
	
	if (bColorized) {
		bColorized = false; // To force the recolorization
		Colorize(iColor);
	}
	
	thread->startThread(this);
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


///////////////////
// Assignment operator
CGameSkin& CGameSkin::operator =(const CGameSkin &oth)
{
	if (this != &oth)  { // Check for self-assignment
		// we must do this because we could need surfaces of different width
		uninit();
		init(oth.iFrameWidth, oth.iFrameHeight, oth.iFrameSpacing, oth.iSkinWidth, oth.iSkinHeight);
		iBotIcon = oth.iBotIcon;
		
		// we must reload it because it's not guaranteed that the other skin itself is ready
		iDefaultColor = oth.iDefaultColor;
		bColorized = oth.bColorized;
		iColor = oth.iColor;
		sFileName = "";
		Change(oth.sFileName);
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

bool CGameSkin::operator==(const CustomVar& o) const {
	const CGameSkin* os = dynamic_cast<const CGameSkin*>(&o);
	if(!os) return false;
	return *this == *os;
}

bool CGameSkin::operator<(const CustomVar& o) const {
	const CGameSkin* os = dynamic_cast<const CGameSkin*>(&o);
	if(!os) return this < &o;
	return stringcasecmp(sFileName, os->sFileName) < 0;
}


Color CGameSkin::renderColorAt(int x, int y, int frame, bool mirrored) {
	Mutex::ScopedLock lock(thread->mutex);
	if(!thread->ready) return Color(0,0,0,SDL_ALPHA_TRANSPARENT);	
	if(x < 0 || y < 0 || x >= iSkinWidth || y >= iSkinHeight) return Color(0,0,0,SDL_ALPHA_TRANSPARENT);
	if(bmpMirrored.get() == NULL || bmpNormal.get() == NULL) return Color(0,0,0,SDL_ALPHA_TRANSPARENT);
	
	if (frame < 0)
		frame = 0;
	else if (frame >= getFrameCount())
		frame = getFrameCount() - 1;
	
	// Get the correct frame
	const int sx = frame * iFrameWidth + iFrameSpacing;
	const int sy = (iFrameHeight - iSkinHeight);
		
	// Draw the skin
	if (mirrored)
		return Color(bmpMirrored->format, GetPixel(bmpMirrored.get(), bmpMirrored->w - sx - iSkinWidth - 1 + x, sy + y));
	else
		return Color(bmpNormal->format, GetPixel(bmpNormal.get(), sx + x, sy + y));
}

///////////////////////
// Draw the worm skin at the specified coordinates
void CGameSkin::DrawInternal(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored, bool blockUntilReady, bool half)
{
	// No skins in dedicated mode
	if (bDedicated)
		return;

	Mutex::ScopedLock lock(thread->mutex);
	if(!thread->ready) {
		if(!blockUntilReady) {
			DrawLoadingAni(surf, x + iSkinWidth/2, y + iSkinWidth/2, iSkinWidth/2, iSkinHeight/2, Color(255,0,0), Color(0,255,0), LAT_CAKE);
			return;			
		}
		while(!thread->ready) thread->signal.wait(thread->mutex);
	}
	
	if(bmpMirrored.get() == NULL || bmpNormal.get() == NULL) return;
	
	if (frame < 0)
		frame = 0;
	else if (frame >= getFrameCount())
		frame = getFrameCount() - 1;
	
	// Get the correct frame
	const int sx = frame * iFrameWidth + iFrameSpacing;
	const int sy = (iFrameHeight - iSkinHeight);

	// Draw the skin
	if (mirrored)  {
		if (half)
			DrawImageScaleHalfAdv(surf, bmpMirrored.get(), bmpMirrored->w - sx - iSkinWidth - 1, sy, x, y, iSkinWidth, iSkinHeight);
		else
			DrawImageAdv(surf, bmpMirrored.get(), bmpMirrored->w - sx - iSkinWidth - 1, sy, x, y, iSkinWidth, iSkinHeight);
	} else {
		if (half)
			DrawImageScaleHalfAdv(surf, bmpNormal.get(), sx, sy, x, y, iSkinWidth, iSkinHeight);
		else
			DrawImageAdv(surf, bmpNormal.get(), sx, sy, x, y, iSkinWidth, iSkinHeight);
	}

	// Bot icon?
	if (iBotIcon >= 0 && draw_cpu && DeprecatedGUI::gfxGame.bmpAI.get())  {
		DrawImageAdv(surf, DeprecatedGUI::gfxGame.bmpAI.get(),
		iBotIcon * CPU_WIDTH, 0, 0, iSkinHeight - DeprecatedGUI::gfxGame.bmpAI->h, CPU_WIDTH, DeprecatedGUI::gfxGame.bmpAI->h); 
	}
}

void CGameSkin::Draw(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored, bool blockUntilReady)
{
	DrawInternal(surf, x, y, frame, draw_cpu, mirrored, blockUntilReady, false);
}

void CGameSkin::DrawHalf(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored, bool blockUntilReady)
{
	DrawInternal(surf, x, y, frame, draw_cpu, mirrored, blockUntilReady, true);
}

void GameSkinPreviewDrawer::draw(SDL_Surface* dest, int x, int y) {
	Mutex::ScopedLock lock(mutex);
	if(skin) {
		Mutex::ScopedLock lock2(skin->thread->mutex);
		if(skin->thread->ready && skin->bmpPreview.get())
			DrawImage(dest, skin->bmpPreview, x, y);
		else
			DrawLoadingAni(dest, x + w/2, y + h/2, w/2, h/2, Color(255,0,0), Color(0,255,0), LAT_CAKE);
	}
	else DrawCross(dest, x, y, WORM_SKIN_WIDTH, WORM_SKIN_HEIGHT, Color(255,0,0));
}

SmartPointer<DynDrawIntf> CGameSkin::getPreview() {
	return thread->skinPreviewDrawer;
}

/////////////////////
// Draw the worm skin shadow
void CGameSkin::DrawShadow(SDL_Surface *surf, int x, int y, int frame, bool mirrored)
{
	// No skins in dedicated mode
	if (bDedicated) return;

	Mutex::ScopedLock lock(thread->mutex);
	if(!thread->ready) return;
	
	if(bmpMirrored.get() == NULL || bmpNormal.get() == NULL) return;
	
	if (getFrameCount() != 0)
		frame %= getFrameCount();
	
	// Get the correct frame
	const int sx = frame * iFrameWidth + iFrameSpacing;
	const int sy = (iFrameHeight - iSkinHeight);

	// Draw the shadow
	if (mirrored)  {
		DrawImageAdv(surf, bmpMirroredShadow.get(), bmpMirroredShadow->w - sx - iSkinWidth - 1, sy, x, y, iSkinWidth, iSkinHeight);
	} else {
		DrawImageAdv(surf, bmpShadow.get(), sx, sy, x, y, iSkinWidth, iSkinHeight);
	}
}

void CGameSkin::DrawShadowOnMap(CMap* cMap, CViewport* v, SDL_Surface *surf, int x, int y, int frame, bool mirrored) {
	// No skins in dedicated mode
	if (bDedicated) return;
	
	Mutex::ScopedLock lock(thread->mutex);
	if(!thread->ready) return;

	if(bmpMirrored.get() == NULL || bmpNormal.get() == NULL) return;
	
	if (getFrameCount() != 0)
		frame %= getFrameCount();
	
	// Get the correct frame
	const int sx = frame * iFrameWidth + iFrameSpacing;
	const int sy = (iFrameHeight - iSkinHeight);

	static const int drop = 4;
	
	// draw the shadow
	if (mirrored)  {
		cMap->DrawObjectShadow(surf, bmpMirroredShadow.get(), bmpMirroredShadow.get(), bmpMirroredShadow->w - sx - iSkinWidth - 1, sy, iSkinWidth, iSkinHeight, v, x - iSkinWidth/2 + drop, y - iSkinHeight/2 + drop);
	} else {
		cMap->DrawObjectShadow(surf, bmpShadow.get(), bmpShadow.get(), sx, sy, iSkinWidth, iSkinHeight, v, x - iSkinWidth/2 + drop, y - iSkinHeight/2 + drop);
	}
}

struct SkinAction_Colorize : Skin_Action {
	SkinAction_Colorize(CGameSkin* s) : Skin_Action(s) {}
	Result handle() {
		skin->Colorize_Execute(breakSignal);
		return true;
	}
};

void CGameSkin::Colorize(Color col) {
	// No skins in dedicated mode
	if (bDedicated) return;
	
	Mutex::ScopedLock lock(thread->mutex);
	
	// Check if we need to change the color
	if (bColorized && col == iColor)
		return;

	iColor = col;
	bColorized = true;
	
	thread->removeActions__unsafe(typeid(SkinAction_Colorize));
	thread->pushAction__unsafe(new SkinAction_Colorize(this));
	thread->startThread__unsafe(this);
}

////////////////////////
// Colorize the skin
void CGameSkin::Colorize_Execute(bool& breakSignal)
{
	if (!bmpSurface.get() || !bmpNormal.get() || !bmpMirrored.get())
		return;
	if (bmpSurface->h < 2 * iFrameHeight)
		return;

	// Lock
	LOCK_OR_QUIT(bmpSurface);
	LOCK_OR_QUIT(bmpNormal);
	LOCK_OR_QUIT(bmpMirrored);

	// Get the color
	// TODO: cleanup
	thread->mutex.lock();
	const Uint8 colR = iColor.r, colG = iColor.g, colB = iColor.b;
	thread->mutex.unlock();

    // Set the colour of the worm
	const Uint32 black = SDL_MapRGB(bmpSurface->format, 0, 0, 0);
	int width = MIN(MIN(bmpSurface->w, bmpNormal->w), bmpMirrored->w);

	// Initialize pixel functions
	PixelGet& getter = getPixelGetFunc(bmpSurface.get());
	PixelPut& putnormal = getPixelPutFunc(bmpNormal.get());
	PixelPut& putmirr = getPixelPutFunc(bmpMirrored.get());
	Uint8 *surfrow = GetPixelAddr(bmpSurface.get(), 0, 0);
	Uint8 *normalrow = GetPixelAddr(bmpNormal.get(), 0, 0);
	Uint8 *mirrrow = GetPixelAddr(bmpMirrored.get(), width - 1, 0);

	for (int y = 0; y < iFrameHeight; ++y) {

		Uint8 *surfpx = surfrow;
		Uint8 *normalpx = normalrow;
		Uint8 *mirrpx = mirrrow;
		for (int x = 0; x < width; ++x, 
			surfpx += bmpSurface->format->BytesPerPixel,
			normalpx += bmpNormal->format->BytesPerPixel,
			mirrpx -= bmpMirrored->format->BytesPerPixel) {

			// Use the mask to check what colours to ignore
			Uint32 pixel = getter.get(surfpx);
			Uint32 mask = getter.get(surfpx + iFrameHeight * bmpSurface->pitch);
            
            // Black means to just copy the colour but don't alter it
            if (EqualRGB(mask, black, bmpSurface.get()->format)) {
				putnormal.put(normalpx, pixel);
				putmirr.put(mirrpx, pixel);
                continue;
            }

            // Pink means just ignore the pixel completely
            if (IsTransparent(bmpSurface.get(), mask))
                continue;

			Color colorized(bmpSurface->format, pixel);

            // Must be white (or some over unknown colour)
			colorized.r = MIN(255, (colorized.r * colR) / 96);
			colorized.g = MIN(255, (colorized.g * colG) / 156);
			colorized.b = MIN(255, (colorized.b * colB) / 252);

			// Bit of a hack to make sure it isn't completey pink (see through)
			if(colorized.r == 255 && colorized.g == 0 && colorized.b == 255) {
				colorized.r = 240;
				colorized.b = 240;
			}

            // Put the colourised pixel
			putnormal.put(normalpx, colorized.get(bmpNormal->format));
			putmirr.put(mirrpx, colorized.get(bmpMirrored->format));
		}

		surfrow += bmpSurface->pitch;
		normalrow += bmpSurface->pitch;
		mirrrow += bmpMirrored->pitch;
		
		if(breakSignal) break;
	}

	UnlockSurface(bmpNormal);
	UnlockSurface(bmpMirrored);
	UnlockSurface(bmpSurface);

	if(breakSignal) return;
	
	// Regenerate the preview
	GeneratePreview();
}

////////////////////
// Returns number of frames in the skin animation
int CGameSkin::getFrameCount() const
{
	if (bmpSurface.get())
		return bmpSurface->w / iFrameWidth;
	else
		return 0;
}

std::string CGameSkin::toString() const {
	return sFileName;
}

bool CGameSkin::fromString( const std::string & str) {
	Change(str);
	return true;
}

REGISTER_CLASS(CGameSkin, LuaID<CustomVar>::value)
