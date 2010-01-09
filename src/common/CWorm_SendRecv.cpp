/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Worm class - Writing & Reading of packets
// Created 24/12/02
// Jason Boettcher



#include "LieroX.h"
#include "Debug.h"
#include "GfxPrimitives.h"
#include "CWorm.h"
#include "Protocol.h"
#include "CServer.h"
#include "MathLib.h"
#include "CClient.h"
#include "CServerConnection.h"
#include "WeaponDesc.h"
#include "ProfileSystem.h"



///////////////////
// Write my info to a bytestream
void CWorm::writeInfo(CBytestream *bs)
{
	bs->writeString(RemoveSpecialChars(sName));
	bs->writeInt(m_type->toInt(), 1);
	bs->writeInt(iTeam, 1);
	bs->writeString(cSkin.getFileName());

	for(short i = 0; i < 3; i++)
		bs->writeInt(cSkin.getDefaultColor()[i], 1);
}


void WormJoinInfo::loadFromProfile(profile_t* p) {
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
	worm->cSkin.Change(skinFilename);
	worm->cSkin.setDefaultColor(skinColor);
	worm->cSkin.Colorize(skinColor);
}

// Note: We don't put charge into the update packet because we only send the update packet to
//       _other_ worms, not to self


///////////////////
// Write a packet out (client-to-server + server-to-client)
void CWorm::writePacket(CBytestream *bs, bool fromServer, CServerConnection* receiver)
{
	short x, y;

	x = (short)vPos.x;
	y = (short)vPos.y;

	// Note: This method of saving 1 byte in position, limits the map size to just under 4096x4096

	// Position
	bs->write2Int12( x, y );

	// Angle
	bs->writeInt( (int)fAngle+90, 1);

	// Bit flags
	uchar bits = 0;
	if(tState.bCarve)
		bits |= 0x01;
	if(iFaceDirectionSide == DIR_RIGHT)
		bits |= 0x02;
	if(tState.bMove)
		bits |= 0x04;
	if(tState.bJump)
		bits |= 0x08;
	if(cNinjaRope.isReleased())
		bits |= 0x10;
	if(tState.bShoot)
		bits |= 0x20;

	bs->writeByte( bits );
	bs->writeByte( iCurrentWeapon );

	// Write out the ninja rope details
	if(cNinjaRope.isReleased())
		cNinjaRope.write(bs);


	// Velocity
	const Version& versionOfReceiver = fromServer ? receiver->getClientVersion() : cClient->getServerVersion();
	if(tState.bShoot || versionOfReceiver >= OLXBetaVersion(5)) {
		CVec v = vVelocity;
		bs->writeInt16( (Sint16)v.x );
		bs->writeInt16( (Sint16)v.y );
	}
	
	// client (>=beta8) sends also current server time
	if(!fromServer && versionOfReceiver >= OLXBetaVersion(8)) {
		bs->writeFloat( (float)cClient->serverTime().seconds() );
	}

	// Update the "last" variables
	updateCheckVariables();
}

//////////////
// Synchronizes the variables used for check below
void CWorm::updateCheckVariables()
{
	tLastState = tState; tLastState.iFaceDirectionSide = iFaceDirectionSide;
	fLastAngle = fAngle;
	fLastUpdateWritten = tLX->currentTime;
	iLastCurWeapon = iCurrentWeapon;
	cNinjaRope.updateCheckVariables();
	vLastUpdatedPos = vPos;
}

////////////////////
// Checks if we need to call writePacket, false when not
bool CWorm::checkPacketNeeded()
{
	// Dead worms don't need any updates
	if (iLives == WRM_OUT || !bAlive)
		return false;

	// State
	if (tState.bCarve)
		return true;
	if (tState.bShoot && !tWeapons[iCurrentWeapon].Reloading)
		return true;

	if (
		(tLastState.bCarve != tState.bCarve) ||
		(tLastState.iFaceDirectionSide != iFaceDirectionSide)  ||
		(tLastState.bMove != tState.bMove) ||
		(tLastState.bJump != tState.bJump) ||
		(tLastState.bShoot != tState.bShoot))
			return true;

	// Changed weapon
	if(iLastCurWeapon != iCurrentWeapon)
		return true;

	// Angle
	if (fabs(fLastAngle - fAngle) > 0.00001f)
		return true;

	// position change
	CVec vPosDif = vLastUpdatedPos - vPos;
	if (vPosDif.GetLength2())
		return true;

	if (vVelocity.GetLength2())
		return true;

	// Rope
	return cNinjaRope.writeNeeded();
}

