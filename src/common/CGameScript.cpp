/////////////////////////////////////////
//
//                  OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Game script class
// Created 7/2/02
// Jason Boettcher

/*
 reference for original Compile* sources:
 http://openlierox.svn.sourceforge.net/viewvc/openlierox/src/common/CGameScript.cpp?revision=3781&view=markup
 */

#include <cstdarg>

#include "EndianSwap.h"
#include "LieroX.h"
#include "Debug.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CGameScript.h"
#include "Error.h"
#include "ConfigHandler.h"
#include "ProjectileDesc.h"
#include "WeaponDesc.h"



void CGameScript::initNewWeapons(int num) {
	if(Weapons) delete[] Weapons;
	SetNumWeapons(num);
	Weapons = new weapon_t[num];
}



///////////////////
// Write a string in pascal format
static void writeString(const std::string& szString, FILE *fp)
{
    if(szString == "") return;
	
	size_t length = szString.size();
	if(length > 255) {
		warnings << "i will cut the following string for writing: " << szString << endl;
		length = 255;
	}
	uchar len = (uchar)length;
	
    fwrite( &len, sizeof(uchar), 1, fp );
    fwrite( szString.c_str(),sizeof(char), length, fp );
}


///////////////////
// Read a string in pascal format
static std::string readString(FILE *fp)
{
	char buf[256];
	
    uchar length;
    fread( &length, sizeof(uchar), 1, fp );
    fread( buf,sizeof(char), length, fp );
	
    buf[length] = '\0';
	
    return buf;
}



///////////////////
// Save the script (compiler)
int CGameScript::Save(const std::string& filename)
{
	FILE *fp;
	int n;


	// Open it
	fp = OpenGameFile(filename,"wb");
	if(fp == NULL) {
		errors << "CGameScript::Save: Could not open " << filename << " for writing" << endl;
		return false;
	}

	Header.Version = GS_VERSION;
	strcpy(Header.ID,"Liero Game Script");


	// Header
	gs_header_t tmpheader = Header;
	EndianSwap(tmpheader.Version);
	fwrite(&tmpheader,sizeof(gs_header_t),1,fp);

	fwrite_endian_compat((NumWeapons),sizeof(int),1,fp);

	savedProjs.clear();
	
	// Weapons
	weapon_t *wpn = Weapons;

	for(n=0;n<NumWeapons;n++,wpn++) {

        writeString(wpn->Name, fp);
		fwrite_endian_compat((wpn->Type),          sizeof(int),    1, fp);

		// Special
		if(wpn->Type == WPN_SPECIAL) {
			fwrite_endian_compat((wpn->Special),   sizeof(int),    1, fp);
			fwrite_endian_compat((wpn->tSpecial),  sizeof(gs_special_t), 1, fp); // WARNING: this works, because it contains only one int
			fwrite_endian_compat((wpn->Recharge*10.f),  sizeof(float),  1, fp);
			fwrite_endian_compat((wpn->Drain),     sizeof(float),  1, fp);
			fwrite_endian_compat((wpn->ROF*1000.0f),       sizeof(float),  1, fp);
			fwrite_endian<int>(fp, wpn->LaserSight);
		}

		// Beam
		if(wpn->Type == WPN_BEAM) {
			fwrite_endian_compat((wpn->Recoil),    sizeof(int),    1, fp);
			fwrite_endian_compat((wpn->Recharge*10.f),  sizeof(float),  1, fp);
			fwrite_endian_compat((wpn->Drain),     sizeof(float),  1, fp);
			fwrite_endian_compat((wpn->ROF*1000.0f),       sizeof(float),  1, fp);
			fwrite_endian<int>(fp, wpn->LaserSight);

			wpn->Bm.write(this, fp);
		}

		// Projectile
		if(wpn->Type == WPN_PROJECTILE) {
			fwrite_endian_compat((wpn->Class),     sizeof(int),    1, fp);
			fwrite_endian_compat((wpn->Recoil),    sizeof(int),    1, fp);
			fwrite_endian_compat((wpn->Recharge*10.f),  sizeof(float),  1, fp);
			fwrite_endian_compat((wpn->Drain),     sizeof(float),  1, fp);
			fwrite_endian_compat((wpn->ROF*1000.0f),       sizeof(float),  1, fp);
			fwrite_endian_compat((wpn->Proj.Speed), sizeof(int),    1, fp);
			fwrite_endian_compat((wpn->Proj.SpeedVar),sizeof(float),1, fp);
			fwrite_endian_compat((wpn->Proj.Spread),sizeof(float),  1, fp);
			fwrite_endian_compat((wpn->Proj.Amount),sizeof(int),    1, fp);
			fwrite_endian<int>(fp, wpn->LaserSight);

			fwrite_endian<int>(fp, wpn->UseSound);
			if(wpn->UseSound)
                writeString(wpn->SndFilename, fp);

			// Recursively save the projectile id's
			SaveProjectile(wpn->Proj.Proj,fp);
		}
	}

	// Extra stuff


	// Ninja Rope
	fwrite_endian_compat((RopeLength),sizeof(int),1,fp);
	fwrite_endian_compat((RestLength),sizeof(int),1,fp);
	fwrite_endian_compat((Strength),sizeof(float),1,fp);

	// Worm
	gs_worm_t tmpworm = Worm;
	EndianSwap(tmpworm.AngleSpeed);
	EndianSwap(tmpworm.GroundSpeed);
	EndianSwap(tmpworm.AirSpeed);
	EndianSwap(tmpworm.Gravity);
	EndianSwap(tmpworm.JumpForce);
	EndianSwap(tmpworm.AirFriction);
	EndianSwap(tmpworm.GroundFriction);
	fwrite(&tmpworm, sizeof(gs_worm_t), 1, fp);


	fclose(fp);

	savedProjs.clear();
	
	return true;
}


