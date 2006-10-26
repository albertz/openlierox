/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Shoot list class
// Created 20/12/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"



///////////////////
// Initialize the list
bool CShootList::Initialize(void)
{
	m_nNumShootings = 0;
	m_fStartTime = -1;

	m_psShoot = new shoot_t[MAX_SHOOTINGS];
	if( m_psShoot == NULL )
		return false;

	return true;
}


///////////////////
// Shutdown the list
void CShootList::Shutdown(void)
{
	if( m_psShoot ) {
		delete[] m_psShoot;
		m_psShoot = NULL;
	}

	m_nNumShootings = 0;
	m_fStartTime = -1;
}


///////////////////
// Clear the list
void CShootList::Clear(void)
{
	m_nNumShootings = 0;
	m_fStartTime = -1;
}


///////////////////
// Return a shot
shoot_t *CShootList::getShot( int index )
{
	if( index >= 0 && index < m_nNumShootings )
		return &m_psShoot[index];

	return NULL;
}


///////////////////
// Add a shot to the list
bool CShootList::addShoot( float fTime, float fSpeed, int nAngle, CWorm *pcWorm )
{
	assert( pcWorm );
	assert( m_psShoot );

	// Check if it isn't too full
	if( m_nNumShootings >= MAX_SHOOTINGS )
		return false;

	// If this is the first shot in the list, set the start time
	if( m_fStartTime == -1)
		m_fStartTime = tLX->fCurTime;

	shoot_t *psShot = &m_psShoot[ m_nNumShootings++ ];

	// Fill in the info
	psShot->cPos = pcWorm->getPos();
	psShot->cWormVel = *pcWorm->getVelocity();
	psShot->fTime = fTime;
	psShot->nAngle = nAngle;
	psShot->nRandom = GetRandomInt(255);
	psShot->nSpeed = (int)( fSpeed*100 );
	psShot->nWeapon = pcWorm->getCurWeapon()->Weapon->ID;
	psShot->nWormID = pcWorm->getID();

	// Debuging
	psShot->devID = 0;
	if(m_nNumShootings > 1)
		psShot->devID = m_psShoot[m_nNumShootings-2].devID+1;

	return true;
}


///////////////////
// Write the shoot list to a bytestream
// Returns true if good (even if we didn't write anything)
// Returns false if the packet couldn't be written (overflow)
bool CShootList::writePacket( CBytestream *bs )
{
	CBytestream strm;

	// If no shots are in the list, just leave
	if( m_nNumShootings == 0)
		return true;

	// Single shot
	if( m_nNumShootings == 1 )
		writeSingle( &strm, 0 );
	else
		// Multiple shots
		writeMulti( &strm, 0 );


	// If there is no room in the bytestream, return false so the server knows that the client has
	// overflowed (during lag)
	if(strm.GetLength() + bs->GetPos() > MAX_DATA)
		return false;

	// Otherwise, append the shootlist onto the bytestream
	// Don't append for now
	bs->Append( &strm );
	m_fLastWrite = tLX->fCurTime;

	return true;
}


///////////////////
// A single shot packet
void CShootList::writeSingle( CBytestream *bs, int index )
{
	byte	flags = 0;
	shoot_t *psShot = &m_psShoot[index];	

	// Set the flags
	if( psShot->nAngle > 255 )
		flags |= SMF_LARGEANGLE;			// Large angle
	if( psShot->nSpeed > 255 || psShot->nSpeed < -255)
		flags |= SMF_LARGESPEED;			// Large speed
	if( psShot->nSpeed < 0 )
		flags |= SMF_NEGSPEED;				// Negative speed


	int x = (int)psShot->cPos.GetX();
	int y = (int)psShot->cPos.GetY();
	int vx = (int)psShot->cWormVel.GetX();
	int vy = (int)psShot->cWormVel.GetY();
	
	// Write the packet
	bs->writeByte( S2C_SINGLESHOOT );
	bs->writeByte( flags );
	bs->writeByte( psShot->nWormID );
	bs->writeFloat( psShot->fTime );
	bs->writeByte( psShot->nWeapon );
	bs->writeByte( x );
	bs->writeByte( (x>>8) | (y<<4) );
	bs->writeByte( (y>>4) );
	bs->writeShort( vx );
	bs->writeShort( vy );
	bs->writeByte( psShot->nRandom );
	
	if( flags & SMF_LARGEANGLE )
		bs->writeByte( psShot->nAngle-255 );
	else
		bs->writeByte( psShot->nAngle );
	
	if( flags & SMF_LARGESPEED )
		bs->writeByte( psShot->nSpeed-255 );
	else if( flags & SMF_NEGSPEED ) {
		if(flags & SMF_LARGESPEED)
			bs->writeByte( (-psShot->nSpeed)-255 );
		else
			bs->writeByte( -psShot->nSpeed );
	}
	else
		bs->writeByte( psShot->nSpeed );
}