// this is used to update the position on the client-side in CWorm::readPacketState
// it also updates frequently the velocity by estimation
void CWorm::net_updatePos(const CVec& newpos) {
	TimeDiff t = tLX->currentTime - fLastPosUpdate;

	// TODO: the following just draws the pos received in packet for debugging
	// atm we only have the debugimage available if _AI_DEBUG is set
	// this should be changed to DEBUG
#ifdef _AI_DEBUG
/*	SmartPointer<SDL_Surface> bmpDest = cClient->getMap()->GetDebugImage();
	if (bmpDest) {
		int node_x = (int)newpos.x*2, node_y = (int)newpos.y*2;
		int onode_x = (int)vPos.x*2, onode_y = (int)vPos.y*2;

		if(node_x-4 >= 0 && node_y-4 >= 0 && node_x+4 < bmpDest->w && node_y+4 < bmpDest->h
		&& onode_x-4 >= 0 && onode_y-4 >= 0 && onode_x+4 < bmpDest->w && onode_y+4 < bmpDest->h) {
			// a line between both
			DrawLine(bmpDest, node_x, node_y, onode_x, onode_y, tLX->clWhite);

			// Draw the old pos
			DrawRectFill(bmpDest,onode_x-3,onode_y-3,onode_x+3,onode_y+3, Color(122,122,255));

			// Draw the new pos
			DrawRectFill(bmpDest,node_x-4,node_y-4,node_x+4,node_y+4, (t == 0) ? Color(0,0,0) : Color(122,122,0));
		}
	} */
#endif

	vPos = newpos;
	bOnGround = CheckOnGround(); // update bOnGround; will perhaps be updated later in simulation

	if (!cGameScript)
		return;

	CVec dist = newpos - vOldPosOfLastPaket;

	// TODO: Why is there an option for disabling this? There is no reason to disable, it
	// should always be better with it activated. There is no magic behind, it's just
	// a more correct estimation/calculation.
	// HINT: Raziel told me about "jelly-jumping" remote worm behavior under lag in Beta3
	// and I believe it's because of an estimation - set net speed in options to "Modem" in client
	// and hang on ninjarope - the server will see remote worm sliding down fuzzily each frame,
	// linear approximation looks bit better for me. Raziel haven't confirmed it though.
	if( ! tLXOptions->bAntilagMovementPrediction )
	{
		// ignoring acceleration in this case for estimation
		if(t > 0.0f)
			vVelocity = dist / t.seconds();
	}
	else
	{
		if(t == TimeDiff(0)) { // this means we got 2 update packets in one frame
			// We want to ignore the last calculation, so use the pre-values to update the last values.
			// Restore them also to save them later again as the pre-values as we want to keep
			// there always some values with t>0 which we can use.
			vOldPosOfLastPaket = vPreOldPosOfLastPaket;
			vLastEstimatedVel = vPreLastEstimatedVel;
			fLastPosUpdate = fPreLastPosUpdate;
			t = tLX->currentTime - fLastPosUpdate;
			dist = newpos - vOldPosOfLastPaket;
		}

		if(t == TimeDiff(0)) { // if it is still =0 after the update, it means that we don't have previous data
			// we can't do anything here, just accept the situation

		} else {
			// Approximate with velocity and acceleration (including gravity)
			CVec a(0, 0);

			const gs_worm_t *wd = cGameScript->getWorm();
			// Air drag (Mainly to dampen the ninja rope)
			float Drag = wd->AirFriction;


			if(!bOnGround)	{
				// TODO: this is also not exact
				CVec preEstimatedVel = vVelocity; //dist / t;
				a.x -= SQR(preEstimatedVel.x) * SIGN(preEstimatedVel.x) * Drag;
				a.y -= SQR(preEstimatedVel.y) * SIGN(preEstimatedVel.y) * Drag;
			}

			if (cNinjaRope.isAttached())  {
				a += cNinjaRope.GetForce(newpos);
			}

			// Gravity
			a.y += wd->Gravity;

			// this estimates the vel of fLastPosUpdate
			// it do a better estimation as we had last time, so believe this more
			CVec estimatedVel = (dist / t.seconds()) - (a * t.seconds() / 2.0f);

			// in fLastPosUpdate, we have old vLastEstimatedVel and new estimatedVel
			// add the dif of them to our current vel
			vVelocity += estimatedVel - vLastEstimatedVel;

			// or just use this estimation
			//vVelocity = estimatedVel;

			// update the estimated vel for next time
			vLastEstimatedVel = vVelocity;
	/*
			// Ultimate in friction
			if(bOnGround) {
				// HINT: also this isn't exact (it would be like it's only one frame)
				estimatedVel.x *= 0.9f;
				// is it ok here?

				// Too slow, just stop
	//			if(fabs(estimatedVel.x) < 5 && !ws->iMove)
	//				estimatedVel.x = 0;
			}
	*/
			// we don't know anything of the moving in between, so we ignore this here
			// this is already calculated in simulation

			// HINT: Don't process the moving as it is already included in the linear part of the calculation
		}

	}

	vPreOldPosOfLastPaket = vOldPosOfLastPaket;
	vPreLastEstimatedVel = vLastEstimatedVel;
	fPreLastPosUpdate = fLastPosUpdate;

	vOldPosOfLastPaket = newpos;
	fLastPosUpdate = tLX->currentTime;

}

