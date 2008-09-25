/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////



#ifndef __CCLIENT_NET_ENGINE_H__
#define __CCLIENT_NET_ENGINE_H__

#include "CBytestream.h"
#include "Version.h"
#include "LieroX.h"


class CClient;

class CClientNetEngine {

protected:

	CClient * client;

public:
	// Constructor
	CClientNetEngine(CClient * _client): client(_client) { }

	virtual ~CClientNetEngine() { }

	// Parsing
	virtual void		ParseConnectionlessPacket(CBytestream *bs);
	virtual void		ParsePacket(CBytestream *bs);

	// Sending
	virtual void		SendGameReady();
	virtual void		SendDeath(int victim, int killer);
	virtual void		SendText(const std::string& sText, std::string sWormName);
	virtual void		SendWormDetails(void);
	virtual void		SendGrabBonus(int id, int wormid);
	virtual void		SendUpdateLobby(bool ready = true);
	virtual void		SendDisconnect();
	virtual void		SendFileData();
	virtual void		SendChatCommandCompletionRequest(const std::string& startStr);
	virtual void		SendAFK(int wormid, AFK_TYPE afkType);
#ifdef FUZZY_ERROR_TESTING
	virtual void		SendRandomPacket();
#endif

private:
	void		SendTextInternal(const std::string& sText, const std::string& sWormName);

protected:

	// Internal details that may be used by child class
	// I expect child class will redirect most in-lobby messages to parent class
	void		ParseChallenge(CBytestream *bs);
	void		ParseConnected(CBytestream *bs);
	void		ParsePong(void);
	void		ParseTraverse(CBytestream *bs);
	void		ParseConnectHere(CBytestream *bs);

	bool		ParsePrepareGame(CBytestream *bs);
	void		ParseStartGame(CBytestream *bs);
	void		ParseSpawnWorm(CBytestream *bs);
	void		ParseWormInfo(CBytestream *bs);
	void		ParseWormWeaponInfo(CBytestream *bs);
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
	void		ParseChatCommandCompletionSolution(CBytestream* bs);
	
};

class CClientNetEngineBeta6: public CClientNetEngine {
public:
	CClientNetEngineBeta6( CClient * _client ): CClientNetEngine( _client ) { }
	// Empty for now, I'll add my super-leet net engine later, hehe
};


#endif  //  __CCLIENT_NET_ENGINE_H__
