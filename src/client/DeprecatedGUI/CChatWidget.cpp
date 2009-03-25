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


#include "DeprecatedGUI/CChatWidget.h"
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

	enum {
		nc_ChatText,
		nc_UserList,
		nc_ChatInput,
	};


static CChatWidget * chatWidget = NULL; // Instance of CChatWidget

void ChatWidget_ChatRegisterCallbacks();
void ChatWidget_ChatDeregisterCallbacks();
void ChatWidget_ChatNewMessage(const std::string& msg, int type);

/////////////////////////
// Re-fills the user list
void ChatWidget_ChatRefillUserList()
{
	if( !chatWidget )
		return;
	const IRCClient *irc = GetGlobalIRC();

	// User list
	CListview *l = (CListview *)chatWidget->getWidget(nc_UserList);
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

CChatWidget::CChatWidget()
{
};

CChatWidget::~CChatWidget()
{
};

void CChatWidget::Create()
{
	if( chatWidget )
		warnings << "CChatWidget::CChatWidget(): more than one instance created" << endl;
	chatWidget = this;
	
	CGuiSkinnedLayout::SetOffset( iX, iY );
	
	if( iWidth < 170 || iHeight < tLX->cFont.GetHeight() + 30 )
		warnings << "CChatWidget::CChatWidget(): too small dimensions given" << endl;

	this->Add( new CBrowser(), nc_ChatText, 0, 0, iWidth - 145, iHeight - tLX->cFont.GetHeight() - 8 );
	this->Add( new CListview(), nc_UserList, iWidth - 140, 0, 140, iHeight);
	this->Add( new CTextbox(), nc_ChatInput, 0,  iHeight - tLX->cFont.GetHeight() - 2, iWidth - 145, tLX->cFont.GetHeight());

	// Messages
	CBrowser *b = (CBrowser *)this->getWidget(nc_ChatText);
	b->InitializeChatBox();

	IRCClient *irc = GetGlobalIRC();
	if (irc)
		for (std::list<IRCClient::IRCChatLine>::const_iterator it = irc->getMessageList().begin();
			it != irc->getMessageList().end(); it++)  {
				ChatWidget_ChatNewMessage(it->text, it->type);
		}

	ChatWidget_ChatRegisterCallbacks();
	ChatWidget_ChatRefillUserList();

	CGuiSkinnedLayout::Create();
}

void CChatWidget::Destroy()
{
	ChatWidget_ChatDeregisterCallbacks();
	chatWidget = NULL;
	CGuiSkinnedLayout::Destroy();
}

CChatWidget * CChatWidget::getInstance()
{
	return chatWidget;
};

void CChatWidget::EnableChat()
{
	ChatWidget_ChatRegisterCallbacks();
};
void CChatWidget::DisableChat()
{
	ChatWidget_ChatDeregisterCallbacks();
	((CListview *)chatWidget->getWidget(nc_UserList))->Clear();
};


void CChatWidget::ProcessChildEvent(int iEvent, CWidget * child)
{
	if( ! child )
		return;

	switch( child->getID() )
	{
			case nc_ChatInput:
				if(iEvent == TXT_ENTER) {
					
					CTextbox *txt = (CTextbox *)this->getWidget(nc_ChatInput);
					std::string text = txt->getText();

                    if(text.size() == 0) // Don't send empty messages
                        break;

					if(GetGlobalIRC() && GetGlobalIRC()->sendChat(text))
						txt->setText(""); // Clear the text box
				}
				break;

			case nc_UserList:
				if (iEvent == LV_DOUBLECLK)  {
					CTextbox *txt = (CTextbox *)this->getWidget(nc_ChatInput);
					CListview *lsv = (CListview *)this->getWidget(nc_UserList);
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

						this->FocusWidget(nc_ChatInput);
					}
				}
			break;
	}
}

void ChatWidget_ChatNewMessage(const std::string& msg, int type)
{
	if( !chatWidget )
		return;
	
	CBrowser *brw = (CBrowser *)chatWidget->getWidget(nc_ChatText);
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
void ChatWidget_ChatDisconnect()
{
	if( !chatWidget )
		return;
	CListview *lsv = (CListview *)chatWidget->getWidget(nc_UserList);
	if (lsv)  {
		lsv->Clear();
	}
}

////////////////////////
// Chat connect (callback)
void ChatWidget_ChatConnect()
{
	ChatWidget_ChatRefillUserList();
}

///////////////////////
// Update users (callback)
void ChatWidget_ChatUpdateUsers(const std::list<std::string>& users)
{
	if( !chatWidget )
		return;
	CListview *lsv = (CListview *)chatWidget->getWidget(nc_UserList);
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

///////////////////////////
// Register the IRC event listeners
void ChatWidget_ChatRegisterCallbacks()
{
	IRCClient *irc = GetGlobalIRC();

	// Setup the callbacks
	if (irc)  {
		irc->setNewMessageCallback(&ChatWidget_ChatNewMessage);
		irc->setDisconnectCallback(&ChatWidget_ChatDisconnect);
		irc->setConnectCallback(&ChatWidget_ChatConnect);
		irc->setUpdateUserListCallback(&ChatWidget_ChatUpdateUsers);
	}
}

//////////////////////////
// Clear the callbacks
void ChatWidget_ChatDeregisterCallbacks()
{
	IRCClient *irc = GetGlobalIRC();
	if (irc)  {
		irc->clearNewMessageCallback();
		irc->clearDisconnectCallback();
		irc->clearConnectCallback();
		irc->clearUpdateUserListCallback();
	}
}

}; // namespace DeprecatedGUI
