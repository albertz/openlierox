/////////////////////////////////////////
//
//              OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Game script class
// Created 7/2/02
// Jason Boettcher


#ifndef __CGAMESCRIPT_H__
#define __CGAMESCRIPT_H__

#include "Sounds.h"
#include "GfxPrimitives.h"
#include "CProjectile.h"

#define		GS_VERSION		7

// Error codes
#define		GSE_OK			1
#define		GSE_MEM			0
#define		GSE_VERSION		-1
#define		GSE_FILE		-2
#define		GSE_BAD			-3


// Weapon classes
#define		WCL_AUTOMATIC	0
#define		WCL_POWERGUN	1
#define		WCL_GRENADE		3
#define		WCL_MISSILE		4
#define		WCL_CLOSERANGE	5

// Weapon types
#define		WPN_PROJECTILE	0
#define		WPN_SPECIAL		1
#define		WPN_BEAM		2

// Projectile types
#define		PRJ_PIXEL		0
#define		PRJ_IMAGE		1

// Projectile method types
#define		PJ_BOUNCE		0
#define		PJ_EXPLODE		1
#define		PJ_INJURE		2
#define		PJ_CARVE		4
#define		PJ_DIRT			5
#define     PJ_GREENDIRT    6
#define     PJ_DISAPPEAR    7
#define		PJ_NOTHING		8

// Projectile trails
#define		TRL_NONE		0
#define		TRL_SMOKE		1
#define		TRL_CHEMSMOKE	2
#define		TRL_PROJECTILE	3
#define		TRL_DOOMSDAY	4
#define		TRL_EXPLOSIVE	5

// Animation types
#define		ANI_ONCE		0
#define		ANI_LOOP		1
#define		ANI_PINGPONG	2

// Special Weapons
#define		SPC_NONE		0
#define		SPC_JETPACK		1


// Header structure
// WARNING: never change this!
// it's used in CGameScript.cpp and it represents
// the original file format
typedef struct {

	char	ID[18];
	Uint32	Version;
	char	ModName[64];

} gs_header_t;


// Special structure
typedef struct {
	Uint32	Thrust;
} gs_special_t;


// Worm structure
// WARNING: never change this!
// it's used in CGameScript.cpp and it represents
// the original file format
typedef struct {
	float	AngleSpeed;

	float	GroundSpeed;
	float	AirSpeed;
	float	Gravity;
	float	JumpForce;
	float	AirFriction;
	float	GroundFriction;

} gs_worm_t;








// Weapon structure
class weapon_t { public:
	int		ID;
	UCString	Name; // (was 64b before)
	int		Type;
	int		Special;
	int		Class;
	int		Recoil;
	float	Recharge;
	float	Drain;
	float	ROF;
	int		UseSound;
	UCString	SndFilename; // (was 64b before)
	int		LaserSight;
	
	// Projectile
	int		ProjSpeed;
	float	ProjSpread;
	float	ProjSpeedVar;
	int		ProjAmount;
	proj_t	*Projectile;

	// Beam
	int		Bm_Colour[3];
	int		Bm_Damage;
	int		Bm_PlyDamage;
	int		Bm_Length;


	// Special
	gs_special_t tSpecial;

#ifndef _CONSOLE
	SoundSample*	smpSample;
#endif

};


class CGameScript {
public:
	// Constructor
	CGameScript() {

		NumWeapons = 0;
		Weapons = NULL;
        pModLog = NULL;

		RopeLength = 150;
		RestLength = 20;
		Strength = 0.5f;
	}


private:
	// Attributes

	UCString		sDirectory;


	// Header
	gs_header_t	Header;
	

	// Weapons
	int			NumWeapons;
	weapon_t	*Weapons;

	// Worm
	gs_worm_t	Worm;


	// Ninja Rope
	int			RopeLength;
	int			RestLength;
	float		Strength;

    // Mod log file
    FILE        *pModLog;


public:
	// Methods

	int			Load(const UCString& dir);
	proj_t		*LoadProjectile(FILE *fp);
	
	int			Save(const UCString& filename);
	int			SaveProjectile(proj_t *proj, FILE *fp);

    void        writeString(const UCString& szString, FILE *fp);
    UCString readString(FILE *fp);

	void		Shutdown(void);
	void		ShutdownProjectile(proj_t *prj);

	UCString	getError(int code);

	weapon_t	*FindWeapon(const UCString& name);
    bool        weaponExists(const UCString& szName);

	static int	CheckFile(const UCString& dir, UCString& name);

    void        modLog(char *fmt, ...);

#ifndef _CONSOLE
	SDL_Surface* LoadGSImage(const UCString& dir, const UCString& filename);
	SoundSample* LoadGSSample(const UCString& dir, const UCString& filename);
#endif


	inline gs_header_t	*GetHeader(void)				{ return &Header; }

	inline int			GetNumWeapons(void)				{ return NumWeapons; }
	inline weapon_t	*GetWeapons(void)				{ return Weapons; }

	inline void		SetNumWeapons(int _w)			{ NumWeapons = _w; }
	inline void		SetWeapons(weapon_t *_w)		{ Weapons = _w; }


	// Ninja Rope settings
	inline void		SetRopeLength(int _l)			{ RopeLength = _l; }
	inline void		SetRestLength(int _l)			{ RestLength = _l; }
	inline void		SetStrength(float _s)			{ Strength = _s; }

	inline int			getRopeLength(void)				{ return RopeLength; }
	inline int			getRestLength(void)				{ return RestLength; }
	inline float		getStrength(void)				{ return Strength; }

	inline gs_worm_t	*getWorm(void)					{ return &Worm; }
	

};





#endif  //  __CGAMESCRIPT_H__