///////////////////
// Save a projectile
bool CGameScript::SaveProjectile(proj_t *proj, FILE *fp)
{
	if(!proj)
		return false;

	if(Header.Version > GS_LX56_VERSION) {
		// well, that's slow but it's not worth to create a reverted-map just for saving
		int index = -1;
		for(Projectiles::iterator it = projectiles.begin(); it != projectiles.end(); ++it) {
			if(it->second == proj) {
				index = it->first;
				break;
			}
		}
		fwrite_endian<int>(fp, index);
		if(savedProjs.find(proj) != savedProjs.end()) return false;
		savedProjs.insert(proj);
	}
	
	fwrite_endian_compat((proj->Type),			sizeof(int),1,fp);
	fwrite_endian_compat((proj->Timer.Time),	sizeof(float),1,fp);
	fwrite_endian_compat((proj->Timer.TimeVar),sizeof(float),1,fp);
	fwrite_endian_compat((proj->Trail.Type),		sizeof(int),1,fp);
	//fwrite_endian_compat(proj->UseCustomGravity, sizeof(int), 1, fp); // use this to test static assert (should fail if sizeof(bool)!=sizeof(int))
	fwrite_endian<int>(fp, proj->UseCustomGravity);

	if(proj->UseCustomGravity)
		fwrite_endian_compat((proj->Gravity),	sizeof(int), 1, fp);

	fwrite_endian_compat((proj->Dampening),	sizeof(int), 1, fp);

	if(Header.Version > GS_LX56_VERSION) {
		fwrite_endian<int>(fp, proj->Width);
		fwrite_endian<int>(fp, proj->Height);
	}
	
    // Pixel type
	switch(proj->Type) {
		case PRJ_POLYGON:
			fwrite_endian<Uint32>(fp, proj->polygon.points.size());
			for(Polygon2D::Points::iterator p = proj->polygon.points.begin(); p != proj->polygon.points.end(); ++p) {
				fwrite_endian<int>(fp, p->x);
				fwrite_endian<int>(fp, p->y);				
			}
			
			// fallthrough to save color
		case PRJ_RECT:
		case PRJ_CIRCLE:
		case PRJ_PIXEL:
			fwrite_endian_compat(((int)proj->Colour.size()), sizeof(int), 1, fp);
			for(size_t i = 0; i < proj->Colour.size(); ++i) {
				if(Header.Version <= GS_LX56_VERSION && i >= 2) {
					warnings << "SaveProjectile: projectile has more than 2 colors, other colors are ignored for LX56 format" << endl;
					break;
				}
				if(Header.Version <= GS_LX56_VERSION) {
					fwrite_endian_compat(((int)proj->Colour[i].r),   sizeof(int),1,fp);
					fwrite_endian_compat(((int)proj->Colour[i].g),   sizeof(int),1,fp);
					fwrite_endian_compat(((int)proj->Colour[i].b),   sizeof(int),1,fp);
				} else {
					fwrite_endian_compat((proj->Colour[i].r), 1,1,fp);
					fwrite_endian_compat((proj->Colour[i].g), 1,1,fp);
					fwrite_endian_compat((proj->Colour[i].b), 1,1,fp);
					fwrite_endian_compat((proj->Colour[i].a), 1,1,fp);	
				}
			}
			break;
			
		case PRJ_IMAGE:
			writeString(proj->ImgFilename, fp);
			fwrite_endian<int>(fp, proj->Rotating);
			fwrite_endian_compat((proj->RotIncrement), sizeof(int), 1, fp);
			fwrite_endian_compat((proj->RotSpeed), sizeof(int),1,fp);
			fwrite_endian<int>(fp, proj->UseAngle);
			fwrite_endian<int>(fp, proj->UseSpecAngle);
			if(proj->UseAngle || proj->UseSpecAngle)
				fwrite_endian_compat((proj->AngleImages),sizeof(int),1,fp);
			fwrite_endian<int>(fp, proj->Animating);
			if(proj->Animating) {
				fwrite_endian_compat((proj->AnimRate),sizeof(float),1,fp);
				fwrite_endian_compat((proj->AnimType),sizeof(int),1,fp);
			}
			break;
					
		case __PRJ_LBOUND: case __PRJ_UBOUND: errors << "SaveProjectile: PRJ BOUND" << endl;
	}




	//
	// Hit
	//
	if(Header.Version <= GS_LX56_VERSION) {
		fwrite_endian_compat((proj->Hit.Type),sizeof(int),1,fp);

		// Hit::Explode
		if(proj->Hit.Type == PJ_EXPLODE) {
			fwrite_endian_compat((proj->Hit.Damage),		sizeof(int),1,fp);
			fwrite_endian<int>(fp, proj->Hit.Projectiles);
			fwrite_endian<int>(fp, proj->Hit.UseSound);
			fwrite_endian_compat((proj->Hit.Shake),		sizeof(int),1,fp);
			if(proj->Hit.UseSound)
				writeString(proj->Hit.SndFilename, fp);
		}

		// Hit::Bounce
		if(proj->Hit.Type == PJ_BOUNCE) {
			fwrite_endian_compat((proj->Hit.BounceCoeff),	sizeof(float),	1,fp);
			fwrite_endian_compat((proj->Hit.BounceExplode),sizeof(int),	1,fp);
		}

		// Hit::Carve
		if(proj->Hit.Type == PJ_CARVE) {
			fwrite_endian_compat((proj->Hit.Damage),		sizeof(int),1,fp);
		}
	}
	else { // newer GS version
		proj->Hit.write(this, fp);
	}




	//
	// Timer
	//
	if(proj->Timer.Time > 0) {
		if(Header.Version <= GS_LX56_VERSION) {
			fwrite_endian_compat((proj->Timer.Type),sizeof(int),1,fp);
			if(proj->Timer.Type == PJ_EXPLODE) {
				fwrite_endian_compat((proj->Timer.Damage),		sizeof(int),1,fp);
				fwrite_endian<int>(fp, proj->Timer.Projectiles);
				fwrite_endian_compat((proj->Timer.Shake),sizeof(int),1,fp);
			}
		}
		else {
			proj->Timer.write(this, fp);
		}
	}


	//
	// Player hit
	//
	if(Header.Version <= GS_LX56_VERSION) {
		fwrite_endian_compat((proj->PlyHit.Type),sizeof(int),1,fp);

		// PlyHit::Explode || PlyHit::Injure
		if(proj->PlyHit.Type == PJ_EXPLODE || proj->PlyHit.Type == PJ_INJURE) {
			fwrite_endian_compat((proj->PlyHit.Damage),sizeof(int),1,fp);
			fwrite_endian<int>(fp, proj->PlyHit.Projectiles);
		}

		// PlyHit::Bounce
		if(proj->PlyHit.Type == PJ_BOUNCE) {
			fwrite_endian_compat((proj->PlyHit.BounceCoeff),sizeof(float),1,fp);
		}
	}
	else { // newer GS version
		proj->PlyHit.write(this, fp);
	}

	
	if(Header.Version <= GS_LX56_VERSION) {
		// NOTE: this is obsolete, they are not used
		
		//
		// Explode
		//
		fwrite_endian_compat((proj->Exp.Type),     sizeof(int), 1, fp);
		fwrite_endian_compat((proj->Exp.Damage),   sizeof(int), 1, fp);
		fwrite_endian<int>(fp, proj->Exp.Projectiles);
		fwrite_endian<int>(fp, proj->Exp.UseSound);
		if(proj->Exp.UseSound)
			writeString(proj->Exp.SndFilename, fp);


		//
		// Touch
		//
		fwrite_endian_compat((proj->Tch.Type),     sizeof(int), 1, fp);
		fwrite_endian_compat((proj->Tch.Damage),   sizeof(int), 1, fp);
		fwrite_endian<int>(fp, proj->Tch.Projectiles);
		fwrite_endian<int>(fp, proj->Tch.UseSound);
		if(proj->Tch.UseSound)
			writeString(proj->Tch.SndFilename, fp);
	}

	if(Header.Version > GS_LX56_VERSION) {
		fwrite_endian<Uint32>(fp, proj->actions.size());
		for(Uint32 i = 0; i < proj->actions.size(); ++i) {
			proj->actions[i].write(this, fp);
		}
	}
	
	if(Header.Version > GS_LX56_VERSION) {
		// always save, there are many cases where we could possibly need it
		proj->GeneralSpawnInfo.write(this, fp);
	}
	else if(proj->Timer.Projectiles || proj->Hit.Projectiles || proj->PlyHit.Projectiles || proj->Exp.Projectiles ||
       proj->Tch.Projectiles) {
		fwrite_endian<int>(fp, proj->GeneralSpawnInfo.Useangle);
		fwrite_endian_compat((proj->GeneralSpawnInfo.Angle),	sizeof(int),	1, fp);
		
		fwrite_endian_compat((proj->GeneralSpawnInfo.Amount),	sizeof(int),	1, fp);
		fwrite_endian_compat((proj->GeneralSpawnInfo.Spread),	sizeof(float),	1, fp);
		fwrite_endian_compat((proj->GeneralSpawnInfo.Speed),	sizeof(int),	1, fp);
		fwrite_endian_compat((proj->GeneralSpawnInfo.SpeedVar),	sizeof(float),	1, fp);

		SaveProjectile(proj->GeneralSpawnInfo.Proj,fp);
	}

	if(Header.Version > GS_LX56_VERSION) {
		if(proj->Timer.Projectiles) {
			fwrite_endian<char>(fp, proj->Timer.Proj.isSet());
			if(proj->Timer.Proj.isSet()) proj->Timer.Proj.write(this, fp);
		}
		if(proj->Hit.Projectiles) {
			fwrite_endian<char>(fp, proj->Hit.Proj.isSet());
			if(proj->Hit.Proj.isSet()) proj->Hit.Proj.write(this, fp);
		}
		if(proj->PlyHit.Projectiles) {
			fwrite_endian<char>(fp, proj->PlyHit.Proj.isSet());
			if(proj->PlyHit.Proj.isSet()) proj->PlyHit.Proj.write(this, fp);		
		}
	}

	// Projectile trail
	if(proj->Trail.Type == TRL_PROJECTILE) {

		fwrite_endian<int>(fp, proj->Trail.Proj.UseParentVelocityForSpread);
		fwrite_endian_compat((proj->Trail.Delay*1000.0f),				sizeof(float),	1, fp);
	
		if(Header.Version <= GS_LX56_VERSION) {
			fwrite_endian_compat((proj->Trail.Proj.Amount),			sizeof(int),	1, fp);
			fwrite_endian_compat((proj->Trail.Proj.Speed),				sizeof(int),	1, fp);
			fwrite_endian_compat((proj->Trail.Proj.SpeedVar),			sizeof(float),	1, fp);
			fwrite_endian_compat((proj->Trail.Proj.Spread),			sizeof(float),	1, fp);

			SaveProjectile(proj->Trail.Proj.Proj, fp);
		}
		else { // newer GS versions
			proj->Trail.Proj.write(this, fp);
		}
		
	}

	return true;
}


///////////////////
// Load the game script from a file (game)
int CGameScript::Load(const std::string& dir)
{
	// Already cached externally
	/*
	// Try cache first
	CGameScript *cached = cCache.GetMod(dir);
	if (cached != NULL)  {
		CopyFrom(cached);
		return GSE_OK;
	}
	*/
	Shutdown();

	int n;
	std::string filename = dir + "/script.lgs";
	sDirectory = dir;

	// Open it
	FILE* fp = OpenGameFile(filename,"rb");
	if(fp == NULL) {
		if(IsFileAvailable(dir + "/main.txt")) {
			hints << "GameScript: '" << dir << "': loading from gamescript source" << endl;
			if(Compile(dir))
				return GSE_OK;
			else {
				warnings << "GameScript::Load(): could not compile source gamescript '" << dir << "'" << endl;
				return GSE_BAD;
			}
		}
		
		warnings << "CGameScript::Load(): Could not load file " << filename << endl;
		return GSE_FILE;
	}

	// Header
	fread_compat(Header,sizeof(gs_header_t),1,fp);
	EndianSwap(Header.Version);
	// for security
	fix_markend(Header.ID);
	fix_markend(Header.ModName);

	// Check ID
	if(strcmp(Header.ID,"Liero Game Script") != 0) {
		fclose(fp);
		SetError("CGameScript::Load(): Bad script id");
		return GSE_BAD;
	}

	// Check version
	if(Header.Version < GS_FIRST_SUPPORTED_VERSION || Header.Version > GS_VERSION) {
		warnings << "GS:CheckFile: WARNING: " << dir << "/script.lgs has the wrong version";
		warnings << " (" << (unsigned)Header.Version << ", required is in the range ";
		warnings << "[" << GS_FIRST_SUPPORTED_VERSION << "," << GS_VERSION << "])" << endl;
		fclose(fp);
		SetError("CGameScript::Load(): Bad script version");
		return GSE_VERSION;
	}

    // Clear an old mod file
    modLog("Loading game mod file " + filename);
	//modLog("  ID = %s", Header.ID);
	//modLog("  Version = %i", Header.Version);

	fread_compat(NumWeapons,sizeof(int),1,fp);
	EndianSwap(NumWeapons);
	//modLog("  NumWeapons = %i", NumWeapons);

	// Do Allocations
	Weapons = new weapon_t[NumWeapons];
	if(Weapons == NULL) {
		SetError("SGameScript::Load(): Out of memory");
		return GSE_MEM;
	}

	// Weapons
	weapon_t *wpn;
	for(n=0;n<NumWeapons;n++) {
		wpn = &Weapons[n];

		wpn->ID = n;
		wpn->Proj.Proj = NULL;

        wpn->Name = readString(fp);
		fread_compat(wpn->Type,           sizeof(int),    1,fp);
		EndianSwap(wpn->Type);

		// Special
		if(wpn->Type == WPN_SPECIAL) {
			fread_compat(wpn->Special,    sizeof(int),    1, fp);
			EndianSwap(wpn->Special);
			fread_compat(wpn->tSpecial,   sizeof(gs_special_t), 1, fp);
			EndianSwap(wpn->tSpecial.Thrust);
			fread_compat(wpn->Recharge,   sizeof(float),  1, fp);
			EndianSwap(wpn->Recharge);
			fread_compat(wpn->Drain,      sizeof(float),  1, fp);
			EndianSwap(wpn->Drain);
			fread_compat(wpn->ROF,        sizeof(float),  1, fp);
			EndianSwap(wpn->ROF);
			fread_endian<int>(fp, wpn->LaserSight);
		}

		// Beam
		if(wpn->Type == WPN_BEAM) {
			fread_compat(wpn->Recoil,     sizeof(int),    1, fp);
			EndianSwap(wpn->Recoil);
			fread_compat(wpn->Recharge,   sizeof(float),  1, fp);
			EndianSwap(wpn->Recharge);
			fread_compat(wpn->Drain,      sizeof(float),  1, fp);
			EndianSwap(wpn->Drain);
			fread_compat(wpn->ROF,        sizeof(float),  1, fp);
			EndianSwap(wpn->ROF);
			fread_endian<int>(fp, wpn->LaserSight);

			wpn->Bm.read(this, fp);
		}

		// Projectile
		if(wpn->Type == WPN_PROJECTILE) {

			fread_compat(wpn->Class,sizeof(int),1,fp);
			EndianSwap(wpn->Class);
			fread_compat(wpn->Recoil,sizeof(int),1,fp);
			EndianSwap(wpn->Recoil);
			fread_compat(wpn->Recharge,sizeof(float),1,fp);
			EndianSwap(wpn->Recharge);
			fread_compat(wpn->Drain,sizeof(float),1,fp);
			EndianSwap(wpn->Drain);
			fread_compat(wpn->ROF,sizeof(float),1,fp);
			EndianSwap(wpn->ROF);
			fread_compat(wpn->Proj.Speed,sizeof(int),1,fp);
			EndianSwap(wpn->Proj.Speed);
			fread_compat(wpn->Proj.SpeedVar,sizeof(float),1,fp);
			EndianSwap(wpn->Proj.SpeedVar);
			fread_compat(wpn->Proj.Spread,sizeof(float),1,fp);
			EndianSwap(wpn->Proj.Spread);
			fread_compat(wpn->Proj.Amount,sizeof(int),1,fp);
			EndianSwap(wpn->Proj.Amount);
			fread_endian<int>(fp, wpn->LaserSight);

			fread_endian<int>(fp, wpn->UseSound);
			if(wpn->UseSound) {
                wpn->SndFilename = readString(fp);

				// Load the sample
				wpn->smpSample = LoadGSSample(dir,wpn->SndFilename);
				if(wpn->smpSample == NULL)
					wpn->UseSound = false;
			}

			wpn->Proj.Proj = LoadProjectile(fp);
		}

		wpn->ROF /= 1000.0f;
		wpn->Recharge /= 10.0f;
	}


	// Extra stuff


	// Ninja Rope
	fread_compat(RopeLength,sizeof(int),1,fp);
	EndianSwap(RopeLength);
	fread_compat(RestLength,sizeof(int),1,fp);
	EndianSwap(RestLength);
	fread_compat(Strength,sizeof(float),1,fp);
	EndianSwap(Strength);

	// Worm
	fread_compat(Worm, sizeof(gs_worm_t), 1, fp);
	EndianSwap(Worm.AngleSpeed);
	EndianSwap(Worm.GroundSpeed);
	EndianSwap(Worm.AirSpeed);
	EndianSwap(Worm.Gravity);
	EndianSwap(Worm.JumpForce);
	EndianSwap(Worm.AirFriction);
	EndianSwap(Worm.GroundFriction);


	fclose(fp);

	// Already cached externally
	// Save to cache
	//cCache.SaveMod(dir, this);

	loaded = true;
	
	return GSE_OK;
}


