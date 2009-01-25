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
#define SKIN_WIDTH	20
#define SKIN_HEIGHT	18
#define CPU_WIDTH 10
#define SHADOW_OPACITY 96 // TODO: move this to some more general header

class CWormSkin  {
public:
	CWormSkin();
	CWormSkin(const std::string& file);
	~CWormSkin();

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
	CWormSkin& operator=(const CWormSkin& oth);
	bool operator==(const CWormSkin& oth);
	bool operator!=(const CWormSkin& oth)  { return !(*this == oth); }

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

	SmartPointer<SDL_Surface>& getLeftImage()	{ return bmpMirrored; }
	SmartPointer<SDL_Surface>& getRightImage()	{ return bmpNormal; }
};

#endif