///////////////////
// A multiple shot packet
void CShootList::writeMulti( CBytestream *bs, int index )
{	
	// If index is over the limit, exit
	if( index >= m_nNumShootings )
		return;

	// If index is the last shot, send a single shot & exit
	if( index == m_nNumShootings-1 ) {
		writeSingle( bs, index );
		return;
	}


	int		num = 1;
	int		wmid = m_psShoot[index].nWormID;
	int		wpid = m_psShoot[index].nWeapon;
	int		i;

	// Calculate how many shots we can/should send in a row
	// (based on worm id, weapon id, angle, time, number, speed & worm velocity)
	for(i=index+1; i<m_nNumShootings; i++) {
		if( m_psShoot[i].nWormID != wmid ||
			m_psShoot[i].nWeapon != wpid ||
			(m_psShoot[i].fTime - m_psShoot[i-1].fTime)*1000 > 255 ||
			abs(m_psShoot[i].nAngle - m_psShoot[i-1].nAngle) > 255 ||
			abs(m_psShoot[i].nSpeed - m_psShoot[i-1].nSpeed) > 255 ||
			abs((int)m_psShoot[i].cWormVel.GetX() - (int)m_psShoot[i-1].cWormVel.GetX()) > 255 ||
			abs((int)m_psShoot[i].cWormVel.GetY() - (int)m_psShoot[i-1].cWormVel.GetY()) > 255 ||
			num > 255) {

			break;
		}

		num++;
	}


	// If we can only send 1, send the 'single' packet
	if( num == 1 ) {
		writeSingle(bs, index);

		// If we could only send one shot, but we still have more shots to send, send them
		if( m_nNumShootings > 1)
			writeMulti(bs, index+1);
		return;
	}

	
	// Write out the header
	byte	flags = 0;
	shoot_t *psShot = &m_psShoot[index];	

	// Set the flags
	if( psShot->nAngle > 255 )
		flags |= SMF_LARGEANGLE;			// Large angle
	if( psShot->nSpeed > 255 || psShot->nSpeed < -255)
		flags |= SMF_LARGESPEED;			// Large speed
	if( psShot->nSpeed < 0 )
		flags |= SMF_NEGSPEED;				// Negative speed


	int x = (int)psShot->cPos.GetX();
	int y = (int)psShot->cPos.GetY();
	int vx = (int)psShot->cWormVel.GetX();
	int vy = (int)psShot->cWormVel.GetY();

	// Write the packet
	bs->writeByte( S2C_MULTISHOOT );
	bs->writeByte( flags );
	bs->writeByte( psShot->nWormID );
	bs->writeFloat( psShot->fTime );
	bs->writeByte( psShot->nWeapon );
	bs->writeByte( num );
	bs->writeByte( x );
	bs->writeByte( (x>>8) | (y<<4) );
	bs->writeByte( (y>>4) );
	bs->writeShort( vx );
	bs->writeShort( vy );
	bs->writeByte( psShot->nRandom );
	
	if( flags & SMF_LARGEANGLE )
		bs->writeByte( psShot->nAngle-255 );
	else
		bs->writeByte( psShot->nAngle );

	int speed = psShot->nSpeed;
	
	if( flags & SMF_LARGESPEED )
		speed = psShot->nSpeed-255;
	if( flags & SMF_NEGSPEED ) {
		if(flags & SMF_LARGESPEED)
			speed = (-psShot->nSpeed)-255;
		else
			speed = -psShot->nSpeed;
	}

	bs->writeByte( speed );

	// Send out the following shots
	for(i=1; i<num; i++)
		writeSmallShot( &m_psShoot[index+i-1], bs, index+i );

	// Continue writing out multiple packets until all shots are written
	writeMulti( bs, index+num );
}