///////////////////
// Load a projectile
proj_t *CGameScript::LoadProjectile(FILE *fp)
{
	int projIndex;
	if(Header.Version > GS_LX56_VERSION) {
		fread_endian<int>(fp, projIndex);
		std::map<int, proj_t*>::iterator f = projectiles.find(projIndex);
		if(f != projectiles.end())
			return f->second;
	}
	else
		projIndex = projectiles.size();
		
	proj_t *proj = new proj_t;
	if(proj == NULL)
		return NULL;
	projectiles[projIndex] = proj;

	fread_compat(proj->Type,			sizeof(int),  1,fp);
	EndianSwap(proj->Type);
	fread_compat(proj->Timer.Time,	sizeof(float),1,fp);
	EndianSwap(proj->Timer.Time);
	fread_compat(proj->Timer.TimeVar,	sizeof(float),1,fp);
	EndianSwap(proj->Timer.TimeVar);
	fread_compat(proj->Trail.Type,			sizeof(int),  1,fp);
	EndianSwap(proj->Trail.Type);
	fread_endian<int>(fp, proj->UseCustomGravity);
	if(proj->UseCustomGravity)
	{
		fread_compat(proj->Gravity,	sizeof(int), 1, fp);
		EndianSwap(proj->Gravity);
	}
	fread_compat(proj->Dampening,		sizeof(int),  1, fp);
	EndianSwap(proj->Dampening);

	if(Header.Version > GS_LX56_VERSION) {
		fread_endian<int>(fp, proj->Width);
		fread_endian<int>(fp, proj->Height);
	}
	else {
		if(proj->Type == PRJ_PIXEL)
			proj->Width = proj->Height = 2;
		else
			proj->Width = proj->Height = 4;
	}
	
	proj->Trail.Proj.Proj = NULL;
	proj->GeneralSpawnInfo.Proj = NULL;
	proj->Hit.Projectiles = false;
	proj->Timer.Projectiles = false;
	proj->PlyHit.Projectiles = false;
	proj->Animating = false;
	proj->Rotating = false;
	proj->RotIncrement = 0;
	proj->Timer.Shake = 0;
	proj->Fallback.Type = PJ_NOTHING;

	switch(proj->Type) {
		case PRJ_POLYGON: {
			proj->polygon.clear();
			Uint32 NumPoints = 0;
			fread_endian<Uint32>(fp, NumPoints);
			for(Uint32 i = 0; i < NumPoints; ++i) {
				VectorD2<int> p;
				fread_endian<int>(fp, p.x);
				fread_endian<int>(fp, p.y);
				proj->polygon.points.push_back(p);
			}
		}
			// fallthrough to read color
		case PRJ_RECT:	
		case PRJ_CIRCLE:
		case PRJ_PIXEL:
		{
			Uint32 NumColours = 0;
			fread_endian<int>(fp, NumColours);
			proj->Colour.resize(NumColours);
			
			for(size_t i = 0; i < NumColours; ++i) {
				if(Header.Version <= GS_LX56_VERSION) {
					int color[3] = {0,0,0};
					fread(color,sizeof(int),3,fp);
					EndianSwap(color[0]);
					EndianSwap(color[1]);
					EndianSwap(color[2]);
					proj->Colour[i] = Color(color[0],color[1],color[2]);
				} else {
					fread_compat(proj->Colour[i].r,1,1,fp);
					fread_compat(proj->Colour[i].g,1,1,fp);
					fread_compat(proj->Colour[i].b,1,1,fp);
					fread_compat(proj->Colour[i].a,1,1,fp);
				}
			}
			break;
		}
		case PRJ_IMAGE:
			proj->ImgFilename = readString(fp);
			
			proj->bmpImage = LoadGSImage(sDirectory, proj->ImgFilename);
			if(!proj->bmpImage)
				modLog("Could not open image '" + proj->ImgFilename + "'");
			
			fread_endian<int>(fp, proj->Rotating);
			fread_compat(proj->RotIncrement, sizeof(int), 1, fp);
			EndianSwap(proj->RotIncrement);
			fread_compat(proj->RotSpeed,sizeof(int),1,fp);
			EndianSwap(proj->RotSpeed);
			fread_endian<int>(fp, proj->UseAngle);
			fread_endian<int>(fp, proj->UseSpecAngle);
			if(proj->UseAngle || proj->UseSpecAngle)
			{
				fread_compat(proj->AngleImages,sizeof(int),1,fp);
				EndianSwap(proj->AngleImages);
			}
			fread_endian<int>(fp, proj->Animating);
			if(proj->Animating) {
				fread_compat(proj->AnimRate,sizeof(float),1,fp);
				EndianSwap(proj->AnimRate);
				fread_compat(proj->AnimType,sizeof(int),1,fp);
				EndianSwap(proj->AnimType);
			}
			break;
						
		case __PRJ_LBOUND: case __PRJ_UBOUND: errors << "LoadProjectile: PRJ BOUND" << endl;
	}


	//
	// Hit
	//
	if(Header.Version <= GS_LX56_VERSION) {
		fread_compat(proj->Hit.Type,sizeof(int),1,fp);
		EndianSwap(proj->Hit.Type);

		// Hit::Explode
		if(proj->Hit.Type == PJ_EXPLODE) {
			fread_compat(proj->Hit.Damage,		sizeof(int),1,fp);
			EndianSwap(proj->Hit.Damage);
			fread_endian<int>(fp, proj->Hit.Projectiles);
			fread_endian<int>(fp, proj->Hit.UseSound);
			fread_compat(proj->Hit.Shake,			sizeof(int),1,fp);
			EndianSwap(proj->Hit.Shake);

			if(proj->Hit.UseSound) {
				proj->Hit.SndFilename = readString(fp);
			}
		}

		// Hit::Bounce
		if(proj->Hit.Type == PJ_BOUNCE) {
			fread_compat(proj->Hit.BounceCoeff,	sizeof(float), 1, fp);
			EndianSwap(proj->Hit.BounceCoeff);
			fread_compat(proj->Hit.BounceExplode, sizeof(int),   1, fp);
			EndianSwap(proj->Hit.BounceExplode);
		}

		// Hit::Carve
		if(proj->Hit.Type == PJ_CARVE) {
			fread_compat(proj->Hit.Damage,		sizeof(int),1,fp);
			EndianSwap(proj->Hit.Damage);
		}
	}
	else { // newer GS version
		proj->Hit.read(this, fp);
	}
	
	if(!bDedicated && proj->Hit.UseSound) {
		// Load the sample
		proj->smpSample = LoadGSSample(sDirectory,proj->Hit.SndFilename);
		
		if(proj->smpSample == NULL) {
			proj->Hit.UseSound = false;
			modLog("Could not open sound '" + proj->Hit.SndFilename + "'");
		}
	} else
		proj->smpSample = NULL;
	



	//
	// Timer
	//
	if(proj->Timer.Time > 0) {
		if(Header.Version <= GS_LX56_VERSION) {
			fread_compat(proj->Timer.Type,sizeof(int),1,fp);
			EndianSwap(proj->Timer.Type);
			if(proj->Timer.Type == PJ_EXPLODE) {
				fread_compat(proj->Timer.Damage,sizeof(int),1,fp);
				EndianSwap(proj->Timer.Damage);
				fread_endian<int>(fp, proj->Timer.Projectiles);
				fread_compat(proj->Timer.Shake,sizeof(int),1,fp);
				EndianSwap(proj->Timer.Shake);
			}
		}
		else {
			proj->Timer.read(this, fp);
		}
	}



	//
	// Player hit
	//
	if(Header.Version <= GS_LX56_VERSION) {
		fread_compat(proj->PlyHit.Type,sizeof(int),1,fp);
		EndianSwap(proj->PlyHit.Type);

		// PlyHit::Explode || PlyHit::Injure
		if(proj->PlyHit.Type == PJ_INJURE || proj->PlyHit.Type == PJ_EXPLODE) {
			fread_endian<int>(fp, proj->PlyHit.Damage);
			fread_endian<int>(fp, proj->PlyHit.Projectiles);
		}

		// PlyHit::Bounce
		if(proj->PlyHit.Type == PJ_BOUNCE) {
			fread_compat(proj->PlyHit.BounceCoeff, sizeof(float), 1, fp);
			EndianSwap(proj->PlyHit.BounceCoeff);
		}
	}
	else {
		proj->PlyHit.read(this, fp);
	}

	if(Header.Version <= GS_LX56_VERSION) {
		// NOTE: this is obsolete, it is used nowhere
		
		//
		// Explode
		//
		fread_compat(proj->Exp.Type,     sizeof(int), 1, fp);
		EndianSwap(proj->Exp.Type);
		fread_compat(proj->Exp.Damage,   sizeof(int), 1, fp);
		EndianSwap(proj->Exp.Damage);
		fread_endian<int>(fp, proj->Exp.Projectiles);
		fread_endian<int>(fp, proj->Exp.UseSound);
		if(proj->Exp.UseSound) {
			proj->Exp.SndFilename = readString(fp);
		}

		//
		// Touch
		//
		fread_compat(proj->Tch.Type,     sizeof(int), 1, fp);
		EndianSwap(proj->Tch.Type);
		fread_compat(proj->Tch.Damage,   sizeof(int), 1, fp);
		EndianSwap(proj->Tch.Damage);
		fread_endian<int>(fp, proj->Tch.Projectiles);
		fread_endian<int>(fp, proj->Tch.UseSound);
		if(proj->Tch.UseSound) {
			proj->Tch.SndFilename = readString(fp);
		}
	}

	if(Header.Version > GS_LX56_VERSION) {
		Uint32 projHitC = 0;
		fread_endian<Uint32>(fp, projHitC);
		proj->actions.resize(projHitC);
		for(Uint32 i = 0; i < projHitC; ++i) {
			proj->actions[i].read(this, fp);
		}
	}
	
	if(Header.Version > GS_LX56_VERSION) {
		proj->GeneralSpawnInfo.read(this, fp);		
	}
	else if(proj->Timer.Projectiles || proj->Hit.Projectiles || proj->PlyHit.Projectiles || proj->Exp.Projectiles ||
       proj->Tch.Projectiles) {
		fread_endian<int>(fp, proj->GeneralSpawnInfo.Useangle);
		fread_compat(proj->GeneralSpawnInfo.Angle,		sizeof(int),	1, fp);
		EndianSwap(proj->GeneralSpawnInfo.Angle);
		fread_compat(proj->GeneralSpawnInfo.Amount,	sizeof(int),	1, fp);
		EndianSwap(proj->GeneralSpawnInfo.Amount);
		fread_compat(proj->GeneralSpawnInfo.Spread,	sizeof(float),	1, fp);
		EndianSwap(proj->GeneralSpawnInfo.Spread);
		fread_compat(proj->GeneralSpawnInfo.Speed,		sizeof(int),	1, fp);
		EndianSwap(proj->GeneralSpawnInfo.Speed);
		fread_compat(proj->GeneralSpawnInfo.SpeedVar,	sizeof(float),	1, fp);
		EndianSwap(proj->GeneralSpawnInfo.SpeedVar);

		proj->GeneralSpawnInfo.Proj = LoadProjectile(fp);
	}

	if(Header.Version > GS_LX56_VERSION) {
		if(proj->Timer.Projectiles) {
			bool projIsSet = false; fread_endian<char>(fp, projIsSet);
			if(projIsSet) proj->Timer.Proj.read(this, fp);
		}
		if(proj->Hit.Projectiles) {
			bool projIsSet = false; fread_endian<char>(fp, projIsSet);
			if(projIsSet) proj->Hit.Proj.read(this, fp);	
		}
		if(proj->PlyHit.Projectiles) {
			bool projIsSet = false; fread_endian<char>(fp, projIsSet);
			if(projIsSet) proj->PlyHit.Proj.read(this, fp);
		}
	}
	
	// Projectile trail
	if(proj->Trail.Type == TRL_PROJECTILE) {

		fread_endian<int>(fp, proj->Trail.Proj.UseParentVelocityForSpread);
		fread_endian<float>(fp, proj->Trail.Delay);
		// Change from milli-seconds to seconds
		proj->Trail.Delay /= 1000.0f;
		
		if(Header.Version <= GS_LX56_VERSION) {
			fread_compat(proj->Trail.Proj.Amount,			sizeof(int),	1, fp);
			EndianSwap(proj->Trail.Proj.Amount);
			fread_compat(proj->Trail.Proj.Speed,			sizeof(int),	1, fp);
			EndianSwap(proj->Trail.Proj.Speed);
			fread_compat(proj->Trail.Proj.SpeedVar,		sizeof(float),	1, fp);
			EndianSwap(proj->Trail.Proj.SpeedVar);
			fread_compat(proj->Trail.Proj.Spread,			sizeof(float),	1, fp);
			EndianSwap(proj->Trail.Proj.Spread);

			proj->Trail.Proj.Proj = LoadProjectile(fp);
		}
		else { // new GS versions
			proj->Trail.Proj.read(this, fp);
		}

	}

	return proj;
}


