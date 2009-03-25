/////////////////////////////////////////
//
//         LieroX Game Script Compiler
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Main compiler
// Created 7/2/02
// Jason Boettcher


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "CVec.h"
#include "CGameScript.h"
#include "ConfigHandler.h"
#include "SmartPointer.h"
#include "CrashHandler.h"


CGameScript	*Game;


// Prototypes
int		CheckArgs(int argc, char *argv[]);

int		Compile(const char* dir);
int		CompileWeapon(const char* dir, const std::string& weapon, int id);
void	CompileBeam(const char* file, weapon_t *Weap);
proj_t  *CompileProjectile(const char* dir, const char* pfile);
int		CompileExtra(const char* dir);

int		CompileJetpack(const char* file, weapon_t *Weap);

int ProjCount = 0;


///////////////////
// Main entry point
int main(int argc, char *argv[])
{
	printf("Liero Xtreme Game Script Compiler v0.3\nCopyright Auxiliary Software 2002\n");
	printf("GameScript Version: %d\n\n\n",GS_VERSION);


	Game = new CGameScript;
	if(Game == NULL) {
		printf("Error: Out of memory\n");
		return 1;
	}

	// Add some keywords
	AddKeyword("WPN_PROJECTILE",WPN_PROJECTILE);
	AddKeyword("WPN_SPECIAL",WPN_SPECIAL);
	AddKeyword("WPN_BEAM",WPN_BEAM);
	AddKeyword("WCL_AUTOMATIC",WCL_AUTOMATIC);
	AddKeyword("WCL_POWERGUN",WCL_POWERGUN);
	AddKeyword("WCL_GRENADE",WCL_GRENADE);
	AddKeyword("WCL_CLOSERANGE",WCL_CLOSERANGE);
	AddKeyword("PRJ_PIXEL",PRJ_PIXEL);
	AddKeyword("PRJ_IMAGE",PRJ_IMAGE);
	AddKeyword("Bounce",PJ_BOUNCE);
	AddKeyword("Explode",PJ_EXPLODE);
	AddKeyword("Injure",PJ_INJURE);
	AddKeyword("Carve",PJ_CARVE);
	AddKeyword("Dirt",PJ_DIRT);
    AddKeyword("GreenDirt",PJ_GREENDIRT);
    AddKeyword("Disappear",PJ_DISAPPEAR);
	AddKeyword("Nothing",PJ_NOTHING);
	AddKeyword("TRL_NONE",TRL_NONE);
	AddKeyword("TRL_SMOKE",TRL_SMOKE);
	AddKeyword("TRL_CHEMSMOKE",TRL_CHEMSMOKE);
	AddKeyword("TRL_PROJECTILE",TRL_PROJECTILE);
	AddKeyword("TRL_DOOMSDAY",TRL_DOOMSDAY);
	AddKeyword("TRL_EXPLOSIVE",TRL_EXPLOSIVE);
	AddKeyword("SPC_JETPACK",SPC_JETPACK);
	AddKeyword("ANI_ONCE",ANI_ONCE);
	AddKeyword("ANI_LOOP",ANI_LOOP);
	AddKeyword("ANI_PINGPONG",ANI_PINGPONG);
	AddKeyword("true",true);
	AddKeyword("false",false);


	if( !CheckArgs(argc, argv) ) {
		delete Game;
		return 1;
	}

	// Compile
	int comp = Compile(argv[1]);

	// Only save if the compile went ok
	if(comp) {
		printf("\nSaving...\n");
		Game->Save(argv[2]);
	}

	if(comp)
		printf("\nInfo:\nWeapons: %d\nProjectiles: %d\n",Game->GetNumWeapons(),ProjCount);

	if(Game) {
		delete Game;
		Game = NULL;
	}
	
	return 0;
}


///////////////////
// Check the arguments
int CheckArgs(int argc, char *argv[])
{
	char *d = strrchr(argv[0],'\\');
	if(!d)
		d = argv[0];
	else
		d++;

	if(argc != 3) {
		printf("Usage:\n");
		printf("%s [Mod dir] [filename]\n",d);
		printf("\nExample:\n");
		printf("%s Base script.lgs\n\n",d);
		return false;
	}

	return true;
}


