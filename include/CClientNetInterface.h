/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////



#ifndef __CCLIENT_NET_INTERFACE_H__
#define __CCLIENT_NET_INTERFACE_H__

#include "CBytestream.h"
#include "Version.h"


class CClient;

class CClientNetInterface {

private:

	CClient * client;

public:
	// Constructor
	CClientNetInterface(CClient * _client): client(_client) { }

	~CClientNetInterface() { }

	// Sending
	virtual void		SendGameReady();
	virtual void		SendDeath(int victim, int killer);
	virtual void		SendText(const std::string& sText, std::string sWormName);
	virtual void		Disconnect(void);
	virtual void		SendWormDetails(void);
#ifdef FUZZY_ERROR_TESTING
	virtual void		SendRandomPacket();
#endif

	// Parsing
	virtual void		ParseConnectionlessPacket(CBytestream *bs);
	virtual void		ParsePacket(CBytestream *bs);

protected:
	// Internal details
	void		ParseChallenge(CBytestream *bs);
	void		ParseConnected(CBytestream *bs);
	void		ParsePong(void);
	void		ParseTraverse(CBytestream *bs);
	void		ParseConnectHere(CBytestream *bs);

	bool		ParsePrepareGame(CBytestream *bs);
	void		ParseStartGame(CBytestream *bs);
	void		ParseSpawnWorm(CBytestream *bs);
	void		ParseWormInfo(CBytestream *bs);
	void		ParseText(CBytestream *bs);
	void		ParseScoreUpdate(CBytestream *bs);
	void		ParseGameOver(CBytestream *bs);
	void		ParseSpawnBonus(CBytestream *bs);
	void		ParseTagUpdate(CBytestream *bs);
	void		ParseCLReady(CBytestream *bs);
	void		ParseUpdateLobby(CBytestream *bs);
	void		ParseClientLeft(CBytestream *bs);
	void		ParseUpdateWorms(CBytestream *bs);
	void		ParseUpdateLobbyGame(CBytestream *bs);
	void		ParseWormDown(CBytestream *bs);
	void		ParseServerLeaving(CBytestream *bs);
	void		ParseSingleShot(CBytestream *bs);
	void		ParseMultiShot(CBytestream *bs);
	void		ParseUpdateStats(CBytestream *bs);
	void		ParseDestroyBonus(CBytestream *bs);
	void		ParseGotoLobby(CBytestream *bs);
    void        ParseDropped(CBytestream *bs);
    void        ParseSendFile(CBytestream *bs);

};


#endif  //  __CCLIENT_NET_INTERFACE_H__
