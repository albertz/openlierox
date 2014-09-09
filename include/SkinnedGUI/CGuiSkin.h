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

class CGuiSkin
{
public:
	CGuiSkin();
	~CGuiSkin();

private:
	std::string	sName;
	std::string	sPath;
	std::string	sAuthor;
	int iBufferCount;  // For double-buffering, triple-buffering etc.

	CGuiSkinnedLayout *cPreviousLayout; // For GUI effects
	CGuiSkinnedLayout *cActiveLayout;

public:
	bool Load(const std::string& skin);
	bool OpenLayout(const std::string& layout);

	const std::string& getName()	{ return sName; }
	const std::string& getPath()	{ return sPath; }
	const std::string& getAuthor()	{ return sAuthor; }
	std::string getSkinFilePath(const std::string& file);
	void setBufferCount(int c)		{ iBufferCount = c; }
	int getBufferCount() const		{ return iBufferCount; }

	// public events
	struct WidgetData {
		void* owner;
		CWidget* widget;
		WidgetData(void* o, CWidget* w) : owner(o), widget(w) {}
	};
	Event<WidgetData> onAddWidget;
	Event<WidgetData> onDestroyWidget;

	// Event handlers
	void SDL_OnKeyDown(SDL_Event *ev);
	void SDL_OnKeyUp(SDL_Event *ev);
	void SDL_OnMouseMotion(SDL_Event* ev);
	void SDL_OnMouseButtonDown(SDL_Event* ev);
	void SDL_OnMouseButtonUp(SDL_Event* ev);
	void SDL_OnMouseWheel(SDL_Event* ev);
	void SDL_OnAddWidget(WidgetData ev);
	void SDL_OnDestroyWidget(WidgetData ev);

	void Frame();

	// Useful functions
	CWidget	*CreateWidgetByTagName(const std::string& tagname, CContainerWidget *parent, const std::string& id);
	FILE *OpenSkinFile(const std::string& filename, const char *mode);
};

extern CGuiSkin *cMainSkin; // The main game skin

}; // namespace SkinnedGUI

#endif