///////////////////
// Compile
int Compile(const char* dir)
{
	char buf[64],wpn[64];
	int num,n;
	sprintf(buf,"%s/Main.txt",dir);


	// Check the file
	FILE *fp = fopen(buf, "rt");
	if(!fp) {
		printf("Error: Could not open the file '%s' for reading\n",buf);
		return false;
	} else
		fclose(fp);


	std::string modname;
	ReadString(buf,"General","ModName", modname,"untitled");
	strcpy(Game->GetWriteableHeader()->ModName, modname.c_str());
	
	printf("Compiling '%s'\n",Game->GetHeader()->ModName);

	ReadInteger(buf,"Weapons","NumWeapons",&num,0);


	// Weapons
	Game->initNewWeapons(num);


	// Compile the weapons
	for(n=0;n<Game->GetNumWeapons();n++) {
		sprintf(wpn,"Weapon%d",n+1);

		std::string weap;
		ReadString(buf,"Weapons",wpn,weap,"");

		if(!CompileWeapon(dir,weap,n))
			return false;
	}

	// Compile the extra stuff
	CompileExtra(dir);

	return true;
}


///////////////////
// Compile a weapon
int CompileWeapon(const char* dir, const std::string& weapon, int id)
{
	weapon_t *Weap = Game->GetWriteableWeapons()+id;
	char file[64];

	sprintf(file,"%s/%s",dir,weapon.c_str());

	Weap->UseSound = false;
	Weap->Special = SPC_NONE;
	Weap->Type = WPN_PROJECTILE;
	Weap->Projectile = NULL;
	Weap->LaserSight = false;

	ReadString(file,"General","Name",Weap->Name,"");
	printf("  Compiling Weapon '%s'\n",Weap->Name.c_str());

	ReadKeyword(file,"General","Type",&Weap->Type,WPN_PROJECTILE);


	// Special Weapons
	if(Weap->Type == WPN_SPECIAL) {
		
		ReadKeyword(file,"General","Special",&Weap->Special,SPC_NONE);

		// If it is a special weapon, read the values depending on the special weapon
		// We don't bother with the 'normal' values
		switch(Weap->Special) {
			// Jetpack
			case SPC_JETPACK:
				CompileJetpack(file, Weap);
				break;

			default:
				printf("   Error: Unknown special type\n");
		}
		return true;
	}


	// Beam Weapons
	if(Weap->Type == WPN_BEAM) {

		CompileBeam(file,Weap);
		return true;
	}


	// Projectile Weapons
	ReadKeyword(file,"General","Class",&Weap->Class,WCL_AUTOMATIC);
	ReadInteger(file,"General","Recoil",&Weap->Recoil,0);
	ReadFloat(file,"General","Recharge",&Weap->Recharge,0);
	ReadFloat(file,"General","Drain",&Weap->Drain,0);
	ReadFloat(file,"General","ROF",&Weap->ROF,0);
	ReadKeyword(file, "General", "LaserSight", &Weap->LaserSight, false);
	if(ReadString(file,"General","Sound",Weap->SndFilename,""))
		Weap->UseSound = true;
	
	
	ReadInteger(file,"Projectile","Speed",&Weap->ProjSpeed,0);
	ReadFloat(file,"Projectile","SpeedVar",&Weap->ProjSpeedVar,0);
	ReadFloat(file,"Projectile","Spread",&Weap->ProjSpread,0);
	ReadInteger(file,"Projectile","Amount",&Weap->ProjAmount,0);


	// Load the projectile
	std::string pfile;
	ReadString(file,"Projectile","Projectile",pfile,"");

	Weap->Projectile = CompileProjectile(dir,pfile.c_str());
	if(Weap->Projectile == NULL)
		return false;

	return true;
}


///////////////////
// Compile a beam weapon
void CompileBeam(const char* file, weapon_t *Weap)
{
	ReadInteger(file,"General","Recoil",&Weap->Recoil,0);
	ReadFloat(file,"General","Recharge",&Weap->Recharge,0);
	ReadFloat(file,"General","Drain",&Weap->Drain,0);
	ReadFloat(file,"General","ROF",&Weap->ROF,0);
	if(ReadString(file,"General","Sound",Weap->SndFilename,""))
		Weap->UseSound = true;

	ReadInteger(file, "Beam", "Damage", &Weap->Bm_Damage, 0);
	ReadInteger(file, "Beam", "Length", &Weap->Bm_Length, 0);
	ReadInteger(file, "Beam", "PlayerDamage", &Weap->Bm_PlyDamage, 0);

	std::string string;
	char *tok;
	ReadString(file,"Beam","Colour",string,"0,0,0");
	char tmp[64];
	strcpy(tmp, string.c_str());
	tok = strtok(tmp,",");	Weap->Bm_Colour[0] = atoi(tok);
	tok = strtok(NULL,",");		Weap->Bm_Colour[1] = atoi(tok);
	tok = strtok(NULL,",");		Weap->Bm_Colour[2] = atoi(tok);
}