///////////////////
// Load an image
SDL_Surface * CGameScript::LoadGSImage(const std::string& dir, const std::string& filename)
{
	SmartPointer<SDL_Surface> img = NULL;

	// First, check the gfx directory in the mod dir
	img = LoadGameImage(dir + "/gfx/" + filename, true);
	if(img.get())  {
		SetColorKey(img.get());
		CachedImages.push_back(img);
		return img.get();
	}

	// Check the gfx directory in the data dir
	img = LoadGameImage("data/gfx/" + filename, true);
	if(img.get())
	{ 
		SetColorKey(img.get());
		CachedImages.push_back(img);
	}
	return img.get();
}


///////////////////
// Load a sample
SoundSample * CGameScript::LoadGSSample(const std::string& dir, const std::string& filename)
{
	SmartPointer<SoundSample> smp = NULL;

	// First, check the sfx directory in the mod dir
	smp = LoadSample(dir + "/sfx/" + filename, 10);

	if(smp.get())
	{
		CachedSamples.push_back(smp);
		return smp.get();
	}

	// Check the sounds directory in the data dir
	smp = LoadSample("data/sounds/" + filename, 10);
	if(smp.get())
		CachedSamples.push_back(smp);
	return smp.get();
}



///////////////////
// Find a weapon based on its name
const weapon_t *CGameScript::FindWeapon(const std::string& name)
{
	int n;

	// Go through each weapon
	weapon_t *wpn = Weapons;
	for(n=0;n<NumWeapons;n++,wpn++) {

		if(stringcaseequal(wpn->Name, name))
			return wpn;
	}

	// Instead of returning NULL, just return the first weapon
	return Weapons;
}


///////////////////
// Returns true if the weapon is in the game script
bool CGameScript::weaponExists(const std::string& szName)
{
    // Go through each weapon
	weapon_t *wpn = Weapons;
	for(int n=0;n<NumWeapons;n++,wpn++) {

		if(stringcasecmp(wpn->Name, szName) == 0)
			return true;
	}

    // Not found
    return false;
}



// Helper function
static size_t GetProjSize(proj_t *prj)
{
	if (prj)
		return 	prj->Exp.SndFilename.size() + prj->filename.size() +
				prj->Hit.SndFilename.size() + prj->ImgFilename.size() +
				prj->Tch.SndFilename.size() +
				/*GetSurfaceMemorySize(prj->bmpImage.get()) + */
				sizeof(proj_t);
	else
		return 0;
}

//////////////////
// Returns the memory occupied by this gamescript
size_t CGameScript::GetMemorySize()
{
	size_t res = sizeof(CGameScript) + sDirectory.size();
	weapon_t *it = Weapons;
	for (int i = 0; i < NumWeapons; i++, it++)  {
		res += sizeof(weapon_t) + sizeof(SoundSample);
		res += it->SndFilename.size();
	}
	for(Projectiles::iterator i = projectiles.begin(); i != projectiles.end(); ++i) {
		res += GetProjSize(i->second);		
	}
	
	return res;
}


///////////////////
// Shutdown the game script
void CGameScript::Shutdown()
{
	loaded = false;
	needCollisionInfo = false;
	
    // Close the log file
    if(pModLog) {
        fclose(pModLog);
        pModLog = NULL;
    }

	for(Projectiles::iterator i = projectiles.begin(); i != projectiles.end(); ++i) {
		ShutdownProjectile(i->second);
	}
	projectiles.clear();
	savedProjs.clear();
	projFileIndexes.clear();
	
	if(Weapons)
		delete[] Weapons;
	Weapons = NULL;
}


///////////////////
// Shutdown a projectile
void CGameScript::ShutdownProjectile(proj_t *prj)
{
	if(prj) {
		delete prj;
	}
}


