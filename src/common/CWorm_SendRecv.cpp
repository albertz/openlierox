/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Worm class - Writing & Reading of packets
// Created 24/12/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


///////////////////
// Write my info to a bytestream
void CWorm::writeInfo(CBytestream *bs)
{
	bs->writeString("%s",sName);
	bs->writeInt(iType,1);
	bs->writeInt(iTeam,1);
    bs->writeString("%s",szSkin);

	for(int i=0;i<3;i++)
		bs->writeInt(iColComps[i],1);
}


///////////////////
// Read info from a bytestream
void CWorm::readInfo(CBytestream *bs)
{
	bs->readString(sName);	

	iType = bs->readInt(1);
	iTeam = bs->readInt(1);
    bs->readString(szSkin);

	Uint8 r = bs->readInt(1);
	Uint8 g = bs->readInt(1);
	Uint8 b = bs->readInt(1);

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
	bs->writeShort(iLives);
	bs->writeInt(iKills,1);
}


///////////////////
// Read my score
void CWorm::readScore(CBytestream *bs)
{
	iLives = bs->readShort();
	iKills = bs->readInt(1);
}


// Note: We don't put charge into the update packet because we only send the update packet to
//       _other_ worms, not to self


///////////////////
// Write a packet out
void CWorm::writePacket(CBytestream *bs)
{
	short x = (short)vPos.GetX();
	short y = (short)vPos.GetY();

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
		bs->writeShort( (short)v.GetX() );
		bs->writeShort( (short)v.GetY() );
	}
}


///////////////////
// Write a packet out (from client 2 server)
/*void CWorm::writeC2SUpdate(CBytestream *bs)
{
	short x = (short)vPos.GetX();
	short y = (short)vPos.GetY();

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
	vPos.SetX( (float)x );
	vPos.SetY( (float)y );	

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
		short vx = bs->readShort();
		short vy = bs->readShort();
		vVelocity = CVec( (float)vx, (float)vy );
	}
}


///////////////////
// Read a packet (client side)
void CWorm::readPacketState(CBytestream *bs, CWorm *worms)
{
	// Position
	short x, y;
	bs->read2Int12( x, y );
	vPos.SetX( (float)x );
	vPos.SetY( (float)y );

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
		d_printf("Bad iCurrentWeapon in worm update packet\n");
		iCurrentWeapon = 0;
	}

	// Velocity
	if(tState.iShoot) {
		short vx = bs->readShort();
		short vy = bs->readShort();
		vVelocity = CVec( (float)vx, (float)vy );
	}
}


///////////////////
// Write out the weapons
void CWorm::writeWeapons(CBytestream *bs)
{
	bs->writeByte(iID);

	for(int i=0; i<5; i++) {
		if(tWeapons[i].Weapon)
			bs->writeByte(tWeapons[i].Weapon->ID);
		else
			d_printf("tWeapons[%d].Weapon not set\n",i);
	}
}


///////////////////
// Read the weapon list
void CWorm::readWeapons(CBytestream *bs)
{
	int i;

	for(i=0; i<5; i++) {
		int id = bs->readByte();

		tWeapons[i].Weapon = NULL;

		if(cGameScript) {
			if(id >= 0 && id < cGameScript->GetNumWeapons())
				tWeapons[i].Weapon = cGameScript->GetWeapons() + id;
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
	int cur = bs->readByte();
	int charge = bs->readByte();


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
