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

enum { IRC_PORT = 6667 };

static CGuiLayout	cChat;
static std::list<std::string> chatText;
static std::list<std::string> chatUsers;
static bool cChatGuiInitialized;

enum {
	nc_Back = 0,
	nc_ChatText,
	nc_UserList,
	nc_ChatInput,
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

	CBrowser *b = (CBrowser *)cChat.getWidget(nc_ChatText);
	b->InitializeChatBox();
	for( std::list<std::string> :: iterator it = chatText.begin(); it != chatText.end(); it++ )
	{
		b->AddChatBoxLine( *it, tLX->clChatText, TXT_CHAT );
	}
	
	b = (CBrowser *)cChat.getWidget(nc_UserList);
	b->InitializeChatBox();
	for( std::list<std::string> :: iterator it = chatUsers.begin(); it != chatUsers.end(); it++ )
	{
		b->AddChatBoxLine( *it, tLX->clChatText, TXT_CHAT );
	}

	cChatGuiInitialized = true;
	return true;
}

//////////////////
// Shutdown the news menu
void Menu_Net_ChatShutdown()
{
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

					if( Menu_Net_Chat_Send(text) )
						cChat.SendMessage(nc_ChatInput, TXS_SETTEXT, "", 0); // Clear the text box
				}
				break;
		}

	}


	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());
}

static std::string strChatServerAddr;
static std::string strChatServerChannel;
static NetworkSocket chatSocket;
static bool socketOpened = false;
static NetworkAddr ChatServerAddr;
static std::string netBuffer;
static bool socketConnected = false;
static bool socketIsReady = false;
static float connectionClosedTime = -1.0f;
static enum { AUTH_NONE, AUTH_NICK_SENT, AUTH_USER_SENT, AUTH_JOINED_CHANNEL } AuthorizedState = AUTH_NONE;
static std::string nick;
static int nickUniqueNumber = -1;
static bool updatingUserList;

struct IrcCommand_t
{
	std::string sender;
	std::string cmd;
	std::vector<std::string> params;
};

void Menu_Net_Chat_ParseIrcCommand( const IrcCommand_t & cmd );
void Menu_Net_Chat_ConnectToServer();
void Menu_Net_Chat_DisconnectFromServer();

void Menu_Net_Chat_InitNet()
{
	if( socketOpened )
		return;
	socketOpened = true;
	
	chatSocket = OpenReliableSocket(0);

	FILE *fp = OpenGameFile("cfg/chatserver.txt", "r");
	if (fp)  {
		strChatServerAddr = ReadUntil(fp, '/');
		strChatServerChannel = ReadUntil(fp, '\n');
		fclose(fp);
		//printf("Chat server %s channel %s\n", strChatServerAddr.c_str(), strChatServerChannel.c_str());
	} else {
		strChatServerAddr = "irc.quakenet.org";
		strChatServerChannel = "#LieroX";
	};

	ResetNetAddr(ChatServerAddr);
	if( !GetNetAddrFromNameAsync( strChatServerAddr, ChatServerAddr ) )
	{
		printf("Wrong chat server addr: %s\n", strChatServerAddr.c_str() );
	};
}

void Menu_Net_Chat_ConnectToServer()
{
	//printf("Menu_Net_Chat_ConnectToServer()\n");
	
	if( !IsNetAddrValid(ChatServerAddr) )
		return;

	std::string addrStr;
	NetAddrToString( ChatServerAddr, addrStr );
	//printf("Menu_Net_Chat_ConnectToServer(): server IP %s\n", addrStr.c_str());
	
	SetNetAddrPort( ChatServerAddr, IRC_PORT );
	
	if( socketConnected )
		Menu_Net_Chat_DisconnectFromServer();

	if( !ConnectSocket( chatSocket, ChatServerAddr ) )
		return;
	
	socketConnected = true;
	socketIsReady = false;
	netBuffer = "";
	AuthorizedState = AUTH_NONE;
	nick = tLXOptions->tGameinfo.sLastSelectedPlayer;
	replace( nick, " ", "_" ); // Escape some symbols
	replace( nick, "@", "-" );
	replace( nick, ":", "-" );
	replace( nick, "!", "-" );
	nickUniqueNumber = -1;
	updatingUserList = false;
	//printf("IRC server connected\n");
};

void Menu_Net_Chat_DisconnectFromServer()
{
	if( socketConnected )
	{
		if( socketIsReady )
			WriteSocket(chatSocket, "QUIT\r\n");
		CloseSocket(chatSocket);
		socketConnected = false;
		connectionClosedTime = tLX->fCurTime;
	};
	//printf("IRC server disconnected\n");
};

void Menu_Net_Chat_Process()
{
	//printf("Menu_Net_Chat_Process()\n");
	Menu_Net_Chat_InitNet();
	
	if( !socketConnected )
	{
		if( tLX->fCurTime - connectionClosedTime > 5.0f ) // Re-connect once per 5 seconds
		{
			connectionClosedTime = tLX->fCurTime;
			Menu_Net_Chat_ConnectToServer();
		}
		return;
	}

	if( ! socketIsReady )
	{
		if( !IsSocketReady(chatSocket) )
			return;
		socketIsReady = true;
	}

	if( AuthorizedState == AUTH_NONE ) // Initiate server response
	{
		WriteSocket(chatSocket, "NICK " + nick + "\r\n");
		AuthorizedState = AUTH_NICK_SENT;
		//printf("Menu_Net_Chat_Process(): sent %s\n", ("NICK " + nick).c_str());
	};
	
	char buf[512];
	int readed = ReadSocket( chatSocket, buf, sizeof(buf) ); // I hope it's non-blocking
	if( readed == 0 )
		return;
	if( readed < 0 ) // Error
	{
		Menu_Net_Chat_DisconnectFromServer();
		return;
	};
	netBuffer.append( buf, readed );
	while( netBuffer.find("\r\n") != std::string::npos )
	{
		std::string line = netBuffer.substr( 0, netBuffer.find("\r\n") );
		//printf("IRC: %s\n", line.c_str());
		netBuffer = netBuffer.substr( netBuffer.find("\r\n")+2 );
		IrcCommand_t cmd;
		if(line[0] == ':')
		{
			cmd.sender = line.substr( 1, line.find(" ") );
			line = line.substr( line.find(" ")+1 );
		};
		cmd.cmd = line.substr( 0, line.find(" ") );
		while( line.find(" ") != std::string::npos )
		{
			line = line.substr( line.find(" ")+1 );
			if( line.size() == 0 )
				break;
			if( line[0] == ':' )
			{
				cmd.params.push_back(line.substr(1));
				break;
			};
			cmd.params.push_back( line.substr( 0, line.find(" ") ));
		};
		Menu_Net_Chat_ParseIrcCommand(cmd);
	};
};

bool Menu_Net_Chat_Send(const std::string & text)
{
	if( !socketConnected || !socketIsReady || AuthorizedState != AUTH_JOINED_CHANNEL )
		return false;
	WriteSocket(chatSocket, "PRIVMSG " + strChatServerChannel + " :" + text + "\r\n");
	//printf("Menu_Net_Chat_Send(): sent %s\n", text.c_str());
	// Route the same message back to our parser func, there's no echo in IRC
	IrcCommand_t cmd;
	cmd.sender = nick;
	cmd.cmd = "PRIVMSG";
	cmd.params.push_back(strChatServerChannel);
	cmd.params.push_back(text);
	Menu_Net_Chat_ParseIrcCommand(cmd);
	return true;
};