///////////////////
// Compile a projectile
proj_t *CompileProjectile(const char* dir, const char* pfile)
{
	proj_t *proj = new proj_t;
	if(proj == NULL)
		return NULL;

	ProjCount++;

	
	// Load the projectile
	char file[128];
	sprintf(file,"%s/%s",dir,pfile);

	printf("    Compiling Projectile '%s'\n",pfile);
	
	proj->filename = pfile;
	
	proj->Timer_Projectiles = false;
	proj->Hit_Projectiles = false;
	proj->PlyHit_Projectiles = false;
    proj->Exp_Projectiles = false;
    proj->Tch_Projectiles = false;
	proj->Projectile = NULL;
	proj->PrjTrl_Proj = NULL;
	proj->Animating = false;
	proj->UseCustomGravity = false;

	ReadKeyword(file,"General","Type",&proj->Type,PRJ_PIXEL);
	ReadFloat(file,"General","Timer",&proj->Timer_Time,0);
	ReadFloat(file, "General", "TimerVar", &proj->Timer_TimeVar, 0);
	ReadKeyword(file,"General","Trail",&proj->Trail,TRL_NONE);
	
	if( ReadInteger(file,"General","Gravity",&proj->Gravity, 0) )
		proj->UseCustomGravity = true;

	ReadFloat(file,"General","Dampening",&proj->Dampening,1.0f);

	if(proj->Type == PRJ_PIXEL) {
		std::string tmp;
		char string[64];
		char *tok;
		proj->NumColours = 1;

		if( ReadString(file,"General","Colour1",tmp,"0,0,0") ) {
			strcpy(string, tmp.c_str());
			tok = strtok(string,",");	proj->Colour1[0] = atoi(tok);
			tok = strtok(NULL,",");		proj->Colour1[1] = atoi(tok);
			tok = strtok(NULL,",");		proj->Colour1[2] = atoi(tok);
		}

		if( ReadString(file,"General","Colour2",tmp,"0,0,0") ) {
			strcpy(string, tmp.c_str());
			tok = strtok(string,",");	proj->Colour2[0] = atoi(tok);
			tok = strtok(NULL,",");		proj->Colour2[1] = atoi(tok);
			tok = strtok(NULL,",");		proj->Colour2[2] = atoi(tok);
			proj->NumColours = 2;
		}

	} else if(proj->Type == PRJ_IMAGE) {
		ReadString(file,"General","Image",proj->ImgFilename,"");
		ReadKeyword(file,"General","Rotating",&proj->Rotating,false);
		ReadInteger(file,"General","RotIncrement",&proj->RotIncrement,0);
		ReadInteger(file,"General","RotSpeed",&proj->RotSpeed,0);
		ReadKeyword(file,"General","UseAngle",&proj->UseAngle,0);
		ReadKeyword(file,"General","UseSpecAngle",&proj->UseSpecAngle,0);
		if(proj->UseAngle || proj->UseSpecAngle)
			ReadInteger(file,"General","AngleImages",&proj->AngleImages,0);

		ReadKeyword(file,"General","Animating",&proj->Animating,0);
		if(proj->Animating) {
			ReadFloat(file,"General","AnimRate",&proj->AnimRate,0);
			ReadKeyword(file,"General","AnimType",&proj->AnimType,ANI_ONCE);
		}
	}
	


	// Hit
	ReadKeyword(file,"Hit","Type",&proj->Hit_Type,PJ_EXPLODE);

	// Hit::Explode
	if(proj->Hit_Type == PJ_EXPLODE) {
		ReadInteger(file,"Hit","Damage",&proj->Hit_Damage,0);
		ReadKeyword(file,"Hit","Projectiles",&proj->Hit_Projectiles,false);
		ReadInteger(file,"Hit","Shake",&proj->Hit_Shake,0);

		proj->Hit_UseSound = false;
		if(ReadString(file,"Hit","Sound",proj->Hit_SndFilename,""))
			proj->Hit_UseSound = true;
	}

	// Hit::Carve
	if(proj->Hit_Type == PJ_CARVE) {
		ReadInteger(file,"Hit","Damage",&proj->Hit_Damage,0);
	}

	// Hit::Bounce
	if(proj->Hit_Type == PJ_BOUNCE) {
		ReadFloat(file,"Hit","BounceCoeff",&proj->Hit_BounceCoeff,0.5);
		ReadInteger(file,"Hit","BounceExplode",&proj->Hit_BounceExplode,0);
	}

	// Timer
	if(proj->Timer_Time > 0) {
		ReadKeyword(file,"Time","Type",&proj->Timer_Type,PJ_EXPLODE);
		if(proj->Timer_Type == PJ_EXPLODE) {
			ReadInteger(file,"Time","Damage",&proj->Timer_Damage,0);
			ReadKeyword(file,"Time","Projectiles",&proj->Timer_Projectiles,false);
			ReadInteger(file,"Time","Shake",&proj->Timer_Shake,0);
		}
	}

	// Player hit
	ReadKeyword(file,"PlayerHit","Type",&proj->PlyHit_Type,PJ_INJURE);

	// PlyHit::Explode || PlyHit::Injure
	if(proj->PlyHit_Type == PJ_EXPLODE || proj->PlyHit_Type == PJ_INJURE) {
		ReadInteger(file,"PlayerHit","Damage",&proj->PlyHit_Damage,0);
		ReadKeyword(file,"PlayerHit","Projectiles",&proj->PlyHit_Projectiles,false);
	}

	// PlyHit::Bounce
	if(proj->PlyHit_Type == PJ_BOUNCE) {
		ReadFloat(file,"PlayerHit","BounceCoeff",&proj->PlyHit_BounceCoeff,0.5);
	}


    // OnExplode
    ReadKeyword( file, "Explode", "Type",       &proj->Exp_Type, PJ_NOTHING );
    ReadInteger( file, "Explode", "Damage",     &proj->Exp_Damage, 0 );
    ReadKeyword( file, "Explode", "Projectiles",&proj->Exp_Projectiles, false );
    ReadInteger( file, "Explode", "Shake",      &proj->Exp_Shake, 0 );
    proj->Exp_UseSound = false;
	if( ReadString(file, "Explode", "Sound", proj->Exp_SndFilename,"") )
		proj->Exp_UseSound = true;


    // Touch
    ReadKeyword( file, "Touch", "Type",       &proj->Tch_Type, PJ_NOTHING );
    ReadInteger( file, "Touch", "Damage",     &proj->Tch_Damage, 0 );
    ReadKeyword( file, "Touch", "Projectiles",&proj->Tch_Projectiles, false );
    ReadInteger( file, "Touch", "Shake",      &proj->Tch_Shake, 0 );
    proj->Tch_UseSound = false;
	if( ReadString(file, "Touch", "Sound", proj->Tch_SndFilename,"") )
		proj->Tch_UseSound = true;


	// Projectiles
	if(proj->Timer_Projectiles || proj->Hit_Projectiles || proj->PlyHit_Projectiles || proj->Exp_Projectiles ||
       proj->Tch_Projectiles) {
		ReadKeyword(file,"Projectile","Useangle",&proj->ProjUseangle,0);
		ReadInteger(file,"Projectile","Angle",&proj->ProjAngle,0);
		ReadInteger(file,"Projectile","Speed",&proj->ProjSpeed,0);
		ReadFloat(file,"Projectile","SpeedVar",&proj->ProjSpeedVar,0);
		ReadFloat(file,"Projectile","Spread",&proj->ProjSpread,0);
		ReadInteger(file,"Projectile","Amount",&proj->ProjAmount,0);

		// Load the projectile
		std::string prjfile;
		ReadString(file,"Projectile","Projectile",prjfile,"");

		proj->Projectile = CompileProjectile(dir,prjfile.c_str());
	}


	// Projectile trail
	if(proj->Trail == TRL_PROJECTILE) {
		ReadKeyword(file, "ProjectileTrail", "UseProjVelocity", &proj->PrjTrl_UsePrjVelocity, false);
		ReadFloat  (file, "ProjectileTrail", "Delay",  &proj->PrjTrl_Delay, 100);
		ReadInteger(file, "ProjectileTrail", "Amount", &proj->PrjTrl_Amount, 1);
		ReadInteger(file, "ProjectileTrail", "Speed",  &proj->PrjTrl_Speed, 100);
		ReadFloat(file, "ProjectileTrail", "SpeedVar",  &proj->PrjTrl_SpeedVar, 0);
		ReadFloat(file, "ProjectileTrail", "Spread", &proj->PrjTrl_Spread, 0);
		
		// Load the projectile
		std::string prjfile;
		ReadString(file, "ProjectileTrail", "Projectile", prjfile, "");

		proj->PrjTrl_Proj = CompileProjectile(dir,prjfile.c_str());
	}
	

	return proj;
}


