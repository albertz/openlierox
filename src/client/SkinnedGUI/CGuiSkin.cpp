/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


#include "SkinnedGUI/CGuiSkin.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "AuxLib.h"
#include "Cursor.h"

#include "SkinnedGUI/CWidget.h"

// Widgets
#include "SkinnedGUI/CAnimation.h"
#include "SkinnedGUI/CButton.h"
#include "SkinnedGUI/CCheckbox.h"
#include "SkinnedGUI/CCombobox.h"
#include "SkinnedGUI/CDialog.h"
#include "SkinnedGUI/CGuiSkinnedLayout.h"
#include "SkinnedGUI/CImageButton.h"
#include "SkinnedGUI/CInputBox.h"
#include "SkinnedGUI/CLabel.h"
#include "SkinnedGUI/CLine.h"
#include "SkinnedGUI/CListview.h"
#include "SkinnedGUI/CMapEditor.h"
#include "SkinnedGUI/CMarquee.h"
#include "SkinnedGUI/CMenu.h"
#include "SkinnedGUI/CMinimap.h"
#include "SkinnedGUI/CProgressbar.h"
#include "SkinnedGUI/CScrollbar.h"
#include "SkinnedGUI/CSlider.h"
#include "SkinnedGUI/CTabControl.h"
#include "SkinnedGUI/CTextbox.h"
#include "SkinnedGUI/CToggleButton.h"