void Menu_Net_Chat_ParseIrcCommand( const IrcCommand_t & cmd )
{
	/*
	printf("IRC: sender '%s' cmd '%s'", cmd.sender.c_str(), cmd.cmd.c_str() );
	for( int i=0; i<cmd.params.size(); i++ )
		printf(" param %i '%s'", i, cmd.params[i].c_str());
	printf("\n");
	*/
	if( cmd.cmd == "PING" )
	{
		WriteSocket(chatSocket, "PONG :" + cmd.params[0] + "\r\n");
		
		if( AuthorizedState == AUTH_NICK_SENT )
		{
			if( nickUniqueNumber >= 0 )
				nick += itoa(nickUniqueNumber);
			WriteSocket(chatSocket, "USER " + nick + " ? ? :" + GetGameVersion().asString() + "\r\n");
			AuthorizedState = AUTH_USER_SENT;
			//printf("Menu_Net_Chat_ParseIrcCommand(): sent USER\n");
		}
	}
	else
	if( atoi(cmd.cmd) == LIBIRC_RFC_ERR_NICKNAMEINUSE )
	{
		nickUniqueNumber ++;
		WriteSocket(chatSocket, "NICK " + nick + itoa(nickUniqueNumber) + "\r\n");
		//printf("Menu_Net_Chat_ParseIrcCommand(): sent %s\n", ("NICK " + nick + itoa(nickUniqueNumber)).c_str());
	}
	else
	if( cmd.cmd == "MODE" && AuthorizedState == AUTH_USER_SENT )
	{
		WriteSocket(chatSocket, "JOIN " + strChatServerChannel + "\r\n");
		AuthorizedState = AUTH_JOINED_CHANNEL;
		//printf("Menu_Net_Chat_ParseIrcCommand(): sent JOIN\n");
	}
	else
	if( cmd.cmd == "PRIVMSG" )
	{
		if( cmd.params.size() < 2 )
			return;
		std::string text = cmd.sender.substr(0, cmd.sender.find("!")) + ": " + cmd.params[1];
		chatText.push_back( text );

		while( chatText.size() > 1024 )
			chatText.pop_front();
			
		if( cChatGuiInitialized )
		{
			CBrowser *b = (CBrowser *)cChat.getWidget(nc_ChatText);
			b->AddChatBoxLine( text, tLX->clChatText, TXT_CHAT );
		}
	}
	else
	if( atoi(cmd.cmd) == LIBIRC_RFC_RPL_NAMREPLY )
	{
		if( ! updatingUserList )
			chatUsers.clear();
		updatingUserList = true;
		if( cmd.params.size() < 4 )
			return;
		
		std::string line = cmd.params[3];
		while( line.find(" ") != std::string::npos )
		{
			chatUsers.push_back( line.substr( 0, line.find(" ") ));
			if( chatUsers.back()[0] == '@' ) // Channel Operator
				chatUsers.back() = chatUsers.back().substr(1);
			line = line.substr( line.find(" ")+1 );
		};
		chatUsers.push_back(line);
		if( chatUsers.back()[0] == '@' ) // Channel Operator
			chatUsers.back() = chatUsers.back().substr(1);
	}
	else
	if( atoi(cmd.cmd) == LIBIRC_RFC_RPL_ENDOFNAMES )
	{
		updatingUserList = false;
		chatUsers.sort();
		if( cChatGuiInitialized )
		{
			CBrowser *b = (CBrowser *)cChat.getWidget(nc_UserList);
			b->InitializeChatBox();
			for( std::list<std::string> :: iterator it = chatUsers.begin(); it != chatUsers.end(); it++ )
			{
				b->AddChatBoxLine( *it, tLX->clChatText, TXT_CHAT );
			}
		}
	}
	else
	if( nick == cmd.sender.substr(0, cmd.sender.find("!")) && // You have been kicked by RazzyBot
		( cmd.cmd == "PART" || cmd.cmd == "QUIT" || cmd.cmd == "KICK" ) )
	{
		Menu_Net_Chat_DisconnectFromServer();
	}
	else
	if( cmd.cmd == "JOIN" || cmd.cmd == "PART" || cmd.cmd == "NICK" || cmd.cmd == "QUIT" || cmd.cmd == "KICK" )
	{
		WriteSocket(chatSocket, "NAMES " + strChatServerChannel + "\r\n"); // Re-request user names
	}
};

}; // namespace DeprecatedGUI