bool CWorm::hasOwnServerTime() {
	if(!getClient()) return false;
	return getClient()->getClientVersion() >= OLXBetaVersion(8);
}

///////////////////
// Read a packet (server side)
void CWorm::readPacket(CBytestream *bs, CWorm *worms)
{
	// Position and velocity
	short x, y;
	bs->read2Int12( x, y );
	vPos = CVec((float)x, (float)y);

	// Angle
	fAngle = (float)bs->readInt(1) - 90;

	// Flags
	uchar bits = bs->readByte();
	iCurrentWeapon = (uchar)CLAMP(bs->readByte(), (uchar)0, (uchar)4);

	iMoveDirectionSide = iFaceDirectionSide = DIR_LEFT;

	tState.bCarve = (bits & 0x01) != 0;
	if(bits & 0x02)
		iMoveDirectionSide = iFaceDirectionSide = DIR_RIGHT;
	tState.bMove = (bits & 0x04) != 0;
	tState.bJump = (bits & 0x08) != 0;
	tState.bShoot = (bits & 0x20) != 0;

	// Ninja rope
	bool rope = (bits & 0x10) != 0;
	if(rope)
		cNinjaRope.read(bs, worms, iID);
	else
		cNinjaRope.Release();

	// Velocity
	const Version& versionOfSender = getClient()->getClientVersion();
	if(tState.bShoot || versionOfSender >= OLXBetaVersion(5)) {
		Sint16 vx = bs->readInt16();
		Sint16 vy = bs->readInt16();
		vVelocity = CVec( (float)vx, (float)vy );
	}

	// client (>=beta8) sends also what it thinks what the server time is (was)
	if(versionOfSender >= OLXBetaVersion(8)) {
		fServertime = bs->readFloat();
		if(fServertime < cServer->getServerTime())
			fServertime = cServer->getServerTime();
	}
	
	// If the worm is inside dirt then it is probably carving
	if (tLX->iGameType == GME_HOST && cServer->getMap())
		if(cServer->getMap()->GetPixelFlag(x, y) & PX_DIRT)
			tState.bCarve = true;


	// Prevent a wall hack
	// HINT: commented out because it could lead to another cheating
	// When you call Spawn() on a worm, it will get a full health
	// It means that people with old LX could limit their FPS and whenever they would need to
	// recover their health they would simply find some thinner wall and try to fly through it
	/*if (tGameInfo.iGameType == GME_HOST && cServer->getMap() && cOwner)  {

		// Out of map
		if(x >= (short)cServer->getMap()->GetWidth() || y >= (short)cServer->getMap()->GetHeight())
		{
			vPos=vLastPos;
			cServer->SpawnWorm(this, vPos, cOwner);
		}

		// In rock
		if(cServer->getMap()->GetPixelFlag(x, y) & PX_ROCK)
		{
			vPos=vLastPos;
			cServer->SpawnWorm(this, vPos, cOwner);
		}

		vLastPos = vPos;
	}*/
}