///////////////////
// Write a small shot out
void CShootList::writeSmallShot( shoot_t *psFirst, CBytestream *bs, int index )
{
	byte	flags = 0;
	byte	extraflags = 0;

	assert( index < m_nNumShootings );

	shoot_t	*psShot = &m_psShoot[index];

	
	int xoff = (int)( psShot->cPos.GetX() - psFirst->cPos.GetX() );
	int yoff = (int)( psShot->cPos.GetX() - psFirst->cPos.GetX() );
	xoff = abs(xoff);
	yoff = abs(yoff);

	// Set the flags
	if( psShot->fTime != psFirst->fTime )
		flags |= SHF_TIMEOFF;
	if( (int)psShot->cPos.GetX() != (int)psFirst->cPos.GetX() )
		flags |= SHF_XPOSOFF;
	if( (int)psShot->cPos.GetY() != (int)psFirst->cPos.GetY() )
		flags |= SHF_YPOSOFF;
	if( psShot->nAngle != psFirst->nAngle )
		flags |= SHF_ANGLEOFF;
	if( psShot->nSpeed != psFirst->nSpeed )
		flags |= SHF_SPEEDOFF;
	if( (int)psShot->cPos.GetX() < (int)psShot->cPos.GetX() ) {
		flags |= SHF_NG_XPOSOFF;
		flags &= ~SHF_XPOSOFF;
	}
	if( (int)psShot->cPos.GetY() < (int)psShot->cPos.GetY() ) {
		flags |= SHF_NG_YPOSOFF;
		flags &= ~SHF_YPOSOFF;
	}

	// Extra flags
	if( psShot->nAngle < psFirst->nAngle ) {
		extraflags |= SHF_NG_ANGLEOFF;
		flags &= ~SHF_ANGLEOFF;
	}
	if( psShot->nSpeed < psFirst->nSpeed ) {
		extraflags |= SHF_NG_SPEEDOFF;
		flags &= ~SHF_SPEEDOFF;
	}
	if( xoff >= 32 )
		extraflags |= SHF_LARGEXOFF;
	if( yoff >= 32 )
		extraflags |= SHF_LARGEYOFF;

	if( (int)psShot->cWormVel.GetX() != (int)psFirst->cWormVel.GetX() )
		flags |= SHF_XWRMVEL;
	if( (int)psShot->cWormVel.GetY() != (int)psFirst->cWormVel.GetY() )
		flags |= SHF_YWRMVEL;
	if( (int)psShot->cWormVel.GetX() < (int)psFirst->cWormVel.GetX() ) {
		extraflags &= ~SHF_XWRMVEL;
		extraflags |= SHF_NG_XWRMVEL;
	}
	if( (int)psShot->cWormVel.GetY() < (int)psFirst->cWormVel.GetY() ) {
		extraflags &= ~SHF_YWRMVEL;
		extraflags |= SHF_NG_YWRMVEL;
	}


	if( extraflags )
		flags |= SHF_EXTRAFLAGS;


	// For now, do some asserts for data that can't be written in the space
	assert( abs( psShot->nAngle - psFirst->nAngle ) <= 255 );
	//assert( abs( psShot->nSpeed - psFirst->nSpeed ) <= 255 );


	// Write the data
	bs->writeByte( flags );
	if( flags & SHF_EXTRAFLAGS )
		bs->writeByte( extraflags );
	if( flags & SHF_TIMEOFF )
		bs->writeByte( (int)( (psShot->fTime - psFirst->fTime) * 1000.0f) );
	
	// X offset
	if( flags & SHF_XPOSOFF || flags & SHF_NG_XPOSOFF ) {
		if( extraflags & SHF_LARGEXOFF )
			bs->writeByte( xoff );
	}
	// Y offset
	if( flags & SHF_YPOSOFF || flags & SHF_NG_YPOSOFF ) {
		if( extraflags & SHF_LARGEYOFF )
			bs->writeByte( yoff );
	}

	// X & Y Small offset
	if( (flags & SHF_XPOSOFF || flags & SHF_NG_XPOSOFF) && (flags & SHF_YPOSOFF || flags & SHF_NG_YPOSOFF) ) {
		if( !(extraflags & SHF_LARGEXOFF) && !(extraflags & SHF_LARGEYOFF) )
			bs->writeByte( xoff | (yoff<<4) );
	}

	if( flags & SHF_ANGLEOFF )
		bs->writeByte( psShot->nAngle - psFirst->nAngle );
	if( flags & SHF_SPEEDOFF )
		bs->writeByte( psShot->nSpeed - psFirst->nSpeed );
	if( extraflags & SHF_NG_ANGLEOFF )
		bs->writeByte( psFirst->nAngle - psShot->nAngle );
	if( extraflags & SHF_NG_SPEEDOFF )
		bs->writeByte( psFirst->nSpeed - psShot->nSpeed );
	
	// Worm velocity
	if( extraflags & SHF_XWRMVEL )
		bs->writeByte( (int)psShot->cWormVel.GetX() - (int)psFirst->cWormVel.GetX() );
	if( extraflags & SHF_YWRMVEL )
		bs->writeByte( (int)psShot->cWormVel.GetY() - (int)psFirst->cWormVel.GetY() );
	if( extraflags & SHF_NG_XWRMVEL )
		bs->writeByte( (int)psFirst->cWormVel.GetX() - (int)psShot->cWormVel.GetX() );
	if( extraflags & SHF_NG_YWRMVEL )
		bs->writeByte( (int)psFirst->cWormVel.GetY() - (int)psShot->cWormVel.GetY() );
}



