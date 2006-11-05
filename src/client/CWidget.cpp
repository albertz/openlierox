/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Widget class
// Created 5/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Setup the widget
void CWidget::Setup(int id, int x, int y, int w, int h)
{
	iID = id;
	iX = x;
	iY = y;
	iWidth = w;
	iHeight = h;
	iEnabled = true;	
}


///////////////////
// Returns true if a point is inside this widget
int CWidget::InBox(int x, int y)
{
	if(x > iX && x < iX+iWidth)
		if(y > iY && y < iY+iHeight)
			return true;

	return false;
}


///////////////////
// Redraw the buffer on the widget
void CWidget::redrawBuffer(void)
{
    Menu_redrawBufferRect(iX, iY, iWidth, iHeight);
}

//////////////////
// Setup the events
void CWidget::SetupEvents(generic_events_t *Events)
{
	strcpy(tEvents.onclick,Events->onclick);
	strcpy(tEvents.onmouseover,Events->onmouseover);
	strcpy(tEvents.onmouseout,Events->onmouseout);
	strcpy(tEvents.onmousedown,Events->onmousedown);
}

/////////////////
// Process the specified event
void CWidget::ProcessEvent(int Event)
{
	return;
	char *Code;
	CGuiLayout *Parent = (CGuiLayout *)cParent;

	// Get the event source code
	switch (Event)  {
	case OnMouseOver:
		Code = &tEvents.onmouseover[0];
	break;
	case OnMouseOut:
		Code = &tEvents.onmouseout[0];
	break;
	case OnMouseDown:
		Code = &tEvents.onmousedown[0];
	break;
	case OnClick:
		Code = &tEvents.onclick[0];
	break;
	}

	TrimSpaces(Code);

	// Nothing to process
	if (!strcmp("",Code))
		return;

	// Read the function name
	char Function[32];
	ReadUntil(Code,'(',Function);
	Code = Code+strlen(Function)+1;
	TrimSpaces(Function);

	if(!stricmp(Function,"change"))  {

		// Get the widget name
		char WidgetName[32];
		ReadUntil(Code,',',WidgetName);
		Code = Code + strlen(WidgetName)+1;
		TrimSpaces(WidgetName);

		// Get the widget ID
		int ID = -1;//cParser->LayoutWidgets[Parent->getID()].getID(WidgetName);
		if (ID == -1)
			return;

		// Read the new image path
		char ImagePath[32];
		ReadUntil(Code,')',ImagePath);
		TrimSpaces(ImagePath);

		CImage *img;
		CWidget *w = Parent->getWidget(ID);
		if(w->getType() == wid_Image)
			img = (CImage *)Parent->getWidget(ID);
		else
			return;

		img->Change(ImagePath);
	}

	else if(!stricmp(Function,"PlaySound"))  {
		char SoundPath[32];
		ReadUntil(Code,')',SoundPath);
		Code = Code + strlen(SoundPath) + 1;
		TrimSpaces(SoundPath);

// TODO: implement sound system
		// BASS_SamplePlay(SoundPath);
	}

	else if(!stricmp(Function,"Show"))  {
		// Get the widget name
		char WidgetName[32];
		ReadUntil(Code,',',WidgetName);
		Code = Code + strlen(WidgetName) +1;
		TrimSpaces(WidgetName);

		// Get the widget visibility parameter
		char Visible[8];
		ReadUntil(Code,')',Visible);
		Code = Code + strlen(Code) +1;
		TrimSpaces(Visible);

		// Convert to boolean
		bool bVisible = false;
		if (!stricmp("true",Visible))
			bVisible = true;
		if (!stricmp("yes",Visible))
			bVisible = true;
		if (!strcmp("1",Visible))
			bVisible = true;


		// Get the widget ID
		int ID = -1;//cParser->LayoutWidgets[Parent->getID()].getID(WidgetName);
		if (ID == -1)
			return;

		// Get the widget
		CWidget *w = Parent->getWidget(ID);
		if(!w)
			return;

		// Hide the widget
		w->setEnabled(bVisible);
	}

}