///////////////////
// Check if a file is a valid liero game script
bool CGameScript::CheckFile(const std::string& dir, std::string& name, bool abs_filename)
{
	name = "";

	// Open it
	FILE *fp = NULL;
	if(abs_filename) {
		std::string filename;
		// we still need to add "/script.lgs" and then do an exact filename search
		if(GetExactFileName(dir + "/script.lgs", filename))
	 		fp = fopen(filename.c_str(), "rb");
	} else
		fp = OpenGameFile(dir + "/script.lgs", "rb");
	
	if(fp == NULL) {
		// try source gamescript
		
		std::string filename = dir + "/main.txt";
		if(abs_filename) {
			if(!GetExactFileName(dir + "/main.txt", filename))
				return false;
		} else {
			if(!IsFileAvailable(filename))
				return false;
		}
		
		ReadString(filename,"General","ModName", name,"untitled", abs_filename);
		return true;
	}

	// Header
	gs_header_t head;
	memset(&head,0,sizeof(gs_header_t));
	fread_compat(head,sizeof(gs_header_t),1,fp);
	fclose(fp);

	EndianSwap(head.Version);
	// for security
	fix_markend(head.ID);
	fix_markend(head.ModName);

	// Check ID
	if(strcmp(head.ID,"Liero Game Script") != 0) {
		warnings << "GS:CheckFile: WARNING: " << dir << "/script.lgs is not a Liero game script";
		warnings << " (but \"" << head.ID << "\" instead)" << endl;
		return false;
	}

	// Check version
	if(head.Version < GS_FIRST_SUPPORTED_VERSION || head.Version > GS_VERSION) {
		warnings << "GS:CheckFile: WARNING: " << dir << "/script.lgs has the wrong version";
		warnings << " (" << (unsigned)head.Version << ", required is in the range ";
		warnings << "[" << GS_FIRST_SUPPORTED_VERSION << "," << GS_VERSION << "])" << endl;
		return false;
	}

	name = head.ModName;
	return true;
}


///////////////////
// Return an error message based on code
std::string CGameScript::getError(int code)
{
	std::string text = "Undefined error";

	switch(code) {

		case GSE_MEM:
			text = "Out of memory";
			break;

		case GSE_VERSION:
			text = "Incorrect version";
			break;

		case GSE_FILE:
			text = "Could not open file";
			break;

		case GSE_BAD:
			text = "Bad file format";
			break;
	}

	return text;
}


///////////////////
// Write info to a mod log file
void CGameScript::modLog(const std::string& text)
{
	notes << "modLog: " << text << endl;

	if(!pModLog) {
		pModLog = OpenGameFile("modlog.txt","wt");
		if(!pModLog)
			return;
		fprintf(pModLog,"Log file for mod:\n%s\n--------------------------------\n",Header.ModName);
	}

	if (text.size() != 0)
		fprintf(pModLog,"%s\n", text.c_str());
}

// Not needed with new caching system - game script won't ever change it's state during game
/*
///////////////////
// Copies infor from anothe gamescript
void CGameScript::CopyFrom(CGameScript *cg)
{
	sDirectory = cg->sDirectory;
	Header = cg->Header;
	NumWeapons = cg->NumWeapons;

	// HINT: only a pointer is copied here, because the weapon info does not change
	// so it would be wasting of memory if we copied the whole list
	Weapons = cg->Weapons;

	Worm = cg->Worm;
	RopeLength = cg->RopeLength;
	RestLength = cg->RestLength;
	Strength = cg->Strength;
}
*/

template <> void SmartPointer_ObjectDeinit<CGameScript> ( CGameScript * obj )
{
	delete obj;
}






///////////////////
// Compile
bool CGameScript::Compile(const std::string& dir)
{
	Shutdown();
	
	CGameScript* Game = this;
	
	// Add some keywords
	// IMPORTANT: Every keyword (key) need to be different.
	AddKeyword("WPN_PROJECTILE",WPN_PROJECTILE);
	AddKeyword("WPN_SPECIAL",WPN_SPECIAL);
	AddKeyword("WPN_BEAM",WPN_BEAM);
	AddKeyword("WCL_AUTOMATIC",WCL_AUTOMATIC);
	AddKeyword("WCL_POWERGUN",WCL_POWERGUN);
	AddKeyword("WCL_GRENADE",WCL_GRENADE);
	AddKeyword("WCL_CLOSERANGE",WCL_CLOSERANGE);
	AddKeyword("PRJ_PIXEL",PRJ_PIXEL);
	AddKeyword("PRJ_IMAGE",PRJ_IMAGE);
	AddKeyword("PRJ_CIRCLE",PRJ_CIRCLE);
	AddKeyword("PRJ_POLYGON",PRJ_POLYGON);
	AddKeyword("PRJ_RECT", PRJ_RECT);
	AddKeyword("Bounce",PJ_BOUNCE);
	AddKeyword("Explode",PJ_EXPLODE);
	AddKeyword("Injure",PJ_INJURE);
	AddKeyword("Carve",PJ_CARVE);
	AddKeyword("Dirt",PJ_DIRT);
	AddKeyword("GreenDirt",PJ_GREENDIRT);
	AddKeyword("Disappear",PJ_DISAPPEAR);
	AddKeyword("Nothing",PJ_NOTHING);
	AddKeyword("Disappear2", PJ_DISAPPEAR2);
	AddKeyword("GoThrough", PJ_GOTHROUGH);
	AddKeyword("Timer", Proj_Event::PET_TIMER); 
	AddKeyword("ProjHit", Proj_Event::PET_PROJHIT); 
	AddKeyword("WormHit", Proj_Event::PET_WORMHIT); 
	AddKeyword("TerrainHit", Proj_Event::PET_TERRAINHIT); 
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
	
	
	int num,n;
	std::string filename = dir + "/Main.txt";

	// Check the file
	FILE *fp = OpenGameFile(filename, "rt");
	if(!fp) {
		warnings << "CGameScript::Compile: Could not open the file '" << filename << "' for reading" << endl;
		return false;
	} else
		fclose(fp);


	std::string modname;
	ReadString(filename,"General","ModName", modname,"untitled");
	fix_strncpy(Header.ModName, modname.c_str());

	notes << "Compiling '" << modName() << "'" << endl;

	ReadInteger(filename,"Weapons","NumWeapons",&num,0);


	// Weapons
	Game->initNewWeapons(num);


	// Compile the weapons
	for(n=0;n<Game->GetNumWeapons();n++) {
		std::string wpn = "Weapon" + itoa(n+1);

		std::string weap;
		ReadString(filename,"Weapons",wpn,weap,"");

		if(!CompileWeapon(dir,weap,n))
			return false;
	}

	// Compile the extra stuff
	CompileExtra(dir);

	loaded = true;
	
	return true;
}


///////////////////
// Compile a weapon
bool CGameScript::CompileWeapon(const std::string& dir, const std::string& weapon, int id)
{
	CGameScript* Game = this;
	
	weapon_t *Weap = Game->Weapons+id;
	std::string file = dir + "/" + weapon;

	Weap->ID = id;
	Weap->Proj.Proj = NULL;
	Weap->UseSound = false;
	Weap->Special = SPC_NONE;
	Weap->Type = WPN_PROJECTILE;
	Weap->Proj.Proj = NULL;
	Weap->LaserSight = false;

	ReadString(file,"General","Name",Weap->Name,"");
	notes << "  Compiling Weapon '" << Weap->Name << "'" << endl;

	ReadKeyword(file,"General","Type",(int*)&Weap->Type,WPN_PROJECTILE);

	
	
	// Special Weapons
	if(Weap->Type == WPN_SPECIAL) {
		
		ReadKeyword(file,"General","Special",(int*)&Weap->Special,SPC_NONE);

		// If it is a special weapon, read the values depending on the special weapon
		// We don't bother with the 'normal' values
		switch(Weap->Special) {
			// Jetpack
			case SPC_JETPACK:
				CompileJetpack(file, Weap);
				break;

			default:
				notes << "   Error: Unknown special type" << endl;
		}
		return true;
	}


	// Beam Weapons
	if(Weap->Type == WPN_BEAM) {

		CompileBeam(file,Weap);
		return true;
	}


	// Projectile Weapons
	ReadKeyword(file,"General","Class",(int*)&Weap->Class,WCL_AUTOMATIC);
	ReadInteger(file,"General","Recoil",&Weap->Recoil,0);
	ReadFloat(file,"General","Recharge",&Weap->Recharge,0); Weap->Recharge /= 10.0f;
	ReadFloat(file,"General","Drain",&Weap->Drain,0);
	ReadFloat(file,"General","ROF",&Weap->ROF,0); Weap->ROF /= 1000.0f;
	ReadKeyword(file, "General", "LaserSight", &Weap->LaserSight, false);
	if(ReadString(file,"General","Sound",Weap->SndFilename,"")) {
		Weap->UseSound = true;
	
		if(!bDedicated) {
			// Load the sample
			Weap->smpSample = LoadGSSample(dir,Weap->SndFilename);
		}	
	}
	
	Weap->Proj.readFromIni(this, dir, file, "Projectile");
	
	if(Weap->Proj.UseParentVelocityForSpread) {
		warnings << "UseProjVelocity is set in Projectile-section (" << weapon << "); this was not supported in LX56 thus we ignore it" << endl;
		Weap->Proj.UseParentVelocityForSpread = false;
	}
	
	if(!Weap->Proj.Useangle) {
		warnings << "Useangle is set in Projectile-section (" << weapon << "); this was not supported in LX56 thus we ignore it" << endl;
		Weap->Proj.Useangle = true;
	}

	if(Weap->Proj.Angle != 0) {
		warnings << "Angle is set in Projectile-section (" << weapon << "); this was not supported in LX56 thus we ignore it" << endl;
		Weap->Proj.Angle = 0;
	}	

	if(Weap->Proj.Proj == NULL) {
		warnings << "projectile not set for weapon " << weapon << endl;
		return false;
	}
	
	return true;
}


///////////////////
// Compile a beam weapon
void CGameScript::CompileBeam(const std::string& file, weapon_t *Weap)
{
	ReadInteger(file,"General","Recoil",&Weap->Recoil,0);
	ReadFloat(file,"General","Recharge",&Weap->Recharge,0); Weap->Recharge /= 10.0f;
	ReadFloat(file,"General","Drain",&Weap->Drain,0);
	ReadFloat(file,"General","ROF",&Weap->ROF,0); Weap->ROF /= 1000.0f;
	if(ReadString(file,"General","Sound",Weap->SndFilename,""))
		Weap->UseSound = true;

	Weap->Bm.readFromIni(file, "Beam");
}


