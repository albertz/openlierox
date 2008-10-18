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

#define IRC_PORT 6667
#define IRC_NICK_MAX_LEN 15

class IRCClient  {
public:
	IRCClient() :
	m_socketOpened(false),
	m_socketConnected(false),
	m_socketIsReady(false),
	m_connectionClosedTime(0),
	m_nickUniqueNumber(-1),
	m_authorizedState(AUTH_NONE),
	m_updatingUserList(false),
	m_keepAliveTimer(NULL),
	m_processConnectingTimer(NULL),
	m_newMsgCallback(NULL),
	m_disconnectCallback(NULL)
	{}

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
	// TODO: use the Event<> class
	typedef void(*IRCNewMessageCB)(const std::string&);
	typedef void(*IRCDisconnectCB)();

private:
	// Attributes

	std::list<std::string> m_chatText;
	std::list<std::string> m_chatUsers;

	NetworkAddr m_chatServerAddr;
	std::string	m_chatServerAddrStr;
	std::string m_chatServerChannel;
	NetworkSocket m_chatSocket;
	std::string	m_netBuffer;
	bool		m_socketOpened;
	bool		m_socketConnected;
	bool		m_socketIsReady;
	float		m_connectionClosedTime;
	std::string	m_myNick;
	IRCState	m_authorizedState;
	int			m_nickUniqueNumber;
	bool		m_updatingUserList;
	Timer		*m_keepAliveTimer;
	Timer		*m_processConnectingTimer;
	IRCNewMessageCB	m_newMsgCallback;
	IRCDisconnectCB	m_disconnectCallback;

private:
	// Methods

	bool	initNet();
	void	parseCommand(const IRCCommand& cmd);
	void	makeNickIRCFriendly();
	void	addChatMessage(const std::string& msg);
	void	onProcessingTimer(Timer::EventData ev);
	bool	processConnecting();

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
	void	parseJoin(const IRCCommand& cmd);
	void	parseNick(const IRCCommand& cmd);


public:
	bool	connect(const std::string& server, const std::string& channel, const std::string& nick);
	void	disconnect();
	bool	sendChat(const std::string& text);

	void	setNewMessageCallback(IRCNewMessageCB cb)	{ m_newMsgCallback = cb; }
	void	clearNewMessageCallback()					{ m_newMsgCallback = NULL; }
	IRCNewMessageCB getNewMessageCallback()				{ return m_newMsgCallback; }

	void	setDisconnectCallback(IRCDisconnectCB cb)	{ m_disconnectCallback = cb; }
	void	clearDisconnectCallback()					{ m_disconnectCallback = NULL; }
	IRCDisconnectCB getDisconnectCallback()				{ return m_disconnectCallback; }
};

#endif // __IRC_H__