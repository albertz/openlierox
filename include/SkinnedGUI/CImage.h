// OpenLieroX


// Image
// Created 29/10/06
// Dark Charlie

// code under LGPL
#if 0

#ifndef __CIMAGE_H__SKINNED_GUI__
#define __CIMAGE_H__SKINNED_GUI__

#include "SkinnedGUI/CWidget.h"


namespace SkinnedGUI {

// Image specific events
class CImage;
typedef void(CWidget::*ImageClickHandler)(CImage *sender, MouseButton button, const ModifiersState& modstate);
#define SET_IMGCLICK(image, func)	SET_EVENT(image, OnClick, ImageClickHandler, func)

class CImage : public CWidget {
public:
	// Constructor
	// TODO: this "cropping" isn't used at all, remove it?
	// Cropping is used in GUI skinning in one place, I thought it would be nice, yet it's safe to remove it.
	CImage(COMMON_PARAMS, const std::string& Path, int _cropX=0, int _cropY=0, int _cropW=0, int _cropH=0);

	CImage(COMMON_PARAMS, SDL_Surface *img);

private:
    // Attributes
	SDL_Surface	*tImage;
	std::string	sPath;
	int cropX, cropY, cropW, cropH;
	DECLARE_EVENT(OnClick, ImageClickHandler);

public:
	// Events
	EVENT_SETGET(OnClick, ImageClickHandler)

    // Methods

	std::string	getPath()		{ return sPath; }
	SDL_Surface		*getSurface()	{ return tImage; }
	void			Change(const std::string& Path);
	void			Change(SDL_Surface *bmpImg);

	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle() {}
};

}; // namespace SkinnedGUI

#endif  //  __CIMAGE_H__SKINNED_GUI__
#endif
