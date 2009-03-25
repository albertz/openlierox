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
#include "Version.h"

static const Version GS_MinLxVersion[] = {
	Version(), // for GS_VERSION == 7
	OLXBetaVersion(9), // for GS_VERSION == 8
};

#define		GS_LX56_VERSION	7
#define		GS_FIRST_SUPPORTED_VERSION	GS_LX56_VERSION
#define		GS_MINLXVERSION(ver)	GS_MinLxVersion[ver - GS_FIRST_SUPPORTED_VERSION]
// current most recent version
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
struct gs_header_t {
	gs_header_t() { ID[0] = 0; Version = 0; ModName[0] = 0; }
	char	ID[18];
	Uint32	Version;
	char	ModName[64];
};


// Special structure
struct gs_special_t {
	Uint32	Thrust;
};


// Worm structure
// WARNING: never change this!
// it's used in CGameScript.cpp and it represents
// the original file format
struct gs_worm_t {
	float	AngleSpeed;

	float	GroundSpeed;
	float	AirSpeed;
	float	Gravity;
	float	JumpForce;
	float	AirFriction;
	float	GroundFriction;

};




class proj_t;



// Weapon structure
class weapon_t { public:
	int		ID;
	std::string	Name; // (was 64b before)
	int		Type;
	int		Special;
	int		Class;
	int		Recoil;
	float	Recharge;
	float	Drain;
	float	ROF;
	int		UseSound;
	std::string	SndFilename; // (was 64b before)
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

	SoundSample * smpSample;	// Read-only var, managed by game script, no need in smartpointer

};


class CGameScript {
public:
	// Constructor
	CGameScript() {
		loaded = false;
		NumWeapons = 0;
		Weapons = NULL;
        pModLog = NULL;

		RopeLength = 150;
		RestLength = 20;
		Strength = 0.5f;
	}

	~CGameScript() {
		Shutdown();
	}

private:
	// Attributes

	std::string		sDirectory;


	// Header
	bool loaded;
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
	
	std::vector< SmartPointer<SDL_Surface> > CachedImages;	// To safely delete the vars, along with CGameScript.
	std::vector< SmartPointer<SoundSample> > CachedSamples;	// To safely delete the vars, along with CGameScript.

private:

	void		Shutdown(void);
	void		ShutdownProjectile(proj_t *prj);

public:
	// Methods

	int			Load(const std::string& dir);
	int			Save(const std::string& filename);
	bool		isLoaded() const { return loaded; }
	
private:
	proj_t		*LoadProjectile(FILE *fp);
	int			SaveProjectile(proj_t *proj, FILE *fp);
    void        writeString(const std::string& szString, FILE *fp);
    std::string readString(FILE *fp);

public:
	size_t		GetMemorySize();
	std::string	getError(int code);

	const weapon_t	*FindWeapon(const std::string& name);
    bool        weaponExists(const std::string& szName);

	static bool	CheckFile(const std::string& dir, std::string& name, bool abs_filename = false);

    void        modLog(const std::string& text);

#ifndef _CONSOLE
	SDL_Surface * LoadGSImage(const std::string& dir, const std::string& filename);
	SoundSample * LoadGSSample(const std::string& dir, const std::string& filename);
#endif

	gs_header_t	*GetWriteableHeader()				{ return &Header; }
	const gs_header_t	*GetHeader()				{ return &Header; }
	static bool	isCompatibleWith(int scriptVer, const Version& ver) {
		if(scriptVer < GS_FIRST_SUPPORTED_VERSION) return false;
		if(scriptVer > GS_VERSION) return false; // or actually no idea
		return GS_MINLXVERSION(scriptVer) <= ver;		
	}
	bool		isCompatibleWith(const Version& ver) const { return isCompatibleWith(Header.Version, ver); }
	std::string modName() const { return Header.ModName; }
	std::string directory() const { return sDirectory; }
	
	int			GetNumWeapons()				{ return NumWeapons; }
	const weapon_t	*GetWeapons()				{ return Weapons; }
	weapon_t	*GetWriteableWeapons()				{ return Weapons; }

	void	initNewWeapons(int num)		{ if(Weapons) delete Weapons; SetNumWeapons(num); Weapons = new weapon_t[num]; }
	
private:
	void		SetNumWeapons(int _w)			{ NumWeapons = _w; }
	void		SetWeapons(weapon_t *_w)		{ Weapons = _w; }

public:
	
	// Ninja Rope settings
	void		SetRopeLength(int _l)			{ RopeLength = _l; }
	void		SetRestLength(int _l)			{ RestLength = _l; }
	void		SetStrength(float _s)			{ Strength = _s; }
	
	
	int			getRopeLength()				{ return RopeLength; }
	int			getRestLength()				{ return RestLength; }
	float		getStrength()				{ return Strength; }

	const gs_worm_t	*getWorm()					{ return &Worm; }
	gs_worm_t	*getWriteableWorm()					{ return &Worm; }


};


extern int ProjCount;
bool	initCompiler();
int		Compile(const char* dir);



#endif  //  __CGAMESCRIPT_H__
