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

#include <iostream>
#include "IRC.h"
#include "LieroX.h"
#include "Version.h"
#include "StringBuf.h"
#include "StringUtils.h"
#include "Event.h"
#include "libirc_rfcnumeric.h"


/////////////////////////
// Initialize the IRC networking things (private)
bool IRCClient::initNet()
{
	// Check if already inited
	if(m_socketOpened)
		return true;
	
	// Open the socket
	m_chatSocket = OpenReliableSocket(0);
	if (!IsSocketStateValid(m_chatSocket))  {
		printf("ERROR: Could not open a socket for IRC networking: " + GetSocketErrorStr(GetSocketErrorNr()) + "\n");
		return false;
	}

	m_socketOpened = true;

	// Get the address
	ResetNetAddr(m_chatServerAddr);
	if(!GetNetAddrFromNameAsync(m_chatServerAddrStr, m_chatServerAddr))	 {
		printf("ERROR: Wrong IRC server addr: %s" + m_chatServerAddrStr + "\n");
		return false;
	}

	return true;
}


///////////////////////////
// Add a chat message and call the "on update" event
void IRCClient::addChatMessage(const std::string &msg)
{
	// Add the text
	m_chatText.push_back(msg);

	// Get rid of old messages
	while (m_chatText.size() > 1024)
		m_chatText.pop_front();

	// Run the event function
	if (m_newMsgCallback)
		m_newMsgCallback(msg);
}

////////////////////
// Process the connecting timer
void IRCClient::onProcessingTimer(Timer::EventData ev)
{
	ev.shouldContinue = processConnecting();
}

///////////////////////
// Connecting process, returns true if finished
bool IRCClient::processConnecting()
{
	// Check for DNS resolution
	if (!IsNetAddrValid(m_chatServerAddr))
		return false;

	// Convert the address to string
	std::string addrStr;
	NetAddrToString(m_chatServerAddr, addrStr);
	
	// Add IRC port
	SetNetAddrPort(m_chatServerAddr, IRC_PORT );

	// Connect
	if (!ConnectSocket(m_chatSocket, m_chatServerAddr))  {
		printf("IRC error: could not connect to the server " + addrStr);
		return true;
	}
	
	// Connected
	m_socketConnected = true;
	m_socketIsReady = false;
	m_netBuffer.clear();
	m_authorizedState = AUTH_NONE;

	// Adjust the nickname
	if (m_myNick.empty())
		m_myNick = "OpenLieroXor";
	makeNickIRCFriendly();

	// Join the channel
	sendJoin();

	return true;
}


//////////////////////////
// Escapes special characters in the nick
void IRCClient::makeNickIRCFriendly()
{
	// Escape some symbols, make nick IRC-compatible
	replace(m_myNick, " ", "_" ); 
	#define S_LETTER_UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	#define S_LETTER_LOWER "abcdefghijklmnopqrstuvwxyz"
	#define S_LETTER S_LETTER_UPPER S_LETTER_LOWER
	#define S_NUMBER "0123456789"
	#define S_SYMBOL "-[]\\`^{}_"

	while (m_myNick.find_first_not_of(S_LETTER S_NUMBER S_SYMBOL) != std::string::npos)
		m_myNick[m_myNick.find_first_not_of(S_LETTER S_NUMBER S_SYMBOL)] = '-';

	m_nickUniqueNumber = -1;
	m_updatingUserList = false;
}


/////////////////////////////
// Connect to server#channel
bool IRCClient::connect(const std::string &server, const std::string &channel, const std::string &nick)
{
	// Disconnect first
	if (m_socketConnected)
		disconnect();

	// Initialize the network
	if (!initNet())
		return false;

	// If the DNS has not been resolved yet, setup the processing timer
	if (!IsNetAddrValid(m_chatServerAddr))  {
		// Set the processing timer
		if (m_processConnectingTimer)
			delete m_processConnectingTimer;
		m_processConnectingTimer = new Timer;
		m_processConnectingTimer->interval = 50;
		m_processConnectingTimer->once = false;
		m_processConnectingTimer->onTimer.handler() = getEventHandler(this, &IRCClient::onProcessingTimer);
		m_processConnectingTimer->start();

		return true;

	// Resolved DNS
	} else
		return processConnecting();
}

///////////////////////
// Disconnect from the server
void IRCClient::disconnect()
{
	// Disconnect only when connected
	if (!m_socketConnected)
		return;

	// Call the disconnect callback
	if (m_disconnectCallback)
		m_disconnectCallback();

	// Close socket
	if (m_socketIsReady)
		WriteSocket(m_chatSocket, "QUIT\r\n");
	CloseSocket(m_chatSocket);

	// Clear the variables
	m_socketConnected = false;
	m_socketIsReady = false;
	m_socketOpened = false;
	m_connectionClosedTime = tLX->fCurTime;
	InvalidateSocketState(m_chatSocket);
	SetNetAddrValid(m_chatServerAddr, false);
	m_chatServerAddrStr.clear();
	m_chatServerChannel.clear();
	m_netBuffer.clear();
	m_myNick.clear();
	m_authorizedState = AUTH_NONE;
	m_nickUniqueNumber = -1;
	m_updatingUserList = false;
	if (m_keepAliveTimer)  {
		delete m_keepAliveTimer;
		m_keepAliveTimer = NULL;
	}

	if (m_processConnectingTimer)  {
		delete m_processConnectingTimer;
		m_processConnectingTimer = NULL;
	}
	m_newMsgCallback = NULL;
	m_disconnectCallback = NULL;

	printf("IRC: disconnected");
}

/*
 *
 * Sending part
 *
 */

