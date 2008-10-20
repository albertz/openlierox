/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// IRC client class
// Sergyi Pylypenko

#ifndef __IRC_H__
#define __IRC_H__

#include <vector>
#include <list>
#include "Networking.h"
#include "Timer.h"

// Basic defines
#define IRC_PORT 6667
#define IRC_NICK_MAX_LEN 15

//
// IRC client class
//

class IRCClient  {
public:
	// Constructors and destructors

	IRCClient() :
	m_socketOpened(false),
	m_socketConnected(false),
	m_socketIsReady(false),
	m_connecting(false),
	m_connectionClosedTime(0),
	m_nickUniqueNumber(-1),
	m_authorizedState(AUTH_NONE),
	m_updatingUserList(false),
	m_newMsgCallback(NULL),
	m_disconnectCallback(NULL),
	m_updateUsersCallback(NULL)
	{}

	~IRCClient()  { disconnect(); }

private:
	// Types

	struct IRCCommand  {
		std::string sender;
		std::string cmd;
		std::vector<std::string> params;
	};

	enum IRCState { 
		AUTH_NONE,
		AUTH_NICK_SENT,
		AUTH_USER_SENT,
		AUTH_JOINED_CHANNEL
	};

public:
	enum IRCTextType  {
		IRC_TEXT_CHAT,
		IRC_TEXT_NOTICE,
		IRC_TEXT_ACTION
	};

	// Chat line class
	class IRCChatLine  {  public:
		// Constructors
		IRCChatLine(const std::string& txt, IRCTextType _type) :
		text(txt), type(_type) {}

		IRCChatLine(const IRCChatLine& oth)  { operator=(oth); }

		// Operators
		IRCChatLine& operator=(const IRCChatLine& oth)  {
			if (this != &oth)  {
				text = oth.text;
				type = oth.type;
			}
			return *this;
		}

		std::string text;
		IRCTextType	type;
	};

	// TODO: use the Event<> class
	typedef void(*IRCNewMessageCB)(const std::string&, int);
	typedef void(*IRCUpdateUserListCB)(const std::list<std::string>&);
	typedef void(*IRCDisconnectCB)();

private:
	// Attributes

	std::list<IRCChatLine> m_chatText;
	std::list<std::string> m_chatUsers;

	NetworkAddr m_chatServerAddr;
	std::string	m_chatServerAddrStr;
	std::string m_chatServerChannel;
	NetworkSocket m_chatSocket;
	std::string	m_netBuffer;
	bool		m_socketOpened;
	bool		m_socketConnected;
	bool		m_socketIsReady;
	bool		m_connecting;
	float		m_connectionClosedTime;
	std::string	m_myNick;
	IRCState	m_authorizedState;
	int			m_nickUniqueNumber;
	bool		m_updatingUserList;
	IRCNewMessageCB	m_newMsgCallback;
	IRCDisconnectCB	m_disconnectCallback;
	IRCUpdateUserListCB	m_updateUsersCallback;

private:
	// Methods

	bool	initNet();
	void	parseCommand(const IRCCommand& cmd);
	void	makeNickIRCFriendly();
	void	addChatMessage(const std::string& msg, IRCTextType type);
	bool	processConnecting();
	void	readData();

	// Sending
	void	sendJoin();
	void	sendNick();
	void	sendPong(const std::string& param);
	void	sendRequestNames();
	void	sendUserAuth();

	// Parsing
	void	parsePing(const IRCCommand& cmd);
	void	parseNickInUse(const IRCCommand& cmd);
	void	parseMode(const IRCCommand& cmd);
	void	parsePrivmsg(const IRCCommand& cmd);
	void	parseNameReply(const IRCCommand& cmd);
	void	parseEndOfNames(const IRCCommand& cmd);
	void	parseDropped(const IRCCommand& cmd);
	void	parseKicked(const IRCCommand& cmd);
	void	parseJoin(const IRCCommand& cmd);
	void	parseNick(const IRCCommand& cmd);
	void	parseNotice(const IRCCommand& cmd);


public:
	bool	connect(const std::string& server, const std::string& channel, const std::string& nick);
	void	disconnect();
	bool	sendChat(const std::string& text);
	void	process();

	const std::list<std::string>& getUserList()			{ return m_chatUsers; }
	const std::list<IRCChatLine>& getMessageList()		{ return m_chatText; }
	const std::string getNick()							{ return m_myNick; }

	void	setNewMessageCallback(IRCNewMessageCB cb)	{ m_newMsgCallback = cb; }
	void	clearNewMessageCallback()					{ m_newMsgCallback = NULL; }
	IRCNewMessageCB getNewMessageCallback()				{ return m_newMsgCallback; }

	void	setDisconnectCallback(IRCDisconnectCB cb)	{ m_disconnectCallback = cb; }
	void	clearDisconnectCallback()					{ m_disconnectCallback = NULL; }
	IRCDisconnectCB getDisconnectCallback()				{ return m_disconnectCallback; }

	void	setUpdateUserListCallback(IRCUpdateUserListCB cb)	{ m_updateUsersCallback = cb; }
	void	clearUpdateUserListCallback()						{ m_updateUsersCallback = NULL; }
	IRCUpdateUserListCB getUpdateUserListCallback()					{ return m_updateUsersCallback; }
};

//
// Global IRC handling
//

bool	InitializeIRC();
void	ShutdownIRC();
IRCClient *GetGlobalIRC();
void	ProcessIRC();

#endif // __IRC_H__