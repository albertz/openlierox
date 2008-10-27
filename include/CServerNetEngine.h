/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


#ifndef __CSERVER_NET_ENGINE_H__
#define __CSERVER_NET_ENGINE_H__

#include "CWorm.h"
#include "CVec.h"

class GameServer;
class CServerConnection;

class CServerNetEngine
{
public:
	// Constructor
	CServerNetEngine( GameServer * _server, CServerConnection * _client ):
		server( _server ), cl( _client )
		{ }

	virtual ~CServerNetEngine() { }
	
	// TODO: move here all net code from CServer
	// Do not move here ParseConnectionlessPacket(), or make it static, 'cause client is not available for connectionless packet
	
	// Parsing
	virtual void		ParsePacket(CBytestream *bs);

	void		ParseImReady(CBytestream *bs);
	void		ParseUpdate(CBytestream *bs);
	void		ParseDeathPacket(CBytestream *bs);
	void		ParseChatText(CBytestream *bs);
	virtual void ParseChatCommandCompletionRequest(CBytestream *bs) { return; };
	virtual void ParseAFK(CBytestream *bs) { return; };
	void		ParseUpdateLobby(CBytestream *bs);
	void		ParseDisconnect();
	void		ParseGrabBonus(CBytestream *bs);
	void		ParseSendFile(CBytestream *bs);

	bool		ParseChatCommand(const std::string& message);

	// Sending
	
	void		SendPacket(CBytestream *bs);

	virtual void SendClientReady(CServerConnection* receiver);
	virtual void SendText(const std::string& text, int type);
	virtual void SendChatCommandCompletionSolution(const std::string& startStr, const std::string& solution) { return; };
	virtual void SendChatCommandCompletionList(const std::string& startStr, const std::list<std::string>& solutions) { return; };
	virtual void SendWormsOut(const std::list<byte>& ids);
	virtual void SendWeapons();
	virtual int SendFiles() { return 0; }; // Returns client ping, or 0 if no packet was sent
	virtual void SendSpawnWorm(CWorm *Worm, CVec pos);

protected:
	// Attributes
	GameServer 	*server;
	CServerConnection *cl;
};

class CServerNetEngineBeta3: public CServerNetEngine 
{

public:
	CServerNetEngineBeta3( GameServer * _server, CServerConnection * _client ):
		CServerNetEngine( _server, _client )
		{ }

	void SendText(const std::string& text, int type);
};

class CServerNetEngineBeta5: public CServerNetEngineBeta3
{

public:
	CServerNetEngineBeta5( GameServer * _server, CServerConnection * _client ):
		CServerNetEngineBeta3( _server, _client )
		{ }
		
	int SendFiles();
};

class CServerNetEngineBeta7: public CServerNetEngineBeta5
{

public:
	CServerNetEngineBeta7( GameServer * _server, CServerConnection * _client ):
		CServerNetEngineBeta5( _server, _client )
		{ }

	void ParseChatCommandCompletionRequest(CBytestream *bs);
	void ParseAFK(CBytestream *bs);

	void SendChatCommandCompletionSolution(const std::string& startStr, const std::string& solution);
	void SendChatCommandCompletionList(const std::string& startStr, const std::list<std::string>& solutions);
};

class CServerNetEngineBeta8: public CServerNetEngineBeta7
{

public:
	CServerNetEngineBeta8( GameServer * _server, CServerConnection * _client ):
		CServerNetEngineBeta7( _server, _client )
		{ }

	void SendText(const std::string& text, int type);
};

#endif  //  __CSERVER_NET_ENGINE_H__
