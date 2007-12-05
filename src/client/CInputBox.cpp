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
#include "Menu.h"
#include "GfxPrimitives.h"
#include "CInput.h"


///////////////////
// Draw the input box
void CInputbox::Draw(SDL_Surface *bmpDest)
{
	if (bRedrawMenu)
		Menu_redrawBufferRect(iX,iY, bmpImage->w, MAX(bmpImage->h, tLX->cFont.GetHeight()));

	int y=0;
	if(iMouseOver)
		y=17;
	DrawImageAdv(bmpDest,bmpImage, 0,y, iX,iY, bmpImage->w,17);
	iMouseOver = false;
    tLX->cFont.DrawCentre(bmpDest, iX+25, iY+1, tLX->clWhite, sText);
}

CInputbox * CInputbox::InputBoxSelected = NULL;
std::string CInputbox::InputBoxLabel;

CWidget * CInputbox::WidgetCreator( const std::vector< CGuiSkin::WidgetVar_t > & p )
{
	CInputbox * w = new CInputbox( 0, "", tMenu->bmpInputbox, p[0].s );
	w->sVar = CGuiSkin::GetVar( p[1].s, CGuiSkin::SVT_STRING ).s;
	if( w->sVar )
		w->setText( *w->sVar );
	return w;
};

static bool CInputbox_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "inputbox", & CInputbox::WidgetCreator )
							( "name", CGuiSkin::WVT_STRING )
							( "var", CGuiSkin::WVT_STRING );
	
void CInputbox::ProcessGuiSkinEvent(int iEvent) 
{
	if( iEvent == INB_MOUSEUP )
	{
		if( sVar )
		{
			InputBoxSelected = this;
			InputBoxLabel = getName();
			CGuiSkin::CallbackHandler c;
			c.Init( "GUI.ChildDialog(InputBoxDialog)", this );
			c.Call();
		};
	};
};

CInputboxInput::CInputboxInput(): CInputbox( 0, "", tMenu->bmpInputbox, "" )
{
};

CInputboxInput::~CInputboxInput()
{
	CGuiSkin::DeRegisterUpdateCallback( this );
};

void CInputboxInput::ProcessGuiSkinEvent(int iEvent)
{
	if( iEvent ==  CGuiSkin::SHOW_WIDGET )
	{
		iSkipFirstFrame = 1;
		CGuiSkin::RegisterUpdateCallback( & UpdateCallback, "", this );
	};
};

void CInputboxInput::UpdateCallback( const std::string & param, CWidget * source )
{ 
	CInputboxInput * in = (CInputboxInput *) source;
	if( in->iSkipFirstFrame )
	{
		in->iSkipFirstFrame = 0;
		return;
	};
	CInput input;
	std::string s;
	input.Wait( s );
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

CWidget * CInputboxInput::WidgetCreator( const std::vector< CGuiSkin::WidgetVar_t > & p )
{
	CInputboxInput * w = new CInputboxInput();
	return w;
};

static bool InputboxLabel_Registered = CGuiSkin::RegisterVars("GUI")
		( CInputbox::InputBoxLabel, "InputBoxLabel" );

static bool CInputboxInput_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "inputbox_input", & CInputboxInput::WidgetCreator );
