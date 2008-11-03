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
	nc_EnableChat
};

/////////////////////////
// Re-fills the user list
void Menu_Net_ChatRefillUserList()
{
	const IRCClient *irc = GetGlobalIRC();

	// User list
	CListview *l = (CListview *)cChat.getWidget(nc_UserList);
	l->Clear();

	if (irc)  {
		if (!irc->isConnected())
			return;

		int i = 0;
		for (std::list<std::string>::const_iterator it = irc->getUserList().begin(); it != irc->getUserList().end(); it++) {
			l->AddItem(*it, i, tLX->clListView);
			l->AddSubitem(LVS_TEXT, *it, NULL, NULL);
		}
	}
}

///////////////////////////
// Register the IRC event listeners
void Menu_Net_ChatRegisterCallbacks()
{
	IRCClient *irc = GetGlobalIRC();

	// Setup the callbacks
	if (irc)  {
		irc->setNewMessageCallback(&Menu_Net_ChatNewMessage);
		irc->setDisconnectCallback(&Menu_Net_ChatDisconnect);
		irc->setConnectCallback(&Menu_Net_ChatConnect);
		irc->setUpdateUserListCallback(&Menu_Net_ChatUpdateUsers);
	}
}


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
	cChat.Add( new CListview(), nc_UserList, 475, 140, 140, 260);
	cChat.Add( new CBrowser(), nc_ChatText, 25, 140, 440, 260);
	cChat.Add( new CCheckbox(tLXOptions->bEnableChat), nc_EnableChat, 100, 440, 20, 20);
	cChat.Add( new CLabel("Enable", tLX->clNormalLabel), -1, 130, 440, 0, 0);

	// Messages
	CBrowser *b = (CBrowser *)cChat.getWidget(nc_ChatText);
	b->InitializeChatBox();

	IRCClient *irc = GetGlobalIRC();
	if (irc)
		for (std::list<IRCClient::IRCChatLine>::const_iterator it = irc->getMessageList().begin();
			it != irc->getMessageList().end(); it++)  {
				Menu_Net_ChatNewMessage(it->text, it->type);
		}

	Menu_Net_ChatRegisterCallbacks();
	Menu_Net_ChatRefillUserList();

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
		irc->clearConnectCallback();
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
					else  {
						InitializeIRC();
						Menu_Net_ChatRegisterCallbacks();
					}
				}
				break;

			case nc_UserList:
				if (ev->iEventMsg == LV_DOUBLECLK)  {
					CTextbox *txt = (CTextbox *)cChat.getWidget(nc_ChatInput);
					CListview *lsv = (CListview *)cChat.getWidget(nc_UserList);
					if (txt)  {
						std::string nick = lsv->getCurSIndex();

						// If there already is some text in the chatbox, insert the nick at the end, optionally adding a space
						if (txt->getText().size())  {
							if (isspace((uchar)*(txt->getText().rbegin())))
								txt->setText(txt->getText() + nick);
							else
								txt->setText(txt->getText() + " " + nick);

						// The textbox is empty, insert "nick: "
						} else {
							txt->setText(nick + ": ");
						}

						txt->setCurPos(txt->getText().size());

						cChat.FocusWidget(nc_ChatInput);
					}
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
		case IRCClient::IRC_TEXT_PRIVATE:
			brw->AddChatBoxLine(msg, tLX->clNetworkText, TXT_PRIVATE);
			break;
		default:
			brw->AddChatBoxLine(msg, tLX->clChatText, TXT_CHAT);
		}


		// Notify the user if the message contains his nick
		if (stringtolower(msg).find(stringtolower(GetGlobalIRC()->getNick())) != std::string::npos ||
			type == IRCClient::IRC_TEXT_PRIVATE)
			NotifyUserOnEvent();
	}
}

////////////////////////
// Chat disconnect (callback)
void Menu_Net_ChatDisconnect()
{
	CListview *lsv = (CListview *)cChat.getWidget(nc_UserList);
	if (lsv)  {
		lsv->Clear();
	}
}

////////////////////////
// Chat connect (callback)
void Menu_Net_ChatConnect()
{
	Menu_Net_ChatRefillUserList();
}

///////////////////////
// Update users (callback)
void Menu_Net_ChatUpdateUsers(const std::list<std::string>& users)
{
	CListview *lsv = (CListview *)cChat.getWidget(nc_UserList);
	if (lsv)  {
		lsv->Clear();

		// Add the users
		int i = 0;
		for (std::list<std::string>::const_iterator it = users.begin(); it != users.end(); it++)  {
			lsv->AddItem(*it, i, tLX->clListView);
			lsv->AddSubitem(LVS_TEXT, *it, NULL, NULL);
		}
	}
}

}; // namespace DeprecatedGUI
