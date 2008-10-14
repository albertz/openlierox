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

protected:
	// Attributes
	GameServer 	*server;
	CServerConnection *cl;
};

class CServerNetEngineBeta7: public CServerNetEngine 
{

public:
	CServerNetEngineBeta7( GameServer * _server, CServerConnection * _client ):
		CServerNetEngine( _server, _client )
		{ }

	void ParseChatCommandCompletionRequest(CBytestream *bs);
	void ParseAFK(CBytestream *bs);
};

#endif  //  __CSERVER_NET_ENGINE_H__