///////////////////
// Read a single shot packet
void CShootList::readSingle( CBytestream *bs )
{
	// Clear the list
	Clear();
	
	byte	flags = 0;
	byte	pos[3];
	short	vx, vy;
	shoot_t *psShot = &m_psShoot[0];


	// Read the packet
	flags = bs->readByte();
	psShot->nWormID = bs->readByte();
	psShot->fTime = bs->readFloat();
	psShot->nWeapon = bs->readByte();
	pos[0] = bs->readByte();
	pos[1] = bs->readByte();
	pos[2] = bs->readByte();
	vx = bs->readShort();
	vy = bs->readShort();
	psShot->nRandom = bs->readByte();

	psShot->nAngle = bs->readByte();
	int speed = bs->readByte();
	psShot->nSpeed = speed;
	
	if( flags & SMF_LARGEANGLE )
		psShot->nAngle += 255;

	if( flags & SMF_LARGESPEED )
		psShot->nSpeed = speed+255;
	if( flags & SMF_NEGSPEED ) {
		if( flags & SMF_LARGESPEED )
			psShot->nSpeed = -(speed+255);
		else
			psShot->nSpeed = -speed;
	}


	// Convert the pos
	int x = pos[0] + ((pos[1]&15)<<8);
	int y = (pos[1]>>4) + (pos[2]<<4);
	psShot->cPos = CVec( (float)x, (float)y );

	// Convert the velocity	
	psShot->cWormVel = CVec( (float)vx, (float)vy );

	m_nNumShootings++;
}


