/*
 *  FlagInfo.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 20.03.09.
 *  code under LGPL
 *
 */

#ifndef __OLX__FLAGINFO_H__
#define __OLX__FLAGINFO_H__

#include "CVec.h"
#include "olx-types.h"

class CBytestream;
class CWorm;
class CServerConnection;
struct SDL_Surface;
class CViewport;
class CGameSkin;
class CMap;

struct FlagSpawnPoint {
	CVec pos;
};

struct Flag {
	Flag(int i = 0); ~Flag();
	Flag(const Flag& f) : id(0), skin(NULL) { operator=(f); }
	Flag& operator=(const Flag& f);
	
	int id;
	CVec pos; // only relevant if no holder worm
	int holderWorm;
	FlagSpawnPoint spawnPoint;
	bool atSpawnPoint;
	CGameSkin* skin;
	
	CVec getPos();
	void setCustomPos(const CVec& p) { pos = p; atSpawnPoint = false; holderWorm = -1; }
	void setHolderWorm(int w) { atSpawnPoint = false; holderWorm = w; }
	void setBack() { atSpawnPoint = true; holderWorm = -1; }
	
	bool operator<(const Flag& f) const { return id < f.id; }
};

struct FlagInfoData;

class FlagInfo {
private:
	FlagInfoData* data;
public:
	FlagInfo(); ~FlagInfo();
	void reset();
	void readUpdate(CBytestream* bs);
	static void skipUpdate(CBytestream* bs);
	
	Flag* initFlag(int id);
	Flag* getFlag(int id);
	Flag* getFlagOfWorm(int worm);
	bool removeFlag(int id);

	int getWidth() const;
	int getHeight() const;

	Flag* applyInitFlag(int id, const CVec& spawnPos);
	void applyRemoveFlag(int id);
	void applyCustomPos(Flag* flag, const CVec& pos);
	void applySpawnPos(Flag* flag, const CVec& spawnPos);
	void applyHolderWorm(Flag* flag, int w);
	void applySetBack(Flag* flag);
	void sendCurrentState(CServerConnection* cl);
	
	void checkWorm(CWorm* worm);

	void draw(SDL_Surface* bmpDest, CViewport* v);
	void drawWormAttachedFlag(CWorm* worm, SDL_Surface* bmpDest, CViewport* v);
	void drawOnMiniMap(CMap* cMap, SDL_Surface* bmpDest, uint miniX, uint miniY);	
};

#endif
