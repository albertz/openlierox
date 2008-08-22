/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#ifndef __CGUISKIN_H__SKINNED_GUI__
#define __CGUISKIN_H__SKINNED_GUI__

#include <SDL.h>
#include <string>
#include "InputEvents.h"


namespace SkinnedGUI {

class CWidget;
class CContainerWidget;
class CGuiSkinnedLayout;

bool	InitializeGuiSkinning();
void	ShutdownGuiSkinning();

class CGuiSkin : public EventListener
{
public:
	CGuiSkin();
	~CGuiSkin();

private:
	std::string	sName;
	std::string	sPath;
	std::string	sAuthor;

	CGuiSkinnedLayout *cPreviousLayout; // For GUI effects
	CGuiSkinnedLayout *cActiveLayout;

public:
	bool Load(const std::string& skin);
	bool OpenLayout(const std::string& layout);

	const std::string& getName()	{ return sName; }
	const std::string& getPath()	{ return sPath; }
	const std::string& getAuthor()	{ return sAuthor; }
	std::string getSkinFilePath(const std::string& file);

	// Event listener
	void OnEvent(SDL_Event *ev);

	void Frame();

	// Useful functions
	CWidget	*CreateWidgetByTagName(const std::string& tagname, CContainerWidget *parent, const std::string& id);
	FILE *OpenSkinFile(const std::string& filename, const char *mode);
};

extern CGuiSkin *cMainSkin; // The main game skin

}; // namespace SkinnedGUI

#endif