///////////////////
// Read a multi shot packet
void CShootList::readMulti( CBytestream *bs )
{
	// Clear the list
	Clear();

	// Read the header
	byte	flags = 0;
	byte	pos[3];
	short	vx, vy;
	int		i;
	int		num = 0;
	shoot_t *psShot = m_psShoot;
	
	flags = bs->readByte();
	psShot->nWormID = bs->readByte();
	psShot->fTime = bs->readFloat();
	psShot->nWeapon = bs->readByte();
	num = bs->readByte();
	pos[0] = bs->readByte();
	pos[1] = bs->readByte();
	pos[2] = bs->readByte();
	vx = bs->readShort();
	vy = bs->readShort();
	psShot->nRandom = bs->readByte();

	psShot->nAngle = bs->readByte();
	int speed = bs->readByte();
	psShot->nSpeed = speed;
	
	if( flags & SMF_LARGEANGLE )
		psShot->nAngle += 255;

	if( flags & SMF_LARGESPEED )
		psShot->nSpeed = speed+255;
	if( flags & SMF_NEGSPEED ) {
		if( flags & SMF_LARGESPEED )
			psShot->nSpeed = -(speed+255);
		else
			psShot->nSpeed = -speed;
	}	

	// Convert the pos
	int x = pos[0] + ((pos[1]&15)<<8);
	int y = (pos[1]>>4) + (pos[2]<<4);
	psShot->cPos = CVec( (float)x, (float)y );

	// Convert the velocity
	psShot->cWormVel = CVec( (float)vx, (float)vy );

	m_nNumShootings++;

	// Read the following shots
	for(i=1; i<num; i++)
		readSmallShot( &m_psShoot[i-1], bs, i );
}


///////////////////
// Read a small shot
void CShootList::readSmallShot( shoot_t *psFirst, CBytestream *bs, int index )
{
	byte	flags = 0;
	byte	extraflags = 0;
	int		x = 0;
	int		y = 0;
	int		pos;

	shoot_t	*psShot = &m_psShoot[index];

	// Copy the base data, so we only apply the changes to the shot
	*psShot = *psFirst;


	flags = bs->readByte();
	if( flags & SHF_EXTRAFLAGS )
		extraflags = bs->readByte();

	// Time offset
	if( flags & SHF_TIMEOFF ) {
		float time = (float)bs->readByte();
		psShot->fTime += time/1000.0f;
	}

	// X Large offset
	if( flags & SHF_XPOSOFF || flags & SHF_NG_XPOSOFF ) {
		if( extraflags & SHF_LARGEXOFF )
			x = bs->readByte();
	}

	// Y Large offset
	if( flags & SHF_YPOSOFF || flags & SHF_NG_YPOSOFF ) {
		if( extraflags & SHF_LARGEYOFF )
			y = bs->readByte();
	}

	// X & Y Small offset
	if( (flags & SHF_XPOSOFF || flags & SHF_NG_XPOSOFF) && (flags & SHF_YPOSOFF || flags & SHF_NG_YPOSOFF) ) {
		if( !(extraflags & SHF_LARGEXOFF) && !(extraflags & SHF_LARGEYOFF) ) {
			pos = bs->readByte();
			x = pos & 15;
			y = pos >> 4;
		}
	}

	// Negate the position offsets
	if( flags & SHF_NG_XPOSOFF )
		x = -x;
	if( flags & SHF_NG_YPOSOFF )
		y = -y;

	
	if( flags & SHF_ANGLEOFF ) {
		int angle = bs->readByte();
		psShot->nAngle += angle;
	}
	if( flags & SHF_SPEEDOFF ) {
		int speed = bs->readByte();
		psShot->nSpeed += speed;
	}

	if( extraflags & SHF_NG_ANGLEOFF ) {
		int angle = bs->readByte();
		psShot->nAngle -= angle;
	}
	if( extraflags & SHF_NG_SPEEDOFF ) {
		int speed = bs->readByte();
		psShot->nSpeed -= speed;
	}

	// Worm velocity
	if( extraflags & SHF_XWRMVEL ) {
		int velx = bs->readByte();
		psShot->cWormVel.SetX( psShot->cWormVel.GetX() + (float)velx );
	}
	if( extraflags & SHF_YWRMVEL ) {
		int vely = bs->readByte();
		psShot->cWormVel.SetY( psShot->cWormVel.GetY() + (float)vely );
	}
	if( extraflags & SHF_NG_XWRMVEL ) {
		int velx = bs->readByte();
		psShot->cWormVel.SetX( psShot->cWormVel.GetX() - (float)velx );
	}
	if( extraflags & SHF_NG_YWRMVEL ) {
		int vely = bs->readByte();
		psShot->cWormVel.SetY( psShot->cWormVel.GetY() - (float)vely );
	}

	m_nNumShootings++;
}
