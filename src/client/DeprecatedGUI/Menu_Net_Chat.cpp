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
#include "DeprecatedGUI/CLabel.h"
#include "DeprecatedGUI/CCheckbox.h"
#include "IRC.h"
#include "StringUtils.h"
#include "AuxLib.h"
#include "NotifyUser.h"


namespace DeprecatedGUI {

CGuiLayout	cChat;
static bool cChatGuiInitialized;

enum {
	nc_Back = 0,
	nc_ChatText,
	nc_UserList,
	nc_ChatInput,
	nc_EnableChat,
	nc_EnableChatNotify,
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

	cChat.Add( new CTextbox(), nc_ChatInput, 25,  410, 590, tLX->cFont.GetHeight());
	cChat.Add( new CBrowser(), nc_UserList, 475, 140, 140, 260);
	cChat.Add( new CBrowser(), nc_ChatText, 25, 140, 440, 260);
	cChat.Add( new CCheckbox(tLXOptions->bEnableChat), nc_EnableChat, 100, 440, 20, 20);
	cChat.Add( new CLabel("Enable", tLX->clNormalLabel), -1, 130, 440, 0, 0);
	cChat.Add( new CCheckbox(tLXOptions->bEnableChatNotification), nc_EnableChatNotify, 200, 440, 20, 20);
	cChat.Add( new CLabel("Notify when my name appears in chat", tLX->clNormalLabel), -1, 230, 440, 0, 0);

	// Messages
	CBrowser *b = (CBrowser *)cChat.getWidget(nc_ChatText);
	b->InitializeChatBox();

	IRCClient *irc = GetGlobalIRC();
	if (irc)
		for (std::list<IRCClient::IRCChatLine>::const_iterator it = irc->getMessageList().begin();
			it != irc->getMessageList().end(); it++)  {
				Menu_Net_ChatNewMessage(it->text, it->type);
		}
	
	// User list
	b = (CBrowser *)cChat.getWidget(nc_UserList);
	b->InitializeChatBox();

	if (irc)
		for (std::list<std::string>::const_iterator it = irc->getUserList().begin(); it != irc->getUserList().end(); it++)
			b->AddChatBoxLine( *it, tLX->clChatText, TXT_CHAT );

	// Setup the callbacks
	if (irc)  {
		irc->setNewMessageCallback(&Menu_Net_ChatNewMessage);
		irc->setDisconnectCallback(&Menu_Net_ChatDisconnect);
		irc->setUpdateUserListCallback(&Menu_Net_ChatUpdateUsers);
	}

	cChatGuiInitialized = true;
	return true;
}

//////////////////
// Shutdown the news menu
void Menu_Net_ChatShutdown()
{
	// Clear the callbacks
	IRCClient *irc = GetGlobalIRC();
	if (irc)  {
		irc->clearNewMessageCallback();
		irc->clearDisconnectCallback();
		irc->clearUpdateUserListCallback();
	}

	cChatGuiInitialized = false;
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

			case nc_ChatInput:
				if(ev->iEventMsg == TXT_ENTER) {
					
					std::string text;
					cChat.SendMessage(nc_ChatInput, TXS_GETTEXT, &text, 0); // Get the text

                    if(text.size() == 0) // Don't send empty messages
                        break;

					if(GetGlobalIRC() && GetGlobalIRC()->sendChat(text))
						cChat.SendMessage(nc_ChatInput, TXS_SETTEXT, "", 0); // Clear the text box
				}
				break;

			case nc_EnableChat:
				if(ev->iEventMsg == CHK_CHANGED) {
					
					PlaySoundSample(sfxGeneral.smpClick);
					tLXOptions->bEnableChat = cChat.SendMessage(nc_EnableChat, CKM_GETCHECK, 1, 1) != 0;
					if (!tLXOptions->bEnableChat)
						ShutdownIRC();
					else
						InitializeIRC();
				}
				break;

			case nc_EnableChatNotify:
				if(ev->iEventMsg == CHK_CHANGED) {
					
					PlaySoundSample(sfxGeneral.smpClick);
					tLXOptions->bEnableChatNotification = cChat.SendMessage(nc_EnableChatNotify, CKM_GETCHECK, 1, 1) != 0;
				}
				break;
			
		}

	}


	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());
}


/////////////////////////
// New message (callback)
void Menu_Net_ChatNewMessage(const std::string& msg, int type)
{
	CBrowser *brw = (CBrowser *)cChat.getWidget(nc_ChatText);
	if (brw && GetGlobalIRC())  {
		switch (type)  {
		case IRCClient::IRC_TEXT_CHAT:
			brw->AddChatBoxLine(msg, tLX->clChatText, TXT_CHAT);
			break;
		case IRCClient::IRC_TEXT_NOTICE:
			brw->AddChatBoxLine(msg, tLX->clNotice, TXT_NOTICE);
			break;
		case IRCClient::IRC_TEXT_ACTION:
			brw->AddChatBoxLine(msg, tLX->clNetworkText, TXT_CHAT);
			break;
		default:
			brw->AddChatBoxLine(msg, tLX->clChatText, TXT_CHAT);
		}


		// Notify the user if the message contains his nick
		if (tLXOptions->bEnableChatNotification && 
				stringtolower(msg).find(stringtolower(GetGlobalIRC()->getNick())) != std::string::npos)
			NotifyUserOnEvent();
	}
}

////////////////////////
// Chat disconnect (callback)
void Menu_Net_ChatDisconnect()
{
	CBrowser *brw = (CBrowser *)cChat.getWidget(nc_UserList);
	if (brw)  {
		brw->InitializeChatBox("");
	}
}

///////////////////////
// Update users (callback)
void Menu_Net_ChatUpdateUsers(const std::list<std::string>& users)
{
	CBrowser *brw = (CBrowser *)cChat.getWidget(nc_UserList);
	if (brw)  {
		brw->InitializeChatBox(""); // Clear

		// Add the users
		for (std::list<std::string>::const_iterator it = users.begin(); it != users.end(); it++)
			brw->AddChatBoxLine(*it, tLX->clNormalText, TXT_NORMAL);
	}
}

}; // namespace DeprecatedGUI