///////////////////
// Compile a projectile
proj_t *CGameScript::CompileProjectile(const std::string& dir, const std::string& pfile)
{
	ProjFileMap::iterator f = projFileIndexes.find(pfile);
	if(f != projFileIndexes.end()) {
		notes << "    Reuse already compiled Projectile '" << pfile << "'" << endl;
		return projectiles[f->second];
	}

	proj_t *proj = new proj_t;
	if(proj == NULL)
		return NULL;

	int projIndex = projectiles.size();
	projectiles[projIndex] = proj;	
	projFileIndexes[pfile] = projIndex;
	
	// Load the projectile
	std::string file = dir + "/" + pfile;
	notes << "    Compiling Projectile '" << pfile << "'" << endl;
	
	if(!IsFileAvailable(dir + "/" + pfile, false))
		warnings << "projectile file " << pfile << " not found, using defaults..." << endl;
	
	proj->filename = pfile;
	
	proj->Timer.Projectiles = false;
	proj->Hit.Projectiles = false;
	proj->PlyHit.Projectiles = false;
	proj->Exp.Projectiles = false;
	proj->Tch.Projectiles = false;
	proj->GeneralSpawnInfo.Proj = NULL;
	proj->Trail.Proj.Proj = NULL;
	proj->Animating = false;
	proj->UseCustomGravity = false;

	ReadKeyword(file,"General","Type",(int*)&proj->Type,PRJ_PIXEL);
	ReadFloat(file,"General","Timer",&proj->Timer.Time,0);
	ReadFloat(file, "General", "TimerVar", &proj->Timer.TimeVar, 0);
	ReadKeyword(file,"General","Trail",(int*)&proj->Trail.Type,TRL_NONE);
	
	if( ReadInteger(file,"General","Gravity",&proj->Gravity, 0) )
		proj->UseCustomGravity = true;

	ReadFloat(file,"General","Dampening",&proj->Dampening,1.0f);

	if(proj->Type == PRJ_PIXEL)
		proj->Width = proj->Height = 2;
	else
		proj->Width = proj->Height = 4;
	
	ReadInteger(file, "General", "Width", &proj->Width, proj->Width);
	ReadInteger(file, "General", "Height", &proj->Height, proj->Height);
	
	proj->Colour.clear();
	proj->polygon.clear();
	switch(proj->Type) {
		case PRJ_POLYGON:
			for(size_t i = 0; ; ++i) {
				VectorD2<int> p;
				if( ReadVectorD2(file,"General", "P" + itoa(i+1), p) ) {
					proj->polygon.points.push_back(p);
				} else
					break;
			}
			if(proj->polygon.points.size() == 0) {
				warnings << "no points specified for PRJ_POLYGON projectile " << pfile << "; fallback to PRJ_PIXEL" << endl;
				proj->Type = PRJ_PIXEL;
			}
			else
				// put the first point to the end to have a closed figure
				proj->polygon.points.push_back( *proj->polygon.points.begin() );
			
			// fallthrough to read color
		case PRJ_RECT:	
		case PRJ_CIRCLE:
		case PRJ_PIXEL:
			for(size_t i = 0; ; ++i) {
				Color col;
				if( ReadColour(file,"General","Colour" + itoa(i+1), col, Color()) ) {
					proj->Colour.push_back(col);
				} else
					break;
			}
			if(proj->Colour.size() == 0) {
				warnings << "no colors specified for projectile " << pfile << ", using black" << endl;
				proj->Colour.push_back(Color());
			}
			break;
			
		case PRJ_IMAGE:
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
				ReadKeyword(file,"General","AnimType",(int*)&proj->AnimType,ANI_ONCE);
			}
	
			if(!bDedicated) {
				proj->bmpImage = LoadGSImage(dir, proj->ImgFilename);
				if(!proj->bmpImage)
					modLog("Could not open image '" + proj->ImgFilename + "'");
			}
			break;
			
		case __PRJ_LBOUND: case __PRJ_UBOUND: errors << "PRJ BOUND err" << endl;
	}

	// general Projectile spawn info
	{
		proj->GeneralSpawnInfo.readFromIni(this, dir, file, "Projectile");
		
		if(proj->GeneralSpawnInfo.UseParentVelocityForSpread) {
			warnings << "UseProjVelocity is set in Projectile-section (" << pfile << "); this was not supported in LX56 thus we ignore it" << endl;
			proj->GeneralSpawnInfo.UseParentVelocityForSpread = false;
		}
		
		if(proj->Timer.needGeneralSpawnInfo() || proj->Hit.needGeneralSpawnInfo() || proj->PlyHit.needGeneralSpawnInfo() || proj->Fallback.needGeneralSpawnInfo()) // HINT: not complete but it's not that important
			if(!proj->GeneralSpawnInfo.isSet())
				warnings << "Projectile section (" << pfile << ") is not specified correctly but needed" << endl;
	}
	
	proj->Hit.readFromIni(this, dir, file, "Hit");
	if(!bDedicated && proj->Hit.UseSound) {
		// Load the sample
		proj->smpSample = LoadGSSample(dir, proj->Hit.SndFilename);
		
		if(proj->smpSample == NULL) {
			modLog("Could not open sound '" + proj->Hit.SndFilename + "'");
		}
	}
	
	if(proj->Hit.needGeneralSpawnInfo() && !proj->GeneralSpawnInfo.isSet()) {
		warnings << dir << "/" << pfile << ": Hit section wants to spawn projectiles but there is no spawning information" << endl;
		proj->Hit.Projectiles = false;
	}
	
	// Timer	
	if(proj->Timer.Time > 0) {
		proj->Timer.readFromIni(this, dir, file, "Time");

		if(proj->Timer.needGeneralSpawnInfo() && !proj->GeneralSpawnInfo.isSet()) {
			warnings << dir << "/" << pfile << ": Timer section wants to spawn projectiles but there is no spawning information" << endl;
			proj->Timer.Projectiles = false;
		}
	}

	// Player hit
	proj->PlyHit.Type = PJ_INJURE;
	proj->PlyHit.readFromIni(this, dir, file, "PlayerHit");
	
	if(proj->PlyHit.Shake != 0) {
		warnings << "projectile " << file << " has PlayerHit.Shake != 0 which was not supported earlier. this is ignored" << endl;
		proj->PlyHit.Shake = 0;
	}
	
	if(proj->PlyHit.BounceExplode != 0) {
		warnings << "projectile " << file << " has PlayerHit.BounceExplode != 0 which was not supported earlier. this is ignored" << endl;
		proj->PlyHit.BounceExplode = 0;
	}
	
	if(proj->PlyHit.UseSound) {
		warnings << "projectile " << file << " has sound set, which was not supported earlier, thus it's ignored now" << endl;
		proj->PlyHit.UseSound = false;
	}

	if(proj->PlyHit.needGeneralSpawnInfo() && !proj->GeneralSpawnInfo.isSet()) {
		warnings << dir << "/" << pfile << ": PlayerHit section wants to spawn projectiles but there is no spawning information" << endl;
		proj->PlyHit.Projectiles = false;
	}
	
	/*
    // OnExplode
	ReadKeyword( file, "Explode", "Type",   (int*)&proj->Exp.Type, PJ_NOTHING );
	ReadInteger( file, "Explode", "Damage",     &proj->Exp.Damage, 0 );
	ReadKeyword( file, "Explode", "Projectiles",&proj->Exp.Projectiles, false );
	ReadInteger( file, "Explode", "Shake",      &proj->Exp.Shake, 0 );
	proj->Exp.UseSound = false;
	if( ReadString(file, "Explode", "Sound", proj->Exp.SndFilename,"") ) {
		proj->Exp.UseSound = true;
	}

    // Touch
	ReadKeyword( file, "Touch", "Type",   (int*)&proj->Tch.Type, PJ_NOTHING );
	ReadInteger( file, "Touch", "Damage",     &proj->Tch.Damage, 0 );
	ReadKeyword( file, "Touch", "Projectiles",&proj->Tch.Projectiles, false );
	ReadInteger( file, "Touch", "Shake",      &proj->Tch.Shake, 0 );
	proj->Tch.UseSound = false;
	if( ReadString(file, "Touch", "Sound", proj->Tch.SndFilename,"") )
		proj->Tch.UseSound = true;
	 */
	
	{
		int projHitC = 0;
		ReadInteger(file, "General", "ActionNum", &projHitC, 0);
		for(int i = 0; i < projHitC; ++i) {
			Proj_EventAndAction act;
			if(!act.readFromIni(this, dir, file, "Action" + itoa(i+1))) continue;
			
			if(act.needGeneralSpawnInfo() && !proj->GeneralSpawnInfo.isSet()) {
				warnings << dir << "/" << pfile << ": Action" << (i+1) << " section wants to spawn projectiles but there is no spawning information" << endl;
				act.Projectiles = false;
			}

			if(!act.hasAction()) {
				warnings << "section Action" << (i+1) << " (" << pfile << ") doesn't have any effect" << endl;
				continue;
			}
			
			proj->actions.push_back(act);
		}
	}

	{
		proj->Fallback.Type = PJ_NOTHING;
		proj->Fallback.readFromIni(this, dir, file, "Fallback");

		if(proj->Fallback.needGeneralSpawnInfo() && !proj->GeneralSpawnInfo.isSet()) {
			warnings << dir << "/" << pfile << ": Fallback section wants to spawn projectiles but there is no spawning information" << endl;
			proj->Fallback.Projectiles = false;
		}
	}
	
	
	// Projectile trail
	if(proj->Trail.Type == TRL_PROJECTILE) {
		ReadFloat(file, "ProjectileTrail", "Delay",  &proj->Trail.Delay, 100); proj->Trail.Delay /= 1000.0f;

		// we have some other default values here
		proj->Trail.Proj.Amount = 1;
		proj->Trail.Proj.Speed = 100;
		
		proj->Trail.Proj.readFromIni(this, dir, file, "ProjectileTrail");
		
		if(proj->Trail.Proj.Useangle) {
			warnings << "Useangle is set in ProjectileTrail-section (" << pfile << "); this was not supported in LX56 thus we ignore it" << endl;
			proj->Trail.Proj.Useangle = false;
		}
		
		if(proj->Trail.Proj.Angle != 0) {
			warnings << "Angle is set in ProjectileTrail-section (" << pfile << "); this was not supported in LX56 thus we ignore it" << endl;
			proj->Trail.Proj.Useangle = false;
		}
		
		if(proj->Trail.Type == TRL_PROJECTILE && !proj->Trail.Proj.isSet()) {
			warnings << dir << "/" << pfile << ": ProjectileTrail section wants to spawn projectiles but there is no spawning information" << endl;
			proj->Trail.Type = TRL_NONE;			
		}
	}

	return proj;
}


