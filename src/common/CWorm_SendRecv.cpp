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


///////////////////
// Write my info to a bytestream
void CWorm::writeInfo(CBytestream *bs)
{
	bs->writeString(RemoveSpecialChars(sName));
	bs->writeInt(iType,1);
	bs->writeInt(iTeam,1);
    bs->writeString(szSkin);

	for(ushort i=0;i<3;i++)
		bs->writeInt(iColComps[i],1);
}


///////////////////
// Read info from a bytestream
void CWorm::readInfo(CBytestream *bs)
{
	sName = bs->readString();

	iType = MAX(MIN(bs->readInt(1),1),0);
	iTeam = MAX(MIN(bs->readInt(1),3),0);
    szSkin = bs->readString();

	Uint8 r = bs->readByte();
	Uint8 g = bs->readByte();
	Uint8 b = bs->readByte();

	iColComps[0] = r;
	iColComps[1] = g;
	iColComps[2] = b;
	
	iColour = MakeColour(r,g,b);
}


///////////////////
// Write my score
void CWorm::writeScore(CBytestream *bs)
{
	bs->writeByte(S2C_SCOREUPDATE);
	bs->writeInt(iID,1);
	bs->writeInt16(iLives);
	bs->writeInt(iKills,1);
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
	iKills = MAX(bs->readInt(1),0);
}


// Note: We don't put charge into the update packet because we only send the update packet to
//       _other_ worms, not to self


///////////////////
// Write a packet out
void CWorm::writePacket(CBytestream *bs)
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


	// Velocity

	// The server only needs to know our velocity for shooting
	// So only send the velocity if our shoot flag is set
	if(tState.iShoot) {
		CVec v = vVelocity;
		bs->writeInt16( (Sint16)v.x );
		bs->writeInt16( (Sint16)v.y );
	}
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


///////////////////
// Read a packet (server side)
void CWorm::readPacket(CBytestream *bs, CWorm *worms)
{
	// Position
	short x, y;
	bs->read2Int12( x, y );
	vPos.x=( (float)x );
	vPos.y=( (float)y );	

	// Angle
	fAngle = (float)bs->readInt(1) - 90;

	// Flags
	uchar bits = bs->readByte();
	iCurrentWeapon = bs->readByte();

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
	vPos.x=( (float)x );
	vPos.y=( (float)y );

	// Angle
	tState.iAngle = bs->readInt(1) - 90;

	// Flags
	uchar bits = bs->readByte();
	iCurrentWeapon = bs->readByte();
	
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

	// Velocity
	if(tState.iShoot) {
		Sint16 vx = bs->readInt16();
		Sint16 vy = bs->readInt16();
		vVelocity = CVec( (float)vx, (float)vy );
	}
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


///////////////////
// Write a worm stat update
void CWorm::writeStatUpdate(CBytestream *bs)
{
	byte charge = (int) (tWeapons[iCurrentWeapon].Charge * 100.0f);
    charge = MAX(charge, (byte)0);
    charge = MIN(charge, (byte)100);

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

    // TODO: isn't it saver / more sensefull to ignore the info if cur>4 ?
	cur = (uchar)MIN(cur, 4);

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
