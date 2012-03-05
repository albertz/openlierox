//
//  ClientConnectionRequestInfo.cpp
//  OpenLieroX
//
//  Created by Albert Zeyer on 08.01.12.
//  code under LGPL
//

#include "ClientConnectionRequestInfo.h"
#include "ProfileSystem.h"
#include "game/Game.h"
#include "game/CWorm.h"
#include "Debug.h"
#include "CBytestream.h"


bool WormJoinInfo::skipInfo(CBytestream *bs) {
	bs->SkipString();
	bs->Skip(2);
	bs->SkipString();
	return bs->Skip(3);
}

void WormJoinInfo::loadFromProfile(const SmartPointer<profile_t>& p) {
	sName = RemoveSpecialChars(p->sName);
	m_type = WormType::fromInt(p->iType);
	if(m_type == NULL) {
		warnings << "WormJoinInfo::loadFromProfile: profile has invalid WormType " << p->iType << endl;
		m_type = PRF_HUMAN; // fallback
	}
	iTeam = CLAMP(p->iTeam, 0, 3);
	skinFilename = p->cSkin.getFileName();
	skinColor = Color(p->R, p->G, p->B);
}

///////////////////
// Read info from a bytestream
void WormJoinInfo::readInfo(CBytestream *bs)
{
	sName = bs->readString();
	
	m_type = bs->readInt(1) ? PRF_COMPUTER : PRF_HUMAN;
	iTeam = CLAMP(bs->readInt(1), 0, 3);
	skinFilename = bs->readString();
	
	Uint8 r = bs->readByte();
	Uint8 g = bs->readByte();
	Uint8 b = bs->readByte();
	skinColor = Color(r, g, b);
}


void WormJoinInfo::applyTo(CWorm* worm) const {
	worm->sName = sName;
	worm->m_type = m_type;
	worm->iTeam = iTeam;	
	worm->cSkin.write().Change(skinFilename);
	worm->cSkin.write().setDefaultColor(skinColor);
	worm->cSkin.write().Colorize(skinColor);
}


WormJoinInfo wormJoinInfoFromProfile(const SmartPointer<profile_t>& prof) {
	WormJoinInfo info;
	info.loadFromProfile(prof);
	return info;
}

WormJoinInfo wormJoinInfoFromWorm(CWorm* w) {
	WormJoinInfo info;
	info.sName = w->getName();
	info.iTeam = w->getTeam();
	info.m_type = w->getType();
	info.skinFilename = w->getSkin().getFileName();
	info.skinColor = w->getSkin().getDefaultColor();
	return info;
}

void ClientConnectionRequestInfo::initFromGame() {
	worms.clear();
	for_each_iterator(CWorm*, w, game.localWorms()) {
		SmartPointer<profile_t> p = w->get()->getProfile();
		if(!p.get())
			p = profileFromWorm(w->get());
		worms.push_back(p);
	}
}
