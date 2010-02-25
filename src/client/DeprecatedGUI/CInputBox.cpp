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

#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "CInput.h"
#include "DeprecatedGUI/CInputBox.h"


namespace DeprecatedGUI {

///////////////////
// Draw the input box
void CInputbox::Draw(SDL_Surface * bmpDest)
{
	if (bRedrawMenu)
		Menu_redrawBufferRect(iX,iY, bmpImage.get()->w, MAX(bmpImage.get()->h, tLX->cFont.GetHeight()));

	int y = bMouseOver ? 17 : 0;
	DrawImageAdv(bmpDest,bmpImage, 0, y, iX, iY, bmpImage.get()->w, 17);
	bMouseOver = false;
    tLX->cFont.DrawCentre(bmpDest, iX+25, iY+1, tLX->clWhite, sText);
}

CInputbox * CInputbox::InputBoxSelected = NULL;
std::string CInputbox::InputBoxLabel;

CWidget * CInputbox::WidgetCreator( const std::vector< ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
{
	CInputbox * w = new CInputbox( 0, "", tMenu->bmpInputbox, p[0].str.get() );
	w->sVar = CScriptableVars::GetVarP<std::string>( p[1].str.get() );
	if( w->sVar )
		w->setText( *w->sVar );
	layout->Add( w, id, x, y, dx, dy );
	return w;
};

static bool CInputbox_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "inputbox", & CInputbox::WidgetCreator )
							( "name", SVT_STRING )
							( "var", SVT_STRING );
	
void CInputbox::ProcessGuiSkinEvent(int iEvent) 
{
	if( iEvent == CGuiSkin::SHOW_WIDGET )
	{
		if( sVar )
			setText( *sVar );
	}
	if( iEvent == INB_MOUSEUP )
	{
		if( sVar )
		{
			InputBoxSelected = this;
			InputBoxLabel = getName();
			CGuiSkin::CallbackHandler c;
			c.Init( "GUI.ChildDialog(InputBoxDialog)", this );
			c.Call();
		}
	}
}

CInputboxInput::CInputboxInput(): CInputbox( 0, "", tMenu->bmpInputbox, "" )
{
	CInput::InitJoysticksTemp(); // for supporting joystick in CInput::Wait
}

CInputboxInput::~CInputboxInput()
{
	CInput::UnInitJoysticksTemp();
	//CGuiSkin::DeRegisterUpdateCallback( this ); // Called in CWidget::~CWidget()
}

void CInputboxInput::ProcessGuiSkinEvent(int iEvent)
{
	if( iEvent ==  CGuiSkin::SHOW_WIDGET )
	{
		iSkipFirstFrame = 1;
		CGuiSkin::RegisterUpdateCallback( & UpdateCallback, "", this );
	}
}

void CInputboxInput::UpdateCallback( const std::string & param, CWidget * source )
{ 
	CInputboxInput * in = (CInputboxInput *) source;
	if( in->iSkipFirstFrame )
	{
		in->iSkipFirstFrame = 0;
		return;
	};
	std::string s;
	CInput::Wait( s );
	if( s != "" )
	{
		in->iSkipFirstFrame = 1;
		if( InputBoxSelected != NULL )
			InputBoxSelected->setText(s);
		CGuiSkin::CallbackHandler c;
		c.Init( "GUI.ExitDialog()", source );
		c.Call();
		CGuiSkin::DeRegisterUpdateCallback( source );
	};
};

CWidget * CInputboxInput::WidgetCreator( const std::vector< ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
{
	CInputboxInput * w = new CInputboxInput();
	layout->Add( w, id, x, y, dx, dy );
	return w;
};

static bool InputboxLabel_Registered = CScriptableVars::RegisterVars("GUI")
		( CInputbox::InputBoxLabel, "InputBoxLabel" );

static bool CInputboxInput_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "inputbox_input", & CInputboxInput::WidgetCreator );

}; // namespace DeprecatedGUI