namespace SkinnedGUI {

CGuiSkin *cMainSkin = NULL;

#define SKIN_DIRECTORY (std::string("gui_skins/"))

///////////////
// Initialize skinning
bool InitializeGuiSkinning()
{
	if (!cMainSkin)
		cMainSkin = new CGuiSkin();

	if (tLXOptions)
		return cMainSkin->Load(tLXOptions->sSkinPath);
	return false;
}

//////////////////
// Shutdown skinning
void ShutdownGuiSkinning()
{
	if (cMainSkin)
		delete cMainSkin;
	cMainSkin = NULL;
}

/////////////
// Constructor
CGuiSkin::CGuiSkin()
{
	sPath = SKIN_DIRECTORY + "default";
	sName = "Default";
	sAuthor = "OpenLieroX Development Team";

	cActiveLayout = NULL;
	cPreviousLayout = NULL;

	iBufferCount = 1;

	// Listen to the events
	sdlEvents[SDL_KEYDOWN].handler() += getEventHandler(this, &CGuiSkin::SDL_OnKeyDown);
	sdlEvents[SDL_KEYUP].handler() += getEventHandler(this, &CGuiSkin::SDL_OnKeyUp);
	sdlEvents[SDL_MOUSEMOTION].handler() += getEventHandler(this, &CGuiSkin::SDL_OnMouseMotion);
	sdlEvents[SDL_MOUSEBUTTONDOWN].handler() += getEventHandler(this, &CGuiSkin::SDL_OnMouseButtonDown);
	sdlEvents[SDL_MOUSEBUTTONUP].handler() += getEventHandler(this, &CGuiSkin::SDL_OnMouseButtonUp);
	
	onAddWidget.handler() = getEventHandler(this, &CGuiSkin::SDL_OnAddWidget);;
	onDestroyWidget.handler() = getEventHandler(this, &CGuiSkin::SDL_OnDestroyWidget);
}

//////////////
// Destructor
CGuiSkin::~CGuiSkin()
{
	sdlEvents[SDL_KEYDOWN].handler() -= getEventHandler(this, &CGuiSkin::SDL_OnKeyDown);
	sdlEvents[SDL_KEYUP].handler() -= getEventHandler(this, &CGuiSkin::SDL_OnKeyUp);
	sdlEvents[SDL_MOUSEMOTION].handler() -= getEventHandler(this, &CGuiSkin::SDL_OnMouseMotion);
	sdlEvents[SDL_MOUSEBUTTONDOWN].handler() -= getEventHandler(this, &CGuiSkin::SDL_OnMouseButtonDown);
	sdlEvents[SDL_MOUSEBUTTONUP].handler() -= getEventHandler(this, &CGuiSkin::SDL_OnMouseButtonUp);

	// No effects, just destroy
	if (cActiveLayout)
		delete cActiveLayout;
	if (cPreviousLayout)
		delete cPreviousLayout;
}

////////////////
// Load the GUI skin
bool CGuiSkin::Load(const std::string& skin)
{
	sPath = SKIN_DIRECTORY + skin;

	// TODO: author + name
	// TODO: verify that the important files exist

	return true;
}

//////////////////
// Open a layout
bool CGuiSkin::OpenLayout(const std::string &layout)
{
	// Because of the GUI effects we have to remember the old layout for a while
	if (cActiveLayout)  {
		cActiveLayout->Destroy();
		
		if (cPreviousLayout) // Another layout still effecting, just quit it
			delete cPreviousLayout;

		cPreviousLayout = cActiveLayout;
	}

	// Load the new layout
	cActiveLayout = new CGuiSkinnedLayout();
	if (cActiveLayout->Load(layout, *this))  {
		cActiveLayout->DoCreate();
		return true;
	}

	return false;
}

////////////////////
// Converts the path that is relative to the skin to path relative to OLX
std::string CGuiSkin::getSkinFilePath(const std::string& file)
{
	// Check
	if (!sPath.size())
		return file;

	return JoinPaths(sPath, file);
}

///////////////////////
// Opens a file from the current skin
FILE *CGuiSkin::OpenSkinFile(const std::string &filename, const char *mode)
{
	if (!filename.size())
		return NULL;

	return OpenGameFile(getSkinFilePath(filename), mode);
}

/////////////////////
// Creates a new widget based on the tag name, returns NULL for unknown widgets
CWidget *CGuiSkin::CreateWidgetByTagName(const std::string& tagname, CContainerWidget *parent, const std::string& id)
{
	if (stringcaseequal(tagname, CAnimation::tagName()))
		return new CAnimation(id, parent);
	else if (stringcaseequal(tagname, CButton::tagName()))
		return new CButton(id, parent);
	else if (stringcaseequal(tagname, CCheckbox::tagName()))
		return new CCheckbox(id, parent);
	else if (stringcaseequal(tagname, CCombobox::tagName()))
		return new CCombobox(id, parent);
	else if (stringcaseequal(tagname, CDialog::tagName()))
		return new CDialog(id, parent);
	else if (stringcaseequal(tagname, CGuiSkinnedLayout::tagName()))
		return new CGuiSkinnedLayout(id, parent);
	else if (stringcaseequal(tagname, CImageButton::tagName()))
		return new CImageButton(id, parent);
	else if (stringcaseequal(tagname, CInputbox::tagName()))
		return new CInputbox(id, parent);
	else if (stringcaseequal(tagname, CLabel::tagName()))
		return new CLabel(id, parent);
	else if (stringcaseequal(tagname, CLine::tagName()))
		return new CLine(id, parent);
	else if (stringcaseequal(tagname, CListview::tagName()))
		return new CListview(id, parent);
	else if (stringcaseequal(tagname, CMapEditor::tagName()))
		return new CMapEditor(id, parent);
	else if (stringcaseequal(tagname, CMarquee::tagName()))
		return new CMarquee(id, parent);
	else if (stringcaseequal(tagname, CMenu::tagName()))
		return new CMenu(id, parent);
	else if (stringcaseequal(tagname, CMinimap::tagName()))
		return new CMinimap(id, parent);
	else if (stringcaseequal(tagname, CProgressBar::tagName()))
		return new CProgressBar(id, parent);
	else if (stringcaseequal(tagname, CScrollbar::tagName()))
		return new CScrollbar(id, parent);
	else if (stringcaseequal(tagname, CSlider::tagName()))
		return new CSlider(id, parent);
	else if (stringcaseequal(tagname, CTabControl::tagName()))
		return new CTabControl(id, parent);
	else if (stringcaseequal(tagname, CTextbox::tagName()))
		return new CTextbox(id, parent);
	else if (stringcaseequal(tagname, CToggleButton::tagName()))
		return new CToggleButton(id, parent);
	else
		return NULL; // Unknown widget
}

//////////////////
// Draws the content and processes things
void CGuiSkin::Frame()
{
	// Reset the game cursor
	SetGameCursor(CUR_ARROW);

	if (cPreviousLayout)  {
		if (cPreviousLayout->needsRepaint())
			cPreviousLayout->DoRepaint();
		cPreviousLayout->Process();
		cPreviousLayout->Draw(VideoPostProcessor::videoSurface(), 0, 0);
	}

	cActiveLayout->Process();
	if (cActiveLayout->needsRepaint())
		cActiveLayout->DoRepaint();

	cActiveLayout->Draw(VideoPostProcessor::videoSurface(), 0, 0);
	DrawCursor(VideoPostProcessor::videoSurface());
	doVideoFrameInMainThread();
}

void CGuiSkin::SDL_OnKeyDown(SDL_Event *ev) {
	if(!cActiveLayout) return;
	// TODO: This is a hack and could not work later anymore. Please fix that.
	// (Any event handler should *never* depend on a state, like GetKeyboard().)
	const KeyboardEvent& key = GetKeyboard()->keyQueue[GetKeyboard()->queueLength - 1];
	cActiveLayout->DoKeyDown(key.ch, key.sym, key.state);
}

void CGuiSkin::SDL_OnKeyUp(SDL_Event *ev) {
	if(!cActiveLayout) return;
	// TODO: This is a hack and could not work later anymore. Please fix that.
	// (Any event handler should *never* depend on a state, like GetKeyboard().)
	const KeyboardEvent& key = GetKeyboard()->keyQueue[GetKeyboard()->queueLength - 1];
	cActiveLayout->DoKeyUp(key.ch, key.sym, key.state);
}

void CGuiSkin::SDL_OnMouseMotion(SDL_Event* ev) {
	if(!cActiveLayout) return;
	cActiveLayout->DoMouseMove(ev->motion.x, ev->motion.y, ev->motion.xrel, ev->motion.yrel, 
		ev->motion.state != 0, SDLButtonStateToMouseButton(ev->motion.state), *GetCurrentModstate());
}

void CGuiSkin::SDL_OnMouseButtonDown(SDL_Event* ev) {
	if(!cActiveLayout) return;
	// Get the button
	switch (ev->button.button)  {

/*
	// TODO: mouse wheel
	
	// HINT: mouse scroll up/down are reported as "clicks"
	case SDL_BUTTON_WHEELDOWN:
		cActiveLayout->DoMouseWheelDown(ev->button.x, ev->button.y, GetMouse()->deltaX, GetMouse()->deltaY, *GetCurrentModstate());
	return;
	case SDL_BUTTON_WHEELUP:
		cActiveLayout->DoMouseWheelUp(ev->button.x, ev->button.y, GetMouse()->deltaX, GetMouse()->deltaY, *GetCurrentModstate());
	return;
*/
	
	// Normal button
	default:
		cActiveLayout->DoMouseDown(ev->button.x, ev->button.y, GetMouse()->deltaX, GetMouse()->deltaY,
			SDLButtonToMouseButton(ev->button.button), *GetCurrentModstate());
	}
}

void CGuiSkin::SDL_OnMouseButtonUp(SDL_Event* ev) {
	if(!cActiveLayout) return;
	cActiveLayout->DoMouseUp(ev->button.x, ev->button.y, GetMouse()->deltaX, GetMouse()->deltaY,
		SDLButtonToMouseButton(ev->button.button), *GetCurrentModstate());

}

void CGuiSkin::SDL_OnAddWidget(WidgetData ev) {
	if(!cActiveLayout) return;
	cActiveLayout->DoChildAddEvent(ev.widget);	
}

void CGuiSkin::SDL_OnDestroyWidget(WidgetData ev) {
	if(!cActiveLayout) return;
	cActiveLayout->DoChildDestroyEvent(ev.widget);
}

}; // namespace SkinnedGUI
