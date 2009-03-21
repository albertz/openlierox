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

// Some basic defines
#define CPU_WIDTH 10
#define SHADOW_OPACITY 96 // TODO: move this to some more general header

class CGameSkin  {
public:
	CGameSkin(int fw, int fh, int fs, int sw, int sh);
	CGameSkin(const CGameSkin& skin);
	CGameSkin(const std::string& file, int fw, int fh, int fs, int sw, int sh);
	~CGameSkin();

private:
	SmartPointer<SDL_Surface>	bmpSurface;
	SmartPointer<SDL_Surface>	bmpNormal;
	SmartPointer<SDL_Surface>	bmpMirrored;
	SmartPointer<SDL_Surface>	bmpShadow;
	SmartPointer<SDL_Surface>	bmpMirroredShadow;
	SmartPointer<SDL_Surface>	bmpPreview;
	std::string					sFileName;
	Uint32						iColor;
	Uint32						iDefaultColor;
	bool						bColorized;
	int							iBotIcon;
	int							iFrameWidth;  // Width of one frame
	int							iFrameHeight;  // Height of one frame
	int							iFrameSpacing;  // Gap between two frames
	int							iSkinWidth;  // Width of the skin itself
	int							iSkinHeight;  // Height of the skin itself

private:
	bool	PrepareNormalSurface();
	bool	PrepareShadowSurface();
	bool	PrepareColorizedSurface();
	bool	PreparePreviewSurface();
	bool	PrepareMirrorSurface();
	void	GenerateNormalSurface();
	void	GenerateShadow();
	void	GeneratePreview();
	void	GenerateMirroredImage();

public:
	CGameSkin& operator=(const CGameSkin& oth);
	bool operator==(const CGameSkin& oth);
	bool operator!=(const CGameSkin& oth)  { return !(*this == oth); }

	void	Draw(SDL_Surface *surf, int x, int y, int frame, bool draw_cpu, bool mirrored);
	void	DrawShadow(SDL_Surface *surf, int x, int y, int frame, bool mirrored);
	void	Colorize(Uint32 col);
	void	RemoveColorization()	{ Colorize(iDefaultColor); }
	void	Change(const std::string& file);
	SmartPointer<SDL_Surface>& getPreview()	{ return bmpPreview; }

	const std::string& getFileName() const  { return sFileName; }
	int getBotIcon() const	{ return iBotIcon; }
	void setBotIcon(int _i) { iBotIcon = _i; }
	Uint32 getColor() const	{ return iColor; }
	Uint32 getDefaultColor() const	{ return iDefaultColor; }
	void setDefaultColor(Uint32 _c)	{ iDefaultColor = _c; }
	int getFrameCount() const;
	int getSkinWidth() const { return iSkinWidth; }
	int getSkinHeight() const { return iSkinHeight; }

	SmartPointer<SDL_Surface>& getLeftImage()	{ return bmpMirrored; }
	SmartPointer<SDL_Surface>& getRightImage()	{ return bmpNormal; }
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
	CWormSkin(const CWormSkin& oth) : CGameSkin(oth) { }
	CWormSkin(const std::string& file) : CGameSkin(file, 
		WORM_SKIN_FRAME_WIDTH, 
		WORM_SKIN_FRAME_HEIGHT,
		WORM_SKIN_FRAME_SPACING,
		WORM_SKIN_WIDTH,
		WORM_SKIN_HEIGHT) {}
};

#endif
