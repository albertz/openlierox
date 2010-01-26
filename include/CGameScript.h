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

#include <map>
#include <set>
#include "game/Sounds.h"
#include "GfxPrimitives.h"
#include "CProjectile.h"
#include "Version.h"
#include "Color.h"
#include "StaticAssert.h"
#include "StringUtils.h"

class IniReader;

static const Version GS_MinLxVersion[] = {
	Version(), // for GS_VERSION == 7
	OLXBetaVersion(0,58,1), // for GS_VERSION == 8
};

#define		GS_LX56_VERSION	7
#define		GS_FIRST_SUPPORTED_VERSION	GS_LX56_VERSION
#define		GS_MINLXVERSION(ver)	GS_MinLxVersion[ver - GS_FIRST_SUPPORTED_VERSION]
// current most recent version
#define		GS_VERSION		8

static_assert(GS_VERSION - GS_FIRST_SUPPORTED_VERSION + 1 == sizeof(GS_MinLxVersion)/sizeof(Version), GS_MinLxVersion__sizecheck);

// Error codes
#define		GSE_OK			1
// TODO: what is this for?
#define		GSE_MEM			0
#define		GSE_VERSION		-1
#define		GSE_FILE		-2
#define		GSE_BAD			-3



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


struct weapon_t;
struct proj_t;
struct Proj_SpawnInfo;
struct Proj_Action;
struct ModInfo;


class CGameScript {
	friend struct Proj_SpawnInfo;
	friend struct Proj_Action;
	friend struct Proj_ProjHitEvent;
public:
	// Constructor
	CGameScript() {
		loaded = false;
		m_gusEngineUsed = false;
		needCollisionInfo = false;
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
	std::string modname;
	bool m_gusEngineUsed;

	// Weapons
	int			NumWeapons;
	weapon_t	*Weapons;

	// Worm
	gs_worm_t	Worm;

	typedef std::map<int, proj_t*> Projectiles;
	typedef std::map<std::string, int, stringcaseless> ProjFileMap;
	ProjFileMap projFileIndexes; // only for compiling
	std::set<proj_t*> savedProjs; // only for saving
	
	Projectiles projectiles;
	bool		needCollisionInfo;
	
	// Ninja Rope
	int			RopeLength;
	int			RestLength;
	float		Strength;

    // Mod log file
    FILE        *pModLog;
	
	std::vector< SmartPointer<SDL_Surface> > CachedImages;	// To safely delete the vars, along with CGameScript.
	std::vector< SmartPointer<SoundSample> > CachedSamples;	// To safely delete the vars, along with CGameScript.

private:

	void		Shutdown();
	void		ShutdownProjectile(proj_t *prj);
	static void	InitDefaultCompilerKeywords();

public:
	// Methods

	int			Load(const std::string& dir, bool loadImagesAndSounds = true);
	int			Save(const std::string& filename);
	bool		isLoaded() const { return loaded; }
	
private:
	proj_t		*LoadProjectile(FILE *fp, bool loadImagesAndSounds = true);
	bool		SaveProjectile(proj_t *proj, FILE *fp);

public:
	size_t		GetMemorySize();
	std::string	getError(int code);

	const weapon_t	*FindWeapon(const std::string& name);
    bool        weaponExists(const std::string& szName);

	static bool	CheckFile(const std::string& dir, std::string& name, bool abs_filename = false, ModInfo* info = NULL);

    void        modLog(const std::string& text);

	SDL_Surface * LoadGSImage(const std::string& dir, const std::string& filename);
	SoundSample * LoadGSSample(const std::string& dir, const std::string& filename);

	const gs_header_t	*GetHeader()				{ return &Header; }
	static bool	isCompatibleWith(int scriptVer, const Version& ver) {
		if(scriptVer < GS_FIRST_SUPPORTED_VERSION) return false;
		if(scriptVer > GS_VERSION) return false; // or actually no idea
		return GS_MINLXVERSION(scriptVer) <= ver;		
	}
	bool		isCompatibleWith(const Version& ver) const { return isCompatibleWith(Header.Version, ver); }
	std::string modName() const { return modname; }
	std::string directory() const { return sDirectory; }
	
	int			GetNumWeapons()				{ return NumWeapons; }
	const weapon_t	*GetWeapons()				{ return Weapons; }

private:
	void	initNewWeapons(int num);
	void	SetNumWeapons(int _w)			{ NumWeapons = _w; }
	void	SetWeapons(weapon_t *_w)		{ Weapons = _w; }

	
	// Ninja Rope settings
	void	SetRopeLength(int _l)			{ RopeLength = _l; }
	void	SetRestLength(int _l)			{ RestLength = _l; }
	void	SetStrength(float _s)			{ Strength = _s; }

public:
	
	
	int		getRopeLength()				{ return RopeLength; }
	int		getRestLength()				{ return RestLength; }
	float	getStrength()				{ return Strength; }

	bool	getNeedCollisionInfo()		{ return needCollisionInfo; }
	
	const gs_worm_t	*getWorm()	const	{ return &Worm; }

	size_t	getProjectileCount() const	{ return projectiles.size(); }
	
	bool	Compile(const std::string& dir);

	bool	gusEngineUsed() const		{ return m_gusEngineUsed; }
	
	static std::vector<std::string> LoadWeaponList(const std::string dir); // Does not load images & GFX, should be way faster than Load()
	std::vector<std::string> GetWeaponList() const;
	
private:
	bool	CompileWeapon(const std::string& dir, const std::string& weapon, int id);
	void	CompileBeam(const IniReader& ini, weapon_t *Weap);
	proj_t  *CompileProjectile(const std::string& dir, const std::string& pfile);
	bool	CompileExtra(const IniReader& ini);
	bool	CompileJetpack(const IniReader& ini, weapon_t *Weap);
};


#endif  //  __CGAMESCRIPT_H__
