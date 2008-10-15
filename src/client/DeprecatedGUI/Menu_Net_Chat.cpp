/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Net menu - News
// Created 12/8/02
// Jason Boettcher


#include "LieroX.h"
#include "Sounds.h"
#include "DeprecatedGUI/Graphics.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CBrowser.h"
#include "DeprecatedGUI/CListview.h"
#include "DeprecatedGUI/CTextbox.h"
#include "HTTP.h"
#include "StringUtils.h"
#include "AuxLib.h"
#include "libirc_rfcnumeric.h"


namespace DeprecatedGUI {

static CGuiLayout	cChat;
std::string strChatAddr;
NetworkSocket chatSocket;
bool opened = false;

enum {
	nc_Back = 0,
	nc_ChatText,
	nc_UserList,
	nc_ChatBrowser,
};


///////////////////
// Initialize the news net menu
bool Menu_Net_ChatInitialize(void)
{
	iNetMode = net_chat;

	// Setup the gui layout
	cChat.Shutdown();
	cChat.Initialize();

	cChat.Add( new CButton(BUT_BACK, tMenu->bmpButtons), nc_Back, 25,440, 50,15);

	cChat.Add( new CTextbox(), nc_ChatText, 25,  410, 590, tLX->cFont.GetHeight());
	cChat.Add( new CBrowser(), nc_UserList, 475, 140, 140, 260);
	cChat.Add( new CBrowser(), nc_ChatBrowser, 25, 140, 440, 260);

	FILE *fp = OpenGameFile("cfg/chatserver.txt", "r");
	if (fp)  {
		strChatAddr = ReadUntil(fp, '\n');
		fclose(fp);
	} else {
		strChatAddr = "irc.quakenet.org/#LieroX"; // Default
	}

	CBrowser *b = (CBrowser *)cChat.getWidget(nc_ChatBrowser);
	
	return true;
}

//////////////////
// Shutdown the news menu
void Menu_Net_ChatShutdown()
{
	cChat.Shutdown();
}


///////////////////
// The net news menu frame
void Menu_Net_ChatFrame(int mouse)
{
	gui_event_t *ev = NULL;


	// Process & Draw the gui
	ev = cChat.Process();
	cChat.Draw( VideoPostProcessor::videoSurface() );


	// Process any events
	if(ev) {

		switch(ev->iControlID) {

			// Back
			case nc_Back:
				if(ev->iEventMsg == BTN_MOUSEUP) {

					// Click!
					PlaySoundSample(sfxGeneral.smpClick);

					// Shutdown
					Menu_Net_ChatShutdown();

					// Back to main menu
					Menu_MainInitialize();
				}
				break;

		}

	}


	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());
}

void Menu_Net_Chat_Process()
{
	if( ! opened )
	{
		chatSocket = OpenReliableSocket(-1);
		opened = true;
	}
	
};

void Menu_Net_Chat_Send(const std::string & text)
{
};

}; // namespace DeprecatedGUI