////////////////
// Skip the packet (client-side)
bool CWorm::skipPacketState(CBytestream *bs)
{
	bs->Skip(4);  // Position + angle
	uchar bits = (uchar)bs->readByte(); // Flags
	bs->Skip(1);  // Current weapon
	if (bits & 0x10)  {  // Skip rope info (see CNinjaRope::read for more details)
		int type = bs->readByte(); // Rope type
		bs->Skip(3);
		if (type == ROP_SHOOTING || type == ROP_PLYHOOKED)
			bs->Skip(1);
	}

	bool shooting = (bits & 0x20) != 0; 

	const Version& versionOfSender = cClient->getServerVersion();
	bool gotVelocity = shooting || versionOfSender >= OLXBetaVersion(5);
		
	// Velocity
	if(gotVelocity) {
		bs->readInt16();
		bs->readInt16();
	}	
	
	return bs->isPosAtEnd();
}

////////////////
// Skip the packet (server-side)
bool CWorm::skipPacket(CBytestream *bs)
{
	bs->Skip(4);  // Position + angle
	uchar bits = (uchar)bs->readByte(); // Flags
	bs->Skip(1);  // Current weapon
	if (bits & 0x10)  {  // Skip rope info (see CNinjaRope::read for more details)
		int type = bs->readByte(); // Rope type
		bs->Skip(3);
		if (type == ROP_SHOOTING || type == ROP_PLYHOOKED)
			bs->Skip(1);
	}
	
	bool shooting = (bits & 0x20) != 0; 

	// Velocity
	const Version& versionOfSender = getClient()->getClientVersion();
	if(shooting || versionOfSender >= OLXBetaVersion(5)) {
		bs->readInt16();
		bs->readInt16();
	}
	
	// client (>=beta8) sends also what it thinks what the server time is (was)
	if(versionOfSender >= OLXBetaVersion(8)) {
		bs->readFloat();
	}
	
	return bs->isPosAtEnd();
}



///////////////////
// Read a packet (client side)
void CWorm::readPacketState(CBytestream *bs, CWorm *worms)
{
	if( NewNet::Active() )
	{
		warnings << "CWorm::readPacketState(): new net engine active" << endl;
		return;
	}
	if(cClient->OwnsWorm(this->getID())) {
		warnings << "get worminfo packet from server for our own worm" << endl;
		skipPacketState(bs);
		return;
	}

	if (!bUsed || !getAlive()) {
		// Note: This can happen also (even a lot of times) when a worm joins and we didn't received all information yet (because of limited reliable bandwidth).
		if(!bUsed)
			notes << "Client: readPacketState called on unused worm " << getID() << ":" << getName() << endl;
		skipPacketState(bs);
		return;
	}

	// Position
	short x, y;
	bs->read2Int12( x, y );

	// Angle
	tState.iAngle = (bs->readInt(1) - 90);
	fAngle = (float)tState.iAngle;

	// Flags
	uchar bits = bs->readByte();
	iCurrentWeapon = (uchar)CLAMP(bs->readByte(), (uchar)0, (uchar)4);

	iMoveDirectionSide = iFaceDirectionSide = DIR_LEFT;
	tState.iFaceDirectionSide = DIR_LEFT;

	tState.bCarve = (bits & 0x01);
	if(bits & 0x02) {
		iMoveDirectionSide = iFaceDirectionSide = DIR_RIGHT;
		tState.iFaceDirectionSide = DIR_RIGHT;
	}
	tState.bMove = (bits & 0x04) != 0;
	tState.bJump = (bits & 0x08) != 0;
	tState.bShoot = (bits & 0x20) != 0;

	// Ninja rope
	bool rope = (bits & 0x10) != 0;
	if(rope)
		cNinjaRope.read(bs,worms,iID);
	else
		cNinjaRope.Release();

	// Safety check
	if( iCurrentWeapon < 0 || iCurrentWeapon > 4) {
		warnings << "Bad iCurrentWeapon in worm update packet" << endl;
		iCurrentWeapon = 0;
	}

	const Version& versionOfSender = cClient->getServerVersion();
	bool gotVelocity = tState.bShoot || versionOfSender >= OLXBetaVersion(5);

	// Update the position
	CVec oldPos = vPos;
	if(!gotVelocity)
		net_updatePos( CVec(x, y) ); // estimation, sets also velocity
	else
		vPos = CVec(x, y);

	// Velocity
	if(gotVelocity) {
		Sint16 vx = bs->readInt16();
		Sint16 vy = bs->readInt16();
		vPreLastEstimatedVel = vLastEstimatedVel = vVelocity = CVec( (float)vx, (float)vy );
		fPreLastPosUpdate = fLastPosUpdate = tLX->currentTime; // update time to get sure that we don't use old data
		vPreOldPosOfLastPaket = vOldPosOfLastPaket = vPos; // same with pos
	}

	// do carving also here as the simulation is only done in next frame and with an updated position
	
	if(cClient->isMapReady()) {
		/* Earlier, we had calculated from the whole way from vOldPos to vPos (just the direct linear way).
		 * That had several issues:
		 *   - vOldPos could be wrong or very inaccurate for several reasons (big lag or hideandseek or something)
		 *   - no carving if bCarve=false
		 *   - if the other client has made any carving we don't know about, we will thus never now about
		 * Now, we just carve at the place where the worm is right now to hopefully solve some of these issues.
		 */
		incrementDirtCount( CarveHole(vPos) );
	}
	
	if(tState.bCarve && cClient->isMapReady()) {
		/* If we get the updates too infrequently, it could be that we don't have carved everything.
		 * Thus, if the len is not too big, we still carve the whole way.
		 */
		CVec dir = vPos - oldPos;
		float len = NormalizeVector( &dir );
		if(len <= 10.0f)
			for(float w = 0.0f; w <= len; w += 3.0f) {
				CVec p = oldPos + dir * w;
				incrementDirtCount( CarveHole(p) );
			}
	}
	
	// carve a bit further were we are heading to (same as in simulation)
	if(tState.bCarve && cClient->isMapReady()) {

		// Calculate dir
		const CVec dir = getFaceDirection();
		incrementDirtCount( CarveHole(getPos() + dir*4) );
	}

	this->fLastSimulationTime = tLX->currentTime; // - ((float)cClient->getMyPing()/1000.0f) / 2.0f; // estime the up-to-date time
}


///////////////////
// Write out the weapons
void CWorm::writeWeapons(CBytestream *bs)
{
	bs->writeByte(iID);

	for(ushort i=0; i<5; i++) {
		if(tWeapons[i].Enabled) {
			if(tWeapons[i].Weapon)
				bs->writeByte(tWeapons[i].Weapon->ID);
			else {
				bs->writeByte(255);
				errors << "tWeapons[" << i << "].Weapon not set" << endl;
			}
		} else { // not enabled weapon
			bs->writeByte(255);
		}
	}
}


///////////////////
// Read the weapon list
void CWorm::readWeapons(CBytestream *bs)
{
	//notes << "weapons for " << iID << ":" << sName << ": ";
	
	for(ushort i=0; i<5; i++) {
		if(i > 0) notes << ", ";
		int id = bs->readByte();

		tWeapons[i].Weapon = NULL;
		tWeapons[i].Enabled = false;

		if(cGameScript) {
			if(id >= 0 && id < cGameScript->GetNumWeapons()) {
				tWeapons[i].Weapon = cGameScript->GetWeapons() + id;
				tWeapons[i].Enabled = true;
				//notes << tWeapons[i].Weapon->Name;
			}
			else if(id == 255) { // special case to unset weapon
				tWeapons[i].Weapon = NULL;
				tWeapons[i].Enabled = false;
				//notes << "UNSET";
			}
			else {
				warnings << "Error when reading weapons (ID is over num weapons)" << endl;
				tWeapons[i].Weapon = NULL;
				tWeapons[i].Enabled = false;
				//notes << id << "?";
			}
		} else {
			errors << "readWeapons: cGameScript == NULL" << endl;
			tWeapons[i].Enabled = false;
			//notes << id;
		}
	}
	//notes << endl;

	// Reset the weapons
	for(ushort i=0; i<5; i++) {
		tWeapons[i].Charge = 1;
		tWeapons[i].Reloading = false;
		tWeapons[i].SlotNum = i;
		tWeapons[i].LastFire = 0;
	}
	
	bWeaponsReady = true;
}



/////////////
// Synchronizes the "stat needed" checking variables
void CWorm::updateStatCheckVariables()
{
	iLastCharge = CLAMP((int)(tWeapons[iCurrentWeapon].Charge * 100.0f), 0, 100);
	if (tWeapons[iCurrentWeapon].Reloading)
		iLastCharge |= 0x80;
	iLastCurWeapon = iCurrentWeapon;
}


/////////////
// Returns true if we need to send the stat update
bool CWorm::checkStatePacketNeeded()
{
	byte charge = CLAMP((int)(tWeapons[iCurrentWeapon].Charge * 100.0f), 0, 100);
	if (tWeapons[iCurrentWeapon].Reloading)
		charge |= 0x80;
	return (charge != iLastCharge) || (iLastCurWeapon != iCurrentWeapon);
}

///////////////////
// Write a worm stat update
void CWorm::writeStatUpdate(CBytestream *bs)
{
	byte charge = CLAMP((int) (tWeapons[iCurrentWeapon].Charge * 100.0f), 0, 100);

	if(tWeapons[iCurrentWeapon].Reloading)
		charge |= 0x80;

	bs->writeByte( iCurrentWeapon );
	bs->writeByte( charge );
}


///////////////////
// Read a worm stat update
void CWorm::readStatUpdate(CBytestream *bs)
{
	uchar cur = bs->readByte();
	uchar charge = bs->readByte();

	if(!bGameReady) {
		// Can happen at connect-during-game.
		return;
	}
	
    // Check
	if (cur > 4)  {
		warnings << "CWorm::readStatUpdate: current weapon not in range, ignored." << endl;
		return;
	}

	if(!tWeapons[cur].Enabled)
		return;
	
	if(tWeapons[cur].Weapon == NULL) {
		warnings << "readStatUpdate: Weapon " << int(cur) << " not set" << endl;
		return;
	}

	// If this is a special weapon, and the charge is processed client side, don't set the charge
	if( tWeapons[cur].Weapon->Type == WPN_SPECIAL )
		return;



	tWeapons[cur].Reloading = (charge & 0x80) != 0;

	charge &= ~(0x80);

	float c = (float)charge/100.0f;

	if( tWeapons[cur].Reloading && (c > tWeapons[cur].Charge || fabs(c - tWeapons[cur].Charge) > 0.1f) )
		tWeapons[cur].Charge = c;

	if( !tWeapons[cur].Reloading && c < tWeapons[cur].Charge )
		tWeapons[cur].Charge = c;

	// If the server is on the same comp as me, just set the charge normally
	if( tLX->iGameType != GME_JOIN )
		tWeapons[cur].Charge = c;
}
