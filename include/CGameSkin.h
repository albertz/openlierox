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
#include "game/Attr.h"

// Some basic defines
#define CPU_WIDTH 10
#define SHADOW_OPACITY 96 // TODO: move this to some more general header

struct GameSkinPreviewDrawer;
class CMap;
class CViewport;
struct SkinAction_Colorize;
struct SkinAction_Load;

#define WORM_SKIN_FRAME_WIDTH 32
#define WORM_SKIN_FRAME_HEIGHT 18
#define WORM_SKIN_FRAME_SPACING 4
#define WORM_SKIN_WIDTH 20
#define WORM_SKIN_HEIGHT 18

class CGameSkin : public CustomVar {
public:
	CGameSkin() : thread(NULL) { *this = WormSkin(); }
	CGameSkin(int fw, int fh, int fs, int sw, int sh);
	~CGameSkin();

	static CGameSkin WormSkin() {
		CGameSkin skin(WORM_SKIN_FRAME_WIDTH,
					   WORM_SKIN_FRAME_HEIGHT,
					   WORM_SKIN_FRAME_SPACING,
					   WORM_SKIN_WIDTH,
					   WORM_SKIN_HEIGHT);
		return skin;
	}

	CGameSkin(const CGameSkin& skin);
	CGameSkin& operator=(const CGameSkin& oth);

	virtual BaseObject* parentObject() const;

private:
	bool loaded;
	SmartPointer<SDL_Surface>	bmpSurface;
	SmartPointer<SDL_Surface>	bmpNormal;
	SmartPointer<SDL_Surface>	bmpMirrored;
	SmartPointer<SDL_Surface>	bmpShadow;
	SmartPointer<SDL_Surface>	bmpMirroredShadow;
	SmartPointer<SDL_Surface>	bmpPreview;
	ATTR(CGameSkin, std::string, sFileName, 1, { onUpdate = onFilenameUpdate; serverside = false; })
	ATTR(CGameSkin, Color,		iColor, 2, { defaultValue = Color(128, 128, 128); onUpdate = onColorUpdate; serverside = false; })
	ATTR(CGameSkin, Color,		iDefaultColor, 3, { defaultValue = Color(128, 128, 128); serverside = false; })
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
	friend void skin_load(const CGameSkin& s);

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
	void	DrawInternal(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored, bool blockUntilReady, bool half) const;

	static void onFilenameUpdate(BaseObject* base, const AttrDesc* attrDesc, ScriptVar_t oldValue);
	static void onColorUpdate(BaseObject* base, const AttrDesc* attrDesc, ScriptVar_t oldValue);

public:
	void	Draw(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored, bool blockUntilReady = false) const;
	void	DrawHalf(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored, bool blockUntilReady = false) const;
	void	DrawShadow(SDL_Surface *surf, int x, int y, int frame, bool mirrored) const;
	void	DrawShadowOnMap(CMap* cMap, CViewport* v, SDL_Surface *surf, int x, int y, int frame, bool mirrored) const;
	Color	renderColorAt(int x, int y, int frame, bool mirrored) const;
	
	void	Colorize(Color col);
	void	ColorizeDefault()	{ Colorize(iDefaultColor); }
	void	Change(const std::string& file);
	SmartPointer<DynDrawIntf> getPreview();

	std::string getFileName() const  { return sFileName; }
	int getBotIcon() const	{ return iBotIcon; }
	void setBotIcon(int _i) { iBotIcon = _i; }
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

#endif
