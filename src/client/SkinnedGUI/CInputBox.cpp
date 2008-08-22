/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Input box
// Created 30/3/03
// Jason Boettcher


#include "LieroX.h"
#include "GfxPrimitives.h"
#include "SkinnedGUI/CInputBox.h"
#include "SkinnedGUI/CLabel.h"
#include "XMLutils.h"


namespace SkinnedGUI {

//////////////////
// Create
CInputbox::CInputbox(COMMON_PARAMS, int val, const std::string& _text) : CALL_DEFAULT_CONSTRUCTOR
{
	iKeyvalue = val;
	sText = _text;

	iType = wid_Inputbox;
}

CInputbox::CInputbox(const std::string &name, CContainerWidget *parent) : CWidget(name, parent)
{
	iKeyvalue = 0;
	sText = "";

	iType = wid_Inputbox;
}

///////////////////
// Draw the input box
void CInputbox::DoRepaint()
{
	CHECK_BUFFER;

	// Background
	cBackground.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());

	// Draw the text
	SDL_Rect r = { cBorder.getLeftW(), cBorder.getTopW(), getWidth() - cBorder.getLeftW() - cBorder.getRightW(),
		getHeight() - cBorder.getTopW() - cBorder.getBottomW()};

	DrawGameText(bmpBuffer.get(), sText, cFont, CTextProperties(&r, algCenter, algMiddle));

	// Border
	cBorder.Draw(bmpBuffer, 0, 0, getWidth(), getHeight());
}

////////////////////////
// Mouse up event
int	CInputbox::DoMouseUp(int x, int y, int dx, int dy, MouseButton button, const ModifiersState& modstate)
{
	if (RelInBox(x, y))
		cDialog = new CInputboxDialog(this); // HINT: freed by our parent guilayout

	CWidget::DoMouseUp(x, y, dx, dy, button, modstate);
	return WID_PROCESSED;
}

////////////////////
// Applies the given selector
void CInputbox::ApplySelector(const CSSParser::Selector& sel, const std::string& prefix)
{
	CWidget::ApplySelector(sel, prefix);
	cBorder.ApplySelector(sel, prefix);
	cBackground.ApplySelector(sel, prefix);
	cFont.ApplySelector(sel, prefix);
}

////////////////////
// Apply the given tag
void CInputbox::ApplyTag(xmlNodePtr node)
{
	CWidget::ApplyTag(node);

	sText = xmlGetString(node, "value", sText);
}



////////////////////
// Create
CInputboxDialog::CInputboxDialog(CInputbox *inpb) : 
CDialog(inpb->getName() + "_dialog", inpb->getParent(), "Change Input", true, false, false)
{
	new CLabel(STATIC, this, "Press any key/mouse");
	new CLabel(STATIC, this, "(Escape to cancel)");
	cInput = new CInput();
	cInputBox = inpb;
}

/////////////////////
// Destroy
CInputboxDialog::~CInputboxDialog()
{
	delete cInput;
}

/////////////////////
// Checks if an event has occured
void CInputboxDialog::Process()
{
	// Check if any input has been pressed
	std::string text;
	if (cInput->Wait(text))  {
		cInputBox->setValue(cInput->getData());
		cInputBox->setText(text);
		Close();
	}
}

/////////////////////
// Key up event
int	CInputboxDialog::DoKeyUp(UnicodeChar c, int keysym,  const ModifiersState& modstate)
{
	// Close on escape
	if (keysym == SDLK_ESCAPE)  {
		cInputBox->setValue(-1);
		cInputBox->setText("");
		Close();
	}

	return WID_PROCESSED;
}

}; // namespace SkinnedGUI