///////////////////
// Compile the extra stuff
int CompileExtra(const char* dir)
{
	char file[64];

	printf("Compiling Extras\n");

	sprintf(file,"%s/main.txt",dir);


	// Ninja Rope
	printf("  Compiling Ninja Rope\n");

	int ropel, restl;
	float strength;

	ReadInteger(file,"NinjaRope","RopeLength",&ropel,0);
	ReadInteger(file,"NinjaRope","RestLength",&restl,0);
	ReadFloat(file,"NinjaRope","Strength",&strength,0);

	Game->SetRopeLength(ropel);
	Game->SetRestLength(restl);
	Game->SetStrength(strength);


	// Worm
	printf("  Compiling Worm\n");
	gs_worm_t *wrm = Game->getWriteableWorm();

	ReadFloat( file, "Worm", "AngleSpeed",		&wrm->AngleSpeed, 150);
	ReadFloat( file, "Worm", "GroundSpeed",		&wrm->GroundSpeed, 8);
	ReadFloat( file, "Worm", "AirSpeed",		&wrm->AirSpeed, 1);
	ReadFloat( file, "Worm", "Gravity",			&wrm->Gravity, 175);
	ReadFloat( file, "Worm", "JumpForce",		&wrm->JumpForce, -140);
	ReadFloat( file, "Worm", "AirFriction",		&wrm->AirFriction, 0);
	ReadFloat( file, "Worm", "GroundFriction",	&wrm->GroundFriction, 0.6f);





	return true;
}


