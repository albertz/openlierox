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

#ifndef __CWORMSKIN_H__
#define __CWORMSKIN_H__

#include "GfxPrimitives.h"
#include "SmartPointer.h"
#include "Color.h"
#include "ThreadPool.h"
#include "DynDraw.h"
#include "util/CustomVar.h"

// Some basic defines
#define CPU_WIDTH 10
#define SHADOW_OPACITY 96 // TODO: move this to some more general header

struct GameSkinPreviewDrawer;
class CMap;
class CViewport;
struct SkinAction_Colorize;
struct SkinAction_Load;

class CGameSkin : CustomVar {
public:
	CGameSkin(int fw, int fh, int fs, int sw, int sh);
	CGameSkin(const std::string& file, int fw, int fh, int fs, int sw, int sh);
	~CGameSkin();
	
	CGameSkin(const CGameSkin& skin);
	CGameSkin& operator=(const CGameSkin& oth);

private:
	SmartPointer<SDL_Surface>	bmpSurface;
	SmartPointer<SDL_Surface>	bmpNormal;
	SmartPointer<SDL_Surface>	bmpMirrored;
	SmartPointer<SDL_Surface>	bmpShadow;
	SmartPointer<SDL_Surface>	bmpMirroredShadow;
	SmartPointer<SDL_Surface>	bmpPreview;
	std::string					sFileName;
	Color						iColor;
	Color						iDefaultColor;
	bool						bColorized;
	int							iBotIcon;
	int							iFrameWidth;  // Width of one frame
	int							iFrameHeight;  // Height of one frame
	int							iFrameSpacing;  // Gap between two frames
	int							iSkinWidth;  // Width of the skin itself
	int							iSkinHeight;  // Height of the skin itself
	
	struct Thread;
	Thread* thread;
	friend struct Thread;
	friend struct GameSkinPreviewDrawer;
	friend struct SkinAction_Colorize;
	friend struct SkinAction_Load;
	
private:
	void init(int fw, int fh, int fs, int sw, int sh); void uninit();
	void createSurfaces();
	bool	PrepareNormalSurface();
	bool	PrepareShadowSurface();
	bool	PrepareColorizedSurface();
	bool	PreparePreviewSurface();
	bool	PrepareMirrorSurface();
	void	GenerateNormalSurface();
	void	GenerateShadow();
	void	GeneratePreview();
	void	GenerateMirroredImage();
	void	Colorize_Execute(bool& breakSignal);
	void	Load_Execute(bool& breakSignal);
	void	DrawInternal(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored, bool blockUntilReady, bool half);

public:
	bool operator==(const CGameSkin& oth);
	bool operator!=(const CGameSkin& oth)  { return !(*this == oth); }

	void	Draw(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored, bool blockUntilReady = false);
	void	DrawHalf(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored, bool blockUntilReady = false);
	void	DrawShadow(SDL_Surface *surf, int x, int y, int frame, bool mirrored);
	void	DrawShadowOnMap(CMap* cMap, CViewport* v, SDL_Surface *surf, int x, int y, int frame, bool mirrored);
	Color	renderColorAt(int x, int y, int frame, bool mirrored);
	
	void	Colorize(Color col);
	void	RemoveColorization()	{ Colorize(iDefaultColor); }
	void	Change(const std::string& file);
	SmartPointer<DynDrawIntf> getPreview();

	const std::string& getFileName() const  { return sFileName; }
	int getBotIcon() const	{ return iBotIcon; }
	void setBotIcon(int _i) { iBotIcon = _i; }
	Color getColor() const	{ return iColor; }
	Color getDefaultColor() const	{ return iDefaultColor; }
	void setDefaultColor(Color _c)	{ iDefaultColor = _c; }
	int getFrameCount() const;
	int getSkinWidth() const { return iSkinWidth; }
	int getSkinHeight() const { return iSkinHeight; }

	// --- CustomVar ---

	virtual CustomVar* copy() const { return new CGameSkin(*this); }
	virtual bool operator==(const CustomVar& o) const;
	virtual bool operator<(const CustomVar& o) const;
	virtual std::string toString() const;
	virtual bool fromString( const std::string & str);
};

#define WORM_SKIN_FRAME_WIDTH 32
#define WORM_SKIN_FRAME_HEIGHT 18
#define WORM_SKIN_FRAME_SPACING 4
#define WORM_SKIN_WIDTH 20
#define WORM_SKIN_HEIGHT 18

class CWormSkin : public CGameSkin  {
public:
	CWormSkin() : CGameSkin(WORM_SKIN_FRAME_WIDTH,
		WORM_SKIN_FRAME_HEIGHT,
		WORM_SKIN_FRAME_SPACING,
		WORM_SKIN_WIDTH,
		WORM_SKIN_HEIGHT)  {}
	CWormSkin(const std::string& file) : CGameSkin(file, 
		WORM_SKIN_FRAME_WIDTH, 
		WORM_SKIN_FRAME_HEIGHT,
		WORM_SKIN_FRAME_SPACING,
		WORM_SKIN_WIDTH,
		WORM_SKIN_HEIGHT) {}

	CWormSkin(const CWormSkin& oth) : CGameSkin(oth) {}
	CWormSkin& operator=(const CWormSkin& oth) { this->CGameSkin::operator=(oth); return *this; }
};

#endif
