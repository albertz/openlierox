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


#include <string>
#include "Consts.h"


class CClient;
class CBytestream;

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
	virtual void		SendChatCommandCompletionRequest(const std::string& startStr) { return; };
	virtual void		SendAFK(int wormid, AFK_TYPE afkType, const std::string & message = "") { return; };
	virtual void		SendReportDamage(int victim, int damage, int offender) { return; };
#ifdef FUZZY_ERROR_TESTING
	virtual void		SendRandomPacket();
#endif

private:
	void		SendTextInternal(const std::string& sText, const std::string& sWormName);

protected:

	// Internal details that may be used by child class
	// I expect child class will redirect most in-lobby messages to parent class
	void		 ParseChallenge(CBytestream *bs);
	void		 ParseConnected(CBytestream *bs);
	void		 ParsePong();
	void		 ParseTimeIs(CBytestream *bs);
	void		 ParseTraverse(CBytestream *bs);
	void		 ParseConnectHere(CBytestream *bs);

	virtual bool ParsePrepareGame(CBytestream *bs);
	void		 ParseStartGame(CBytestream *bs);
	void		 ParseSpawnWorm(CBytestream *bs);
	void		 ParseWormInfo(CBytestream *bs);
	void		 ParseWormWeaponInfo(CBytestream *bs);
	void		 ParseText(CBytestream *bs);
	void		 ParseScoreUpdate(CBytestream *bs);
	void		 ParseGameOver(CBytestream *bs);
	void		 ParseSpawnBonus(CBytestream *bs);
	void		 ParseTagUpdate(CBytestream *bs);
	void		 ParseCLReady(CBytestream *bs);
	void		 ParseUpdateLobby(CBytestream *bs);
	void		 ParseWormsOut(CBytestream *bs);
	void		 ParseUpdateWorms(CBytestream *bs);
	virtual void ParseUpdateLobbyGame(CBytestream *bs);
	void		 ParseWormDown(CBytestream *bs);
	void		 ParseServerLeaving(CBytestream *bs);
	void		 ParseSingleShot(CBytestream *bs);
	void		 ParseMultiShot(CBytestream *bs);
	void		 ParseUpdateStats(CBytestream *bs);
	void		 ParseDestroyBonus(CBytestream *bs);
	void		 ParseGotoLobby(CBytestream *bs);
    void		 ParseDropped(CBytestream *bs);
    void		 ParseSendFile(CBytestream *bs);
	virtual void ParseChatCommandCompletionSolution(CBytestream* bs) { return; };
	virtual void ParseChatCommandCompletionList(CBytestream* bs) { return; };
	virtual void ParseAFK(CBytestream* bs) { return; };
    virtual void ParseReportDamage(CBytestream *bs) { return; };
	
};

// TODO: maybe add CClientNetEngineBeta5 for map/mod downloading - SendFileData() and ParseSendFile() funcs
class CClientNetEngineBeta7: public CClientNetEngine {
public:
	CClientNetEngineBeta7( CClient * _client ): CClientNetEngine( _client ) { }

	virtual void SendChatCommandCompletionRequest(const std::string& startStr);
	virtual void SendAFK(int wormid, AFK_TYPE afkType, const std::string & message = "");

	virtual bool ParsePrepareGame(CBytestream *bs);
	virtual void ParseUpdateLobbyGame(CBytestream *bs);
	virtual void ParseChatCommandCompletionSolution(CBytestream* bs);
	virtual void ParseChatCommandCompletionList(CBytestream* bs);
	virtual void ParseAFK(CBytestream* bs);
};

class CClientNetEngineBeta9: public CClientNetEngineBeta7 {
public:
	CClientNetEngineBeta9( CClient * _client ): CClientNetEngineBeta7( _client ) { }

	virtual bool ParsePrepareGame(CBytestream *bs);
	virtual void SendReportDamage(int victim, int damage, int offender);
    virtual void ParseReportDamage(CBytestream *bs);
};


#endif  //  __CCLIENT_NET_ENGINE_H__
