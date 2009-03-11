/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Shoot list class
// Created 20/12/02
// Jason Boettcher


#ifndef __CSHOOTLIST_H__
#define __CSHOOTLIST_H__


class CBytestream;
class CWorm;


// Shoot Main flags
#define		SMF_LARGEANGLE	0x01
#define		SMF_LARGESPEED	0x02
#define		SMF_NEGSPEED	0x04


// Shoot Offsets (positive values)
#define		SHF_TIMEOFF		0x01
#define		SHF_XPOSOFF		0x02
#define		SHF_YPOSOFF		0x04
#define		SHF_ANGLEOFF	0x08
#define		SHF_SPEEDOFF	0x10

// Shoot Offsets (negative values)
#define		SHF_NG_XPOSOFF	0x20
#define		SHF_NG_YPOSOFF	0x40
#define		SHF_EXTRAFLAGS	0x80

// Shoot Extra offsets
#define		SHF_NG_ANGLEOFF	0x01
#define		SHF_NG_SPEEDOFF 0x02
#define		SHF_LARGEXOFF	0x04
#define		SHF_LARGEYOFF	0x08
#define		SHF_XWRMVEL		0x10
#define		SHF_YWRMVEL		0x20
#define		SHF_NG_XWRMVEL	0x40
#define		SHF_NG_YWRMVEL	0x80


#define		MAX_SHOOTINGS	256


// Weapon Shooting structure
class shoot_t {
public:

	TimeDiff	fTime;
	int		nWeapon;
	CVec	cPos;
	CVec	cWormVel;
	int		nAngle;
	int		nRandom;
	int		nSpeed;
	int		nWormID;

	int		devID;

};



// Shoot list class
class CShootList {
public:
	// Constructor
	CShootList() {
		m_nNumShootings = 0;
		m_psShoot = NULL;
		m_fStartTime = Time();
		m_fLastWrite = Time();
	}

	// Destructor
	~CShootList() {
		Shutdown();
	}


private:
	// Attributes

	int			m_nNumShootings;
	shoot_t		*m_psShoot;
	Time		m_fStartTime;
	Time		m_fLastWrite;



public:
	// Methods

	bool		Initialize(void);
	void		Shutdown(void);

	bool		addShoot(TimeDiff fTime, float fSpeed, int nAngle, CWorm *pcWorm);

	bool		writePacket(CBytestream *bs);
	
private:
	void		writeSingle(CBytestream *bs, int index);
	void		writeMulti(CBytestream *bs, int index);
	void		writeSmallShot(shoot_t *psFirst, CBytestream *bs, int index);

public:
	void		readSingle(CBytestream *bs, int max_weapon_id);
	static bool skipSingle(CBytestream *bs);
	void		readMulti(CBytestream *bs, int max_weapon_id);
	static bool skipMulti(CBytestream *bs);
	void		readSmallShot(shoot_t *psFirst, CBytestream *bs, int index);
	static bool	skipSmallShot(CBytestream *bs);

	void		Clear(void);

	shoot_t		*getShot(int index);

	int			getNumShots(void)			{ return m_nNumShootings; }
	Time		getStartTime(void)			{ return m_fStartTime; }
	Time		getLastWrite(void)			{ return m_fLastWrite; }

};




#endif  //  __CSHOOTLIST_H__