/*
===============================

		Special items

===============================
*/


///////////////////
// Compile the jetpack
int CompileJetpack(const char* file, weapon_t *Weap)
{
	Weap->Projectile = NULL;

	ReadInteger(file, "JetPack", "Thrust", (int*)&Weap->tSpecial.Thrust, 0);
	ReadFloat(file, "JetPack", "Drain", &Weap->Drain, 0);
	ReadFloat(file, "JetPack", "Recharge", &Weap->Recharge, 0);	

	Weap->ROF = 1;

	return true;
}







// some dummies/stubs are following to be able to compile with OLX sources

FILE* OpenGameFile(const std::string& file, const char* mod) {
	// stub
	return fopen(file.c_str(), mod);
}

bool GetExactFileName(const std::string& fn, std::string& exactfn) {
	// sub
	exactfn = fn;
	return true;
}

struct SoundSample;
template <> void SmartPointer_ObjectDeinit<SoundSample> ( SoundSample * obj )
{
	errors << "SmartPointer_ObjectDeinit SoundSample: stub" << endl;
}

template <> void SmartPointer_ObjectDeinit<SDL_Surface> ( SDL_Surface * obj )
{
	errors << "SmartPointer_ObjectDeinit SDL_Surface: stub" << endl;
}

SmartPointer<SoundSample> LoadSample(const std::string& _filename, int maxplaying) {
	// stub
	return NULL;
}

SmartPointer<SDL_Surface> LoadGameImage(const std::string& _filename, bool withalpha) {
	// stub
	return NULL;
}

void SetColorKey(SDL_Surface * dst) {} // stub

bool bDedicated = true;

void SetError(const std::string& text) { errors << "SetError: " << text << endl; }

struct GameOptions;
GameOptions *tLXOptions = NULL;

bool Con_IsInited() { return false; }

CrashHandler* CrashHandler::get() {	return NULL; }

void Con_AddText(int colour, const std::string& text, bool alsoToLogger) {}

SDL_PixelFormat defaultFallbackFormat =
{
 NULL, //SDL_Palette *palette;
 32, //Uint8  BitsPerPixel;
 4, //Uint8  BytesPerPixel;
 0, 0, 0, 0, //Uint8  Rloss, Gloss, Bloss, Aloss;
 24, 16, 8, 0, //Uint8  Rshift, Gshift, Bshift, Ashift;
 0xff000000, 0xff0000, 0xff00, 0xff, //Uint32 Rmask, Gmask, Bmask, Amask;
 0, //Uint32 colorkey;
 255 //Uint8  alpha;
};

SDL_PixelFormat* mainPixelFormat = &defaultFallbackFormat;