///////////////////
// Compile the extra stuff
bool CGameScript::CompileExtra(const std::string& dir)
{
	CGameScript* Game = this;
	
	notes << "   Compiling Extras" << endl;

	std::string file = dir + "/main.txt";

	// Ninja Rope
	notes << "  Compiling Ninja Rope" << endl;

	int ropel, restl;
	float strength;

	ReadInteger(file,"NinjaRope","RopeLength",&ropel,0);
	ReadInteger(file,"NinjaRope","RestLength",&restl,0);
	ReadFloat(file,"NinjaRope","Strength",&strength,0);

	Game->SetRopeLength(ropel);
	Game->SetRestLength(restl);
	Game->SetStrength(strength);


	// Worm
	notes << "  Compiling Worm" << endl;
	gs_worm_t *wrm = &Game->Worm;

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
bool CGameScript::CompileJetpack(const std::string& file, weapon_t *Weap)
{
	Weap->Proj.Proj = NULL;

	ReadInteger(file, "JetPack", "Thrust", (int*)&Weap->tSpecial.Thrust, 0);
	ReadFloat(file, "JetPack", "Drain", &Weap->Drain, 0);
	ReadFloat(file, "JetPack", "Recharge", &Weap->Recharge, 0);	Weap->Recharge /= 10.0f;

	Weap->ROF = 1.0f / 1000.0f;

	return true;
}


bool Proj_SpawnInfo::readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section) {
	ReadKeyword(file, section, "AddParentVel", &AddParentVel, AddParentVel); // new in OLX beta9
	ReadMatrixD2(file, section, "ParentVelFactor", ParentVelFactor, ParentVelFactor); // new in OLX beta9
	ReadVectorD2(file, section, "PosDiff", PosDiff, PosDiff); // new in OLX beta9
	ReadVectorD2(file, section, "SnapToGrid", SnapToGrid, SnapToGrid); // new in OLX beta9
	
	ReadKeyword(file, section, "Useangle", &Useangle, Useangle);
	ReadInteger(file, section, "Angle", &Angle, Angle);
	
	ReadKeyword(file, section, "UseProjVelocity", &UseParentVelocityForSpread, UseParentVelocityForSpread);
	
	ReadInteger(file, section, "Amount", &Amount, Amount);
	ReadInteger(file, section, "Speed",  &Speed, Speed);
	ReadFloat(file, section, "SpeedVar",  &SpeedVar, SpeedVar);
	ReadFloat(file, section, "Spread", &Spread, Spread);
	
	std::string prjfile;
	ReadString(file, section, "Projectile", prjfile, "");
	if(prjfile != "") Proj = gs->CompileProjectile(dir, prjfile);
	else Proj = NULL;
	
	return true;
}

bool Proj_SpawnInfo::read(CGameScript* gs, FILE* fp) {
	if(gs->GetHeader()->Version <= GS_LX56_VERSION) {
		errors << "Proj_SpawnInfo::read called for old GS version" << endl;
		return false;
	}

	fread_endian<char>(fp, AddParentVel);
	fread_endian_M<float>(fp, ParentVelFactor);
	fread_endian_V<int>(fp, PosDiff);
	fread_endian_V<int>(fp, SnapToGrid);
	fread_endian<char>(fp, Useangle);
	fread_endian<int>(fp, Angle);
	fread_endian<int>(fp, Amount);
	fread_endian<int>(fp, Speed);
	fread_endian<float>(fp, SpeedVar);
	fread_endian<float>(fp, Spread);
	Proj = gs->LoadProjectile(fp);
	return Proj != NULL;
}

bool Proj_SpawnInfo::write(CGameScript* gs, FILE* fp) {
	if(gs->GetHeader()->Version <= GS_LX56_VERSION) {
		errors << "Proj_SpawnInfo::write called for old GS version" << endl;
		return false;
	}

	fwrite_endian<char>(fp, AddParentVel);
	fwrite_endian_M<float>(fp, ParentVelFactor);
	fwrite_endian_V<int>(fp, PosDiff);
	fwrite_endian_V<int>(fp, SnapToGrid);
	fwrite_endian<char>(fp, Useangle);
	fwrite_endian<int>(fp, Angle);
	fwrite_endian<int>(fp, Amount);
	fwrite_endian<int>(fp, Speed);
	fwrite_endian<float>(fp, SpeedVar);
	fwrite_endian<float>(fp, Spread);
	return gs->SaveProjectile(Proj, fp);
}


bool Wpn_Beam::readFromIni(const std::string& file, const std::string& section) {
	ReadInteger(file, section, "Damage", &Damage, Damage);
	ReadInteger(file, section, "Length", &Length, Length);
	ReadInteger(file, section, "PlayerDamage", &PlyDamage, PlyDamage);
	ReadColour(file, section, "Colour", Colour, Colour);
	ReadInteger(file, section, "InitWidth", &InitWidth, InitWidth);
	ReadFloat(file, section, "WidthIncrease", &WidthIncrease, WidthIncrease);
	ReadKeyword(file, section, "DistributeDamageOverWidth", &DistributeDamageOverWidth, DistributeDamageOverWidth);
	return true;
}

bool Wpn_Beam::read(CGameScript* gs, FILE* fp) {
	if(gs->GetHeader()->Version <= GS_LX56_VERSION) {
		int r=0,g=0,b=0;
		fread_endian<int>(fp, r);
		fread_endian<int>(fp, g);
		fread_endian<int>(fp, b);
		Colour = Color(r,g,b);
	}
	else {
		fread_compat(Colour.r, 1,  1, fp);
		fread_compat(Colour.g, 1,  1, fp);
		fread_compat(Colour.b, 1,  1, fp);
		fread_compat(Colour.a, 1,  1, fp);
	}
	
	fread_endian<int>(fp, Damage);
	fread_endian<int>(fp, PlyDamage);
	fread_endian<int>(fp, Length);

	if(gs->GetHeader()->Version > GS_LX56_VERSION) {
		fread_endian<int>(fp, InitWidth);
		fread_endian<float>(fp, WidthIncrease);
		fread_endian<char>(fp, DistributeDamageOverWidth);
	}
	
	return true;
}

bool Wpn_Beam::write(CGameScript* gs, FILE* fp) {	
	if(gs->GetHeader()->Version <= GS_LX56_VERSION) {
		fwrite_endian<int>(fp, Colour.r);
		fwrite_endian<int>(fp, Colour.g);
		fwrite_endian<int>(fp, Colour.b);
	}
	else {
		fwrite_endian<Uint8>(fp, Colour.r);
		fwrite_endian<Uint8>(fp, Colour.g);
		fwrite_endian<Uint8>(fp, Colour.b);
		fwrite_endian<Uint8>(fp, Colour.a);
	}
	
	fwrite_endian<int>(fp, Damage);
	fwrite_endian<int>(fp, PlyDamage);
	fwrite_endian<int>(fp, Length);

	if(gs->GetHeader()->Version > GS_LX56_VERSION) {
		fwrite_endian<int>(fp, InitWidth);
		fwrite_endian<float>(fp, WidthIncrease);
		fwrite_endian<char>(fp, DistributeDamageOverWidth);
	}
	
	return true;
}


bool Proj_Action::readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section, int deepCounter) {
	ReadKeyword(file, section, "Type", (int*)&Type, Type);
	ReadKeyword(file,section,"Projectiles",&Projectiles,false);
	ReadInteger(file,section,"Damage",&Damage,Damage);
	ReadInteger(file,section,"Shake",&Shake,Shake);
		
	UseSound = false;
	if(ReadString(file,section,"Sound",SndFilename,"")) {
		UseSound = true;			
	}
	
	ReadFloat(file,section,"BounceCoeff",&BounceCoeff,BounceCoeff);
	ReadInteger(file,section,"BounceExplode",&BounceExplode,BounceExplode);

	ReadFloat(file,section,"GoThroughSpeed",&GoThroughSpeed,GoThroughSpeed);	

	if(ReadVectorD2(file, section, "OverwriteOwnSpeed", OverwriteOwnSpeed, OverwriteOwnSpeed))
		UseOverwriteOwnSpeed = true;
	ReadMatrixD2(file, section, "ChangeOwnSpeed", ChangeOwnSpeed, ChangeOwnSpeed);
	if(ReadVectorD2(file, section, "OverwriteTargetSpeed", OverwriteTargetSpeed, OverwriteTargetSpeed))
		UseOverwriteTargetSpeed = true;
	ReadMatrixD2(file, section, "ChangeTargetSpeed", ChangeTargetSpeed, ChangeTargetSpeed);
	
	if(Projectiles)
		Proj.readFromIni(gs, dir, file, section + ".Projectile");
	
	std::string addActionSection;
	ReadString(file, section, "Additional", addActionSection, "");
	TrimSpaces(addActionSection);
	if(addActionSection != "") {
		if(deepCounter > 1000) {
			warnings << "There is probably an additional action definition loop in " << file << ":" << section << endl;
			return false;
		}
		
		additionalAction = new Proj_Action();
		additionalAction->Type = PJ_NOTHING;
		if(!additionalAction->readFromIni(gs, dir, file, addActionSection, deepCounter + 1)) {
			delete additionalAction; additionalAction = NULL;
			return false;
		}
		else if(!additionalAction->hasAction()) {
			warnings << "additional action " << addActionSection << "(in " << file << ":" << section << ") does not have any effect" << endl;
			delete additionalAction; additionalAction = NULL;
		}
	}
	else
		additionalAction = NULL;
	
	return true;
}