///////////////
// Join a channel
void IRCClient::sendJoin()
{
	WriteSocket(m_chatSocket, "JOIN " + m_chatServerChannel + "\r\n");
	m_authorizedState = AUTH_JOINED_CHANNEL;
}


///////////////////
// Tell the server our nickname
void IRCClient::sendNick()
{
	if (m_nickUniqueNumber < 0)
		WriteSocket(m_chatSocket, "NICK " + m_myNick + "\r\n");
	else
		WriteSocket(m_chatSocket, "NICK " + m_myNick + itoa(m_nickUniqueNumber) + "\r\n");
}

///////////////////////
// Send a ping reply
void IRCClient::sendPong(const std::string &param)
{
	WriteSocket(m_chatSocket, "PONG :" + param + "\r\n");
}

///////////////////////
// Request nick list from the server
void IRCClient::sendRequestNames()
{
	WriteSocket(m_chatSocket, "NAMES " + m_chatServerChannel + "\r\n");
}

///////////////////////
// Sends user authentication to the server
void IRCClient::sendUserAuth()
{
	std::string nick = m_myNick;
	if (m_nickUniqueNumber >= 0)
		nick += itoa(m_nickUniqueNumber);
	WriteSocket(m_chatSocket, "USER " + nick + " ? ? :" + GetGameVersion().asString() + "\r\n");
	m_authorizedState = AUTH_USER_SENT;
}


/*
 *
 * Parsing part
 *
 */

/////////////////////
// End of connection
void IRCClient::parseDropped(const IRCClient::IRCCommand &cmd)
{
	disconnect();
}

///////////////////
// End of name list
void IRCClient::parseEndOfNames(const IRCClient::IRCCommand &cmd)
{
	m_updatingUserList = false;
	m_chatUsers.sort();
}

////////////////////
// Join successful
void IRCClient::parseJoin(const IRCClient::IRCCommand& cmd)
{
	// Re-request the nick names
	sendRequestNames();
}

/////////////////////
// Parse a mode packet
void IRCClient::parseMode(const IRCClient::IRCCommand &cmd)
{
	if (m_authorizedState == AUTH_USER_SENT)  {
		sendJoin();
		m_authorizedState = AUTH_JOINED_CHANNEL;
	}
}

///////////////////////
// List of the nicknames in the channel
void IRCClient::parseNameReply(const IRCClient::IRCCommand &cmd)
{
	// Check
	if(cmd.params.size() < 4 )
		return;

	// If this is the first reply packet, clear the user list
	if(!m_updatingUserList )
		m_chatUsers.clear();

	m_updatingUserList = true;
	
	// Get the nick names
	StringBuf line(cmd.params[3]);
	std::vector<std::string> nicks = line.splitBy(' ');
	
	for (std::vector<std::string>::iterator it = nicks.begin(); it != nicks.end(); it++)  {
		// Check for validity
		std::string& user = *it;
		if (user.size() == 0)
			continue;

		if (user[0] == '@') // Channel Operator
			user.erase(0);

		m_chatUsers.push_back(user); // Add the user to the list
	}
}

////////////////////
// Parse the NICK command
void IRCClient::parseNick(const IRCClient::IRCCommand &cmd)
{
	sendRequestNames(); // Re-request the nicknames
}

//////////////////////
// The nickname is in use
void IRCClient::parseNickInUse(const IRCClient::IRCCommand &cmd)
{
	++m_nickUniqueNumber;
	sendNick();
}

///////////////////////
// Ping
void IRCClient::parsePing(const IRCClient::IRCCommand &cmd)
{
	// Check
	if (cmd.params.size() == 0)
		return;

	// Send reply
	sendPong(cmd.params[0]);
}

///////////////////////
// Private message
void IRCClient::parsePrivmsg(const IRCClient::IRCCommand &cmd)
{
	// Check
	if( cmd.params.size() < 2 )
		return;

	// The sender has to contain an exclamation mark
	size_t pos = cmd.sender.find('!');
	if (pos == std::string::npos)
		return;

	// Add the message
	std::string text = cmd.sender.substr(0, pos) + ": " + cmd.params[1];
	addChatMessage(text);
}

//////////////////////////////
// Parse an IRC command (private)
void IRCClient::parseCommand(const IRCClient::IRCCommand &cmd)
{
	/*
	printf("IRC: sender '%s' cmd '%s'", cmd.sender.c_str(), cmd.cmd.c_str() );
	for( int i=0; i<cmd.params.size(); i++ )
		printf(" param %i '%s'", i, cmd.params[i].c_str());
	printf("\n");
	*/

	// Process depending on the command

	if (cmd.cmd == "PING")
		parsePing(cmd);

	else if (atoi(cmd.cmd) == LIBIRC_RFC_ERR_NICKNAMEINUSE)
		parseNickInUse(cmd);

	else if (cmd.cmd == "MODE")
		parseMode(cmd);

	else if (cmd.cmd == "PRIVMSG")
		parsePrivmsg(cmd);

	else if (atoi(cmd.cmd) == LIBIRC_RFC_RPL_NAMREPLY)
		parseNameReply(cmd);

	else if (atoi(cmd.cmd) == LIBIRC_RFC_RPL_ENDOFNAMES)
		parseEndOfNames(cmd);

	else if (m_myNick == cmd.sender.substr(0, cmd.sender.find("!")) && // You have been kicked by RazzyBot
		(cmd.cmd == "PART" || cmd.cmd == "QUIT" || cmd.cmd == "KICK" ))
		parseDropped(cmd);

	else if (cmd.cmd == "JOIN")
		parseJoin(cmd);

	else if (cmd.cmd == "NICK")
		parseNick(cmd);
	else
		printf("IRC: unknown command " + cmd.cmd);
}