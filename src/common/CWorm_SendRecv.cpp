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
#include "GfxPrimitives.h"
#include "CWorm.h"
#include "Protocol.h"
#include "CServer.h"
#include "MathLib.h"


///////////////////
// Write my info to a bytestream
void CWorm::writeInfo(CBytestream *bs)
{
	bs->writeString(RemoveSpecialChars(sName));
	bs->writeInt(iType, 1);
	bs->writeInt(iTeam, 1);
    bs->writeString(szSkin);

	Uint8 rgb[3];
    GetColour3(iColour, getMainPixelFormat(), &rgb[0], &rgb[1], &rgb[2]);

	for(short i = 0; i < 3; i++)
		bs->writeInt(rgb[i], 1);
}


///////////////////
// Read info from a bytestream
void CWorm::readInfo(CBytestream *bs)
{
	sName = bs->readString();

	iType = bs->readInt(1) ? 1 : 0;
	iTeam = CLAMP(bs->readInt(1), 0, 3);
    szSkin = bs->readString();

	Uint8 r = bs->readByte();
	Uint8 g = bs->readByte();
	Uint8 b = bs->readByte();
	
	iColour = MakeColour(r, g, b);
	iOldColour = iColour;
}


///////////////////
// Write my score
void CWorm::writeScore(CBytestream *bs)
{
	bs->writeByte(S2C_SCOREUPDATE);
	bs->writeInt(iID, 1);
	bs->writeInt16(iLives);
	bs->writeInt(iKills, 1);
}


///////////////////
// Read my score
void CWorm::readScore(CBytestream *bs)
{
	// NOTE: ID and S2C_SCOREUPDATE is read in CClient::ParseScoreUpdate
	// TODO: make this better
	if (tGameInfo.iLives == WRM_UNLIM)
		iLives = MAX((int)bs->readInt16(),WRM_UNLIM);
	else
		iLives = MAX((int)bs->readInt16(),WRM_OUT);
	iKills = MAX(bs->readInt(1), 0);
}


// Note: We don't put charge into the update packet because we only send the update packet to
//       _other_ worms, not to self


///////////////////
// Write a packet out
void CWorm::writePacket(CBytestream *bs)
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
	if(tState.iCarve)
		bits |= 0x01;
	if(iDirection == DIR_RIGHT)
		bits |= 0x02;
	if(tState.iMove)
		bits |= 0x04;
	if(tState.iJump)
		bits |= 0x08;
	if(cNinjaRope.isReleased())
		bits |= 0x10;
	if(tState.iShoot)
		bits |= 0x20;

	bs->writeByte( bits );
	bs->writeByte( iCurrentWeapon );

	// Write out the ninja rope details
	if(cNinjaRope.isReleased())
		cNinjaRope.write(bs);


	// Velocity

	// The server only needs to know our velocity for shooting
	// So only send the velocity if our shoot flag is set
	if(tState.iShoot) {
		CVec v = vVelocity;
		bs->writeInt16( (Sint16)v.x );
		bs->writeInt16( (Sint16)v.y );
	}

	// Update the "last" variables
	updateCheckVariables();
}

//////////////
// Synchronizes the variables used for check below
void CWorm::updateCheckVariables()
{
	tLastState = tState;
	fLastAngle = fAngle;
	fLastUpdateWritten = tLX->fCurTime;
	iLastCurWeapon = iCurrentWeapon;
	cNinjaRope.updateCheckVariables();
}

////////////////////
// Checks if we need to call writePacket, false when not
bool CWorm::checkPacketNeeded()
{
	// State
	if (tState.iCarve)
		return true;
	if (tState.iShoot && !tWeapons[iCurrentWeapon].Reloading)
		return true;

	if (
		(tLastState.iCarve != tState.iCarve) ||
		(tLastState.iDirection != tState.iDirection)  ||
		(tLastState.iMove != tState.iMove) ||
		(tLastState.iJump != tState.iJump) ||
		(tLastState.iShoot != tState.iShoot))
			return true;

	// Angle
	if (fabs(fLastAngle - fAngle) > 0.00001 && tLX->fCurTime - fLastUpdateWritten > 0.05f)
		return true;

	// TODO: don't check velocity here but the pos-change instead
	// (which is not the same because we have independent physics-simulation)
	// Time
	if (vVelocity.GetLength2())
		if (tLX->fCurTime - fLastUpdateWritten >= MAX(3.0f/vVelocity.GetLength(), 1.0f/80.0f))
			return true;

	// Flag
	if(getFlag())
		return true;

	// Changed weapon
	if(iLastCurWeapon != iCurrentWeapon)
		return true;

	// Rope
	return cNinjaRope.writeNeeded();
}

///////////////////
// Write a packet out (from client 2 server)
/*void CWorm::writeC2SUpdate(CBytestream *bs)
{
	short x = (short)vPos.x;
	short y = (short)vPos.y;

	// Note: This method of saving 1 byte in position, limits the map size to just under 4096x4096

	// Position
	bs->write2Int12( x, y );

	// Angle
	bs->writeInt( (int)fAngle+90, 1);

	// Bit flags
	uchar bits = 0;
	if(tState.iCarve)
		bits |= 0x01;
	if(iDirection == DIR_RIGHT)
		bits |= 0x02;
	if(tState.iMove)
		bits |= 0x04;
	if(tState.iJump)
		bits |= 0x08;
	if(cNinjaRope.isReleased())
		bits |= 0x10;
	if(tState.iShoot)
		bits |= 0x20;

	bs->writeByte( bits );
	bs->writeByte( iCurrentWeapon );

	// Write out the ninja rope details
	if(cNinjaRope.isReleased())
		cNinjaRope.write(bs);
}*/

// this is used to update the position on the client-side in CWorm::readPacketState
// it also updates frequently the velocity by estimation
void CWorm::net_updatePos(const CVec& newpos) {
	if (!cGameScript)
		return;

	if(tLX->fCurTime - fLastPosUpdate > 0.05f) {
		float t = tLX->fCurTime - fLastPosUpdate;
		CVec estimatedVel;
		
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
			estimatedVel = (newpos - vOldPosOfLastPaket) / t;
		}
		else
		{

			// Approximate with current velocity and add gravity
			CVec dist = newpos - vOldPosOfLastPaket;
			CVec a(0, 0);
		
			const gs_worm_t *wd = cGameScript->getWorm();
			// Air drag (Mainly to dampen the ninja rope)
			float Drag = wd->AirFriction;


			if(!bOnGround)	{
				// TODO: this is also not exact
				CVec preEstimatedVel = (newpos - vOldPosOfLastPaket) / t;
				a.x -= SQR(preEstimatedVel.x) * SIGN(preEstimatedVel.x) * Drag;
				a.y -= SQR(preEstimatedVel.y) * SIGN(preEstimatedVel.y) * Drag;
			}

			if (cNinjaRope.isAttached())  {
				a += cNinjaRope.GetForce(newpos);
			}
		
			// Gravity
			a.y += wd->Gravity;
		
			estimatedVel = (dist / t) + (a * t / 2);

			// Ultimate in friction
			if(bOnGround) {
				// HINT: also this isn't exact (it would be like it's only one frame)
				estimatedVel.x *= pow(0.9f, t * 100.0f); // TODO: for simulation, this is not so good for some reason.
				// is it ok here?

				// Too slow, just stop
//				if(fabs(estimatedVel.x) < 5 && !ws->iMove)
//					estimatedVel.x = 0;
			}

			// Process the moving
			float speed = bOnGround ? wd->GroundSpeed : wd->AirSpeed;
			if(tState.iMove) {
				if(iDirection == DIR_RIGHT) {
					// Right
					if(estimatedVel.x < 30)
						estimatedVel.x += speed * 90.0f * t;
				} else {
					// Left
					if(estimatedVel.x > -30)
						estimatedVel.x -= speed * 90.0f * t;
				}
			}
		}
		
		//vVelocity = (vVelocity + estimatedVel) / 2;
		//vVelocity = CVec(0,0); // temp hack
		vVelocity = estimatedVel * 0.5f; // just don't be to fast, unwanted jumps else
		
		fLastPosUpdate = tLX->fCurTime;
		vOldPosOfLastPaket = newpos;
	}
	
	vPos = newpos;
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

	iDirection = DIR_LEFT;
		

	tState.iCarve = (bits & 0x01);
	if(bits & 0x02)
		iDirection = DIR_RIGHT;
	tState.iMove = (bits & 0x04);
	tState.iJump = (bits & 0x08);
	tState.iShoot = (bits & 0x20);

	// Ninja rope
	int rope = (bits & 0x10);
	if(rope)
		cNinjaRope.read(bs,worms,iID);
	else
		cNinjaRope.Release();

	// Velocity
	if(tState.iShoot) {
		Sint16 vx = bs->readInt16();
		Sint16 vy = bs->readInt16();
		vVelocity = CVec( (float)vx, (float)vy );
	}

	CClient *cl = cServer->getClient(iID);
	CWorm *w = cl->getWorm(0);

	// If the worm is inside dirt then it is probably carving
	if (tGameInfo.iGameType == GME_HOST && cServer->getMap()) 
		if(cServer->getMap()->GetPixelFlag(x, y) & PX_DIRT)
			tState.iCarve = true;


	// Prevent a wall hack
	if (tGameInfo.iGameType == GME_HOST && cServer->getMap())  {

		// Out of map
		if(x > (short)cServer->getMap()->GetWidth() || y > (short)cServer->getMap()->GetHeight())
		{
			vPos=vLastPos; 
			cServer->SpawnWorm(w, vPos, cl);
		}

		// In rock
		if(cServer->getMap()->GetPixelFlag(x, y) & PX_ROCK)
		{
			vPos=vLastPos;
			cServer->SpawnWorm(w, vPos, cl);
		}
	
		// TODO: isn't vLastPos intendent to be the same as vOldPos? if so, remove it
		vLastPos = vPos;
	}
}

////////////////
// Skip the packet
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
	if (bits & 0x20)  {
		bs->Skip(4);  // 2*Int16
	}
	return bs->isPosAtEnd();
}


///////////////////
// Read a packet (client side)
void CWorm::readPacketState(CBytestream *bs, CWorm *worms)
{
	// Position
	short x, y;
	bs->read2Int12( x, y );

	// Angle
	tState.iAngle = bs->readInt(1) - 90;

	// Flags
	uchar bits = bs->readByte();
	iCurrentWeapon = (uchar)CLAMP(bs->readByte(), (uchar)0, (uchar)4);
	
	tState.iDirection = DIR_LEFT;

	tState.iCarve = (bits & 0x01);
	if(bits & 0x02)
		tState.iDirection = DIR_RIGHT;
	tState.iMove = (bits & 0x04);
	tState.iJump = (bits & 0x08);
	tState.iShoot = (bits & 0x20);
	
	// Ninja rope
	int rope = (bits & 0x10);
	if(rope)
		cNinjaRope.read(bs,worms,iID);
	else
		cNinjaRope.Release();

	// Safety check
	if( iCurrentWeapon < 0 || iCurrentWeapon > 4) {
		printf("Bad iCurrentWeapon in worm update packet\n");
		iCurrentWeapon = 0;
	}

	// Update the position (estimation, sets also velocity)
	net_updatePos( CVec(x, y) );

	// Velocity
	if(tState.iShoot) {
		Sint16 vx = bs->readInt16();
		Sint16 vy = bs->readInt16();
		vVelocity = CVec( (float)vx, (float)vy );
	}
	
	this->fLastSimulationTime = tLX->fCurTime; // - ((float)cClient->getMyPing()/1000.0f) / 2.0f; // estime the up-to-date time
}

	
///////////////////
// Write out the weapons
void CWorm::writeWeapons(CBytestream *bs)
{
	bs->writeByte(iID);

	for(ushort i=0; i<5; i++) {
		if(tWeapons[i].Weapon)
			bs->writeByte(tWeapons[i].Weapon->ID);
		else
			printf("tWeapons[%d].Weapon not set\n",i);
	}
}


///////////////////
// Read the weapon list
void CWorm::readWeapons(CBytestream *bs)
{
	ushort i;
	int id;
	
	for(i=0; i<5; i++) {
		id = bs->readByte();

		tWeapons[i].Weapon = NULL;
		tWeapons[i].Enabled = true;

		if(cGameScript) {
			if(id >= 0 && id < cGameScript->GetNumWeapons())
				tWeapons[i].Weapon = cGameScript->GetWeapons() + id;
			else
				printf("Error when reading weapons");
		}
	}

	// Reset the weapons
	for(i=0; i<5; i++) {
		tWeapons[i].Charge = 1;
		tWeapons[i].Reloading = false;
		tWeapons[i].SlotNum = i;
		tWeapons[i].LastFire = 0;
	}
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

    // Check
	if (cur > 4)  {
		printf("CWorm::readStatUpdate: current weapon not in range, ignored.\n");
		return;
	}

	if(tWeapons[cur].Weapon == NULL) {
		printf("WARNING: readStatUpdate: Weapon == NULL\n");
		return;
	}
	
	// If this is a special weapon, and the charge is processed client side, don't set the charge
	if( tWeapons[cur].Weapon->Type == WPN_SPECIAL )
		return;


		
	tWeapons[cur].Reloading = charge & 0x80;
	
	charge &= ~(0x80);

	float c = (float)charge/100.0f;

	if( tWeapons[cur].Reloading && (c > tWeapons[cur].Charge || fabs(c - tWeapons[cur].Charge) > 0.1f) )
		tWeapons[cur].Charge = c;

	if( !tWeapons[cur].Reloading && c < tWeapons[cur].Charge )
		tWeapons[cur].Charge = c;

	// If the server is on the same comp as me, just set the charge normally
	if( tGameInfo.iGameType != GME_JOIN )
		tWeapons[cur].Charge = c;
}