bool Proj_Action::read(CGameScript* gs, FILE* fp) {
	if(gs->GetHeader()->Version <= GS_LX56_VERSION) {
		errors << "Proj_Action::read called for old GS version" << endl;
		return false;
	}
	
	fread_endian<int>(fp, (int&)Type);
	fread_endian<char>(fp, Projectiles);
	fread_endian<int>(fp, Damage);
	fread_endian<int>(fp, Shake);
	fread_endian<char>(fp, UseSound);	
	if(UseSound) SndFilename = readString(fp);
	fread_endian<float>(fp, BounceCoeff);
	fread_endian<int>(fp, BounceExplode);
	fread_endian<float>(fp, GoThroughSpeed);
	fread_endian<char>(fp, UseOverwriteOwnSpeed);
	fread_endian<char>(fp, UseOverwriteTargetSpeed);
	fread_endian_V<float>(fp, OverwriteOwnSpeed);
	fread_endian_M<float>(fp, ChangeOwnSpeed);
	fread_endian_V<float>(fp, OverwriteTargetSpeed);
	fread_endian_M<float>(fp, ChangeTargetSpeed);
	
	bool haveAddAction = false;
	fread_endian<char>(fp, haveAddAction);
	if(haveAddAction) {
		additionalAction = new Proj_Action();
		return additionalAction->read(gs, fp);
	}
	return true;
}

bool Proj_Action::write(CGameScript* gs, FILE* fp) {
	if(gs->GetHeader()->Version <= GS_LX56_VERSION) {
		errors << "Proj_Action::write called for old GS version" << endl;
		return false;
	}

	fwrite_endian<int>(fp, Type);
	fwrite_endian<char>(fp, Projectiles);
	fwrite_endian<int>(fp, Damage);
	fwrite_endian<int>(fp, Shake);
	fwrite_endian<char>(fp, UseSound);
	if(UseSound) writeString(SndFilename, fp);
	fwrite_endian<float>(fp, BounceCoeff);
	fwrite_endian<int>(fp, BounceExplode);
	fwrite_endian<float>(fp, GoThroughSpeed);
	fwrite_endian<char>(fp, UseOverwriteOwnSpeed);
	fwrite_endian<char>(fp, UseOverwriteTargetSpeed);
	fwrite_endian_V<float>(fp, OverwriteOwnSpeed);
	fwrite_endian_M<float>(fp, ChangeOwnSpeed);
	fwrite_endian_V<float>(fp, OverwriteTargetSpeed);
	fwrite_endian_M<float>(fp, ChangeTargetSpeed);
	
	if(!additionalAction)
		fwrite_endian<char>(fp, 0);
	else {
		fwrite_endian<char>(fp, 1);
		additionalAction->write(gs, fp);
	}
	return true;
}

bool Proj_Event::readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section) {
	if(!ReadKeyword(file, section, "Type", (int*)&type, type)) {
		warnings << file << ":" << section << ": Type attribute isn't set for event" << endl;
		return false;
	}
	if(get() == NULL) {
		warnings << file << ":" << section << ": Type attribute is invalid for event" << endl;
		return false;
	}
	return get()->readFromIni(gs, dir, file, section);
}

bool Proj_Event::read(CGameScript* gs, FILE* fp) {
	fread_endian<int>(fp, (int&)type);
	if(get() == NULL) return false;
	return get()->read(gs, fp);	
}

bool Proj_Event::write(CGameScript* gs, FILE* fp) {
	fwrite_endian<int>(fp, type);
	if(get() == NULL) return false;
	return get()->write(gs, fp);
}

bool Proj_EventAndAction::readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section) {
	std::string eventSection;
	ReadString(file, section, "Event", eventSection, eventSection);
	if(eventSection == "") {
		warnings << file << ":" << section << ": Event not set" << endl;
		return false;
	}

	while(eventSection != "") {
		if(events.size() > 1000) {
			warnings << file << ":" << section << ": Probably there is a loop definition for Event" << endl;
			events.clear();
			return false;
		}
		Proj_Event ev;
		if(!ev.readFromIni(gs, dir, file, eventSection)) return events.size() > 0;
		events.push_back(ev);
		if(!ReadString(file, std::string(eventSection), "AndEvent", eventSection, eventSection)) break;
	}
	
	return Proj_Action::readFromIni(gs, dir, file, section);
}

bool Proj_EventAndAction::read(CGameScript* gs, FILE* fp) {
	Uint32 eventNum = 0;
	fread_endian<Uint32>(fp, eventNum);
	events.resize(eventNum);
	
	for(Uint32 i = 0; i < eventNum; ++i)
		if(!events[i].read(gs, fp)) {
			errors << "Proj_EventAndAction: error while reading game script projectile actions" << endl;
			events.clear(); // would crash otherwise
			return false;
		}
	
	return Proj_Action::read(gs, fp);
}

bool Proj_EventAndAction::write(CGameScript* gs, FILE* fp) {
	fwrite_endian<Uint32>(fp, events.size());
	
	for(Uint32 i = 0; i < events.size(); ++i)
		events[i].write(gs, fp);
	
	return Proj_Action::write(gs, fp);	
}


bool Proj_TimerEvent::readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section) {
	ReadFloat(file, section, "Delay", &Delay, Delay);
	ReadKeyword(file, section, "Repeat", &Repeat, Repeat);
	ReadKeyword(file, section, "UseGlobalTime", &UseGlobalTime, UseGlobalTime);	
	return true;
}

bool Proj_TimerEvent::read(CGameScript* gs, FILE* fp) {
	fread_endian<float>(fp, Delay);
	fread_endian<char>(fp, Repeat);
	fread_endian<char>(fp, UseGlobalTime);
	return true;
}

bool Proj_TimerEvent::write(CGameScript* gs, FILE* fp) {
	fwrite_endian<float>(fp, Delay);
	fwrite_endian<char>(fp, Repeat);
	fwrite_endian<char>(fp, UseGlobalTime);
	return true;
}

bool Proj_ProjHitEvent::readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section) {
	gs->needCollisionInfo = true;
	
	ReadInteger(file, section, "MinHitCount", &MinHitCount, MinHitCount);
	ReadInteger(file, section, "MaxHitCount", &MaxHitCount, MaxHitCount);
	
	if(MaxHitCount >= 0 && MaxHitCount < MinHitCount) {
		warnings << file << ":" << section << ": ignoring MaxHitCount because MaxHitCount < MinHitCount" << endl;
		MaxHitCount = -1;
	}
		
	ReadInteger(file, section, "Width", &Width, Width);
	ReadInteger(file, section, "Height", &Height, Height);	
	
	std::string prjfile;
	ReadString(file, section, "Target", prjfile, "");
	if(prjfile != "")
		Target = gs->CompileProjectile(dir, prjfile);
	else
		Target = NULL;
	return true;
}

bool Proj_ProjHitEvent::read(CGameScript* gs, FILE* fp) {
	if(gs->GetHeader()->Version <= GS_LX56_VERSION) {
		errors << "Proj_ProjHitEvent::read called for old GS version" << endl;
		return false;
	}
	
	gs->needCollisionInfo = true;
	fread_endian<int>(fp, MinHitCount);
	fread_endian<int>(fp, MaxHitCount);
	fread_endian<int>(fp, Width);
	fread_endian<int>(fp, Height);
	
	bool hasSpecificTarget = false;
	fread_endian<char>(fp, hasSpecificTarget);
	if(hasSpecificTarget)
		Target = gs->LoadProjectile(fp);
	else
		Target = NULL;
	return !hasSpecificTarget || Target != NULL;	
}

bool Proj_ProjHitEvent::write(CGameScript* gs, FILE* fp) {
	if(gs->GetHeader()->Version <= GS_LX56_VERSION) {
		errors << "Proj_ProjHitEvent::write called for old GS version" << endl;
		return false;
	}
	
	fwrite_endian<int>(fp, MinHitCount);
	fwrite_endian<int>(fp, MaxHitCount);
	fwrite_endian<int>(fp, Width);
	fwrite_endian<int>(fp, Height);
	
	fwrite_endian<char>(fp, Target != NULL);
	if(Target) return gs->SaveProjectile(Target, fp);
	return true;	
}



bool Proj_WormHitEvent::readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section) {
	ReadKeyword(file, section, "SameWormAsProjOwner", &SameWormAsProjOwner, SameWormAsProjOwner);
	ReadKeyword(file, section, "SameTeamAsProjOwner", &SameTeamAsProjOwner, SameTeamAsProjOwner);
	ReadKeyword(file, section, "DiffWormAsProjOwner", &DiffWormAsProjOwner, DiffWormAsProjOwner);
	ReadKeyword(file, section, "DiffTeamAsProjOwner", &DiffTeamAsProjOwner, DiffTeamAsProjOwner);
	return true;
}

bool Proj_WormHitEvent::read(CGameScript* gs, FILE* fp) {
	fread_endian<char>(fp, SameWormAsProjOwner);
	fread_endian<char>(fp, SameTeamAsProjOwner);
	fread_endian<char>(fp, DiffWormAsProjOwner);
	fread_endian<char>(fp, DiffTeamAsProjOwner);
	return true;
}

bool Proj_WormHitEvent::write(CGameScript* gs, FILE* fp) {
	fwrite_endian<char>(fp, SameWormAsProjOwner);
	fwrite_endian<char>(fp, SameTeamAsProjOwner);
	fwrite_endian<char>(fp, DiffWormAsProjOwner);
	fwrite_endian<char>(fp, DiffTeamAsProjOwner);
	return true;
}



bool Proj_TerrainHitEvent::readFromIni(CGameScript* gs, const std::string& dir, const std::string& file, const std::string& section) {
	ReadKeyword(file, section, "MapBound", &MapBound, MapBound);
	ReadKeyword(file, section, "Dirt", &Dirt, Dirt);
	ReadKeyword(file, section, "Rock", &Rock, Rock);
	return true;
}

bool Proj_TerrainHitEvent::read(CGameScript* gs, FILE* fp) {
	fread_endian<char>(fp, MapBound);
	fread_endian<char>(fp, Dirt);
	fread_endian<char>(fp, Rock);
	return true;
}

bool Proj_TerrainHitEvent::write(CGameScript* gs, FILE* fp) {
	fwrite_endian<char>(fp, MapBound);
	fwrite_endian<char>(fp, Dirt);
	fwrite_endian<char>(fp, Rock);
	return true;
}
