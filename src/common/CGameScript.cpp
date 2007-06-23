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

#include <stdarg.h>

#include "EndianSwap.h"
#include "LieroX.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CGameScript.h"
#include "Error.h"


///////////////////
// Save the script (compiler)
int CGameScript::Save(const std::string& filename)
{
	FILE *fp;
	int n;


	// Open it
	fp = OpenGameFile(filename,"wb");
	if(fp == NULL) {
		printf("Error: Could not open %s for writing\n",filename.c_str());
		return false;
	}

	Header.Version = GS_VERSION;
	strcpy(Header.ID,"Liero Game Script");


	// Header
	gs_header_t tmpheader = Header;
	EndianSwap(tmpheader.Version);
	fwrite(&tmpheader,sizeof(gs_header_t),1,fp);

	fwrite(GetEndianSwapped(NumWeapons),sizeof(int),1,fp);

	// Weapons
	weapon_t *wpn = Weapons;
	
	for(n=0;n<NumWeapons;n++,wpn++) {

        writeString(wpn->Name, fp);
		fwrite(GetEndianSwapped(wpn->Type),          sizeof(int),    1, fp);

		// Special
		if(wpn->Type == WPN_SPECIAL) {
			fwrite(GetEndianSwapped(wpn->Special),   sizeof(int),    1, fp);
			fwrite(GetEndianSwapped(wpn->tSpecial),  sizeof(gs_special_t), 1, fp); // WARNING: this works, because it contains only one int
			fwrite(GetEndianSwapped(wpn->Recharge),  sizeof(float),  1, fp);
			fwrite(GetEndianSwapped(wpn->Drain),     sizeof(float),  1, fp);
			fwrite(GetEndianSwapped(wpn->ROF),       sizeof(float),  1, fp);
			fwrite(GetEndianSwapped(wpn->LaserSight),sizeof(int),    1, fp);
		}

		// Beam
		if(wpn->Type == WPN_BEAM) {
			fwrite(GetEndianSwapped(wpn->Recoil),    sizeof(int),    1, fp);
			fwrite(GetEndianSwapped(wpn->Recharge),  sizeof(float),  1, fp);
			fwrite(GetEndianSwapped(wpn->Drain),     sizeof(float),  1, fp);
			fwrite(GetEndianSwapped(wpn->ROF),       sizeof(float),  1, fp);
			fwrite(GetEndianSwapped(wpn->LaserSight),sizeof(int),    1, fp);

			fwrite(GetEndianSwapped(wpn->Bm_Colour[0]), sizeof(int), 1, fp);
			fwrite(GetEndianSwapped(wpn->Bm_Colour[1]), sizeof(int), 1, fp);
			fwrite(GetEndianSwapped(wpn->Bm_Colour[2]), sizeof(int), 1, fp);
			fwrite(GetEndianSwapped(wpn->Bm_Damage), sizeof(int),    1, fp);
			fwrite(GetEndianSwapped(wpn->Bm_PlyDamage), sizeof(int), 1, fp);
			fwrite(GetEndianSwapped(wpn->Bm_Length), sizeof(int),    1, fp);
		}

		// Projectile
		if(wpn->Type == WPN_PROJECTILE) {
			fwrite(GetEndianSwapped(wpn->Class),     sizeof(int),    1, fp);
			fwrite(GetEndianSwapped(wpn->Recoil),    sizeof(int),    1, fp);
			fwrite(GetEndianSwapped(wpn->Recharge),  sizeof(float),  1, fp);
			fwrite(GetEndianSwapped(wpn->Drain),     sizeof(float),  1, fp);
			fwrite(GetEndianSwapped(wpn->ROF),       sizeof(float),  1, fp);
			fwrite(GetEndianSwapped(wpn->ProjSpeed), sizeof(int),    1, fp);
			fwrite(GetEndianSwapped(wpn->ProjSpeedVar),sizeof(float),1, fp);
			fwrite(GetEndianSwapped(wpn->ProjSpread),sizeof(float),  1, fp);
			fwrite(GetEndianSwapped(wpn->ProjAmount),sizeof(int),    1, fp);
			fwrite(GetEndianSwapped(wpn->LaserSight),sizeof(int),    1, fp);

			fwrite(GetEndianSwapped(wpn->UseSound),  sizeof(int),    1, fp);
			if(wpn->UseSound)
                writeString(wpn->SndFilename, fp);

			// Recursively save the projectile id's
			SaveProjectile(wpn->Projectile,fp);
		}
	}

	// Extra stuff


	// Ninja Rope
	fwrite(GetEndianSwapped(RopeLength),sizeof(int),1,fp);
	fwrite(GetEndianSwapped(RestLength),sizeof(int),1,fp);
	fwrite(GetEndianSwapped(Strength),sizeof(float),1,fp);

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

	return true;
}


///////////////////
// Save a projectile
int CGameScript::SaveProjectile(proj_t *proj, FILE *fp)
{
	if(!proj)
		return false;

	fwrite(GetEndianSwapped(proj->Type),			sizeof(int),1,fp);
	fwrite(GetEndianSwapped(proj->Timer_Time),	sizeof(float),1,fp);
	fwrite(GetEndianSwapped(proj->Timer_TimeVar),sizeof(float),1,fp);
	fwrite(GetEndianSwapped(proj->Trail),		sizeof(int),1,fp);
	fwrite(GetEndianSwapped(proj->UseCustomGravity), sizeof(int), 1, fp);
	
	if(proj->UseCustomGravity)
		fwrite(GetEndianSwapped(proj->Gravity),	sizeof(int), 1, fp);

	fwrite(GetEndianSwapped(proj->Dampening),	sizeof(int), 1, fp);

    // Pixel type
	if(proj->Type == PRJ_PIXEL) {
		fwrite(GetEndianSwapped(proj->NumColours), sizeof(int), 1, fp);
		fwrite(GetEndianSwapped(proj->Colour1[0]),   sizeof(int),1,fp);
		fwrite(GetEndianSwapped(proj->Colour1[1]),   sizeof(int),1,fp);
		fwrite(GetEndianSwapped(proj->Colour1[2]),   sizeof(int),1,fp);
		if(proj->NumColours == 2)
		{
			fwrite(GetEndianSwapped(proj->Colour2[0]),   sizeof(int),1,fp);
			fwrite(GetEndianSwapped(proj->Colour2[1]),   sizeof(int),1,fp);
			fwrite(GetEndianSwapped(proj->Colour2[2]),   sizeof(int),1,fp);
		}
	}

    // Image type
	else if(proj->Type == PRJ_IMAGE) {
        writeString(proj->ImgFilename, fp);
		fwrite(GetEndianSwapped(proj->Rotating), sizeof(int), 1, fp);
		fwrite(GetEndianSwapped(proj->RotIncrement), sizeof(int), 1, fp);
		fwrite(GetEndianSwapped(proj->RotSpeed), sizeof(int),1,fp);
		fwrite(GetEndianSwapped(proj->UseAngle), sizeof(int),1,fp);
		fwrite(GetEndianSwapped(proj->UseSpecAngle),sizeof(int),1,fp);
		if(proj->UseAngle || proj->UseSpecAngle)
			fwrite(GetEndianSwapped(proj->AngleImages),sizeof(int),1,fp);
		fwrite(GetEndianSwapped(proj->Animating),sizeof(int),1,fp);
		if(proj->Animating) {
			fwrite(GetEndianSwapped(proj->AnimRate),sizeof(float),1,fp);
			fwrite(GetEndianSwapped(proj->AnimType),sizeof(int),1,fp);
		}
	}




	//
	// Hit
	//
	fwrite(GetEndianSwapped(proj->Hit_Type),sizeof(int),1,fp);

	// Hit::Explode
	if(proj->Hit_Type == PJ_EXPLODE) {
		fwrite(GetEndianSwapped(proj->Hit_Damage),		sizeof(int),1,fp);
		fwrite(GetEndianSwapped(proj->Hit_Projectiles),	sizeof(int),1,fp);
		fwrite(GetEndianSwapped(proj->Hit_UseSound),		sizeof(int),1,fp);
		fwrite(GetEndianSwapped(proj->Hit_Shake),		sizeof(int),1,fp);
		if(proj->Hit_UseSound)
            writeString(proj->Hit_SndFilename, fp);
	}

	// Hit::Bounce
	if(proj->Hit_Type == PJ_BOUNCE) {
		fwrite(GetEndianSwapped(proj->Hit_BounceCoeff),	sizeof(float),	1,fp);
		fwrite(GetEndianSwapped(proj->Hit_BounceExplode),sizeof(int),	1,fp); 
	}

	// Hit::Carve
	if(proj->Hit_Type == PJ_CARVE) {
		fwrite(GetEndianSwapped(proj->Hit_Damage),		sizeof(int),1,fp);
	}





	//
	// Timer
	//
	if(proj->Timer_Time > 0) {
		fwrite(GetEndianSwapped(proj->Timer_Type),sizeof(int),1,fp);
		if(proj->Timer_Type == PJ_EXPLODE) {
			fwrite(GetEndianSwapped(proj->Timer_Damage),		sizeof(int),1,fp);
			fwrite(GetEndianSwapped(proj->Timer_Projectiles),sizeof(int),1,fp);
			fwrite(GetEndianSwapped(proj->Timer_Shake),sizeof(int),1,fp);
		}
	}
    

	//
	// Player hit
	//
	fwrite(GetEndianSwapped(proj->PlyHit_Type),sizeof(int),1,fp);

	// PlyHit::Explode || PlyHit::Injure
	if(proj->PlyHit_Type == PJ_EXPLODE || proj->PlyHit_Type == PJ_INJURE) {
		fwrite(GetEndianSwapped(proj->PlyHit_Damage),sizeof(int),1,fp);
		fwrite(GetEndianSwapped(proj->PlyHit_Projectiles),sizeof(int),1,fp);
	}

	// PlyHit::Bounce
	if(proj->PlyHit_Type == PJ_BOUNCE) {
		fwrite(GetEndianSwapped(proj->PlyHit_BounceCoeff),sizeof(float),1,fp);
	}


    //
    // Explode
    //
    fwrite(GetEndianSwapped(proj->Exp_Type),     sizeof(int), 1, fp);
    fwrite(GetEndianSwapped(proj->Exp_Damage),   sizeof(int), 1, fp);
    fwrite(GetEndianSwapped(proj->Exp_Projectiles), sizeof(int), 1, fp);
    fwrite(GetEndianSwapped(proj->Exp_UseSound), sizeof(int), 1, fp);
    if(proj->Exp_UseSound)
        writeString(proj->Exp_SndFilename, fp);


    //
    // Touch
    //
    fwrite(GetEndianSwapped(proj->Tch_Type),     sizeof(int), 1, fp);
    fwrite(GetEndianSwapped(proj->Tch_Damage),   sizeof(int), 1, fp);
    fwrite(GetEndianSwapped(proj->Tch_Projectiles), sizeof(int), 1, fp);
    fwrite(GetEndianSwapped(proj->Tch_UseSound), sizeof(int), 1, fp);
    if(proj->Tch_UseSound)
        writeString(proj->Tch_SndFilename, fp);
        


	if(proj->Timer_Projectiles || proj->Hit_Projectiles || proj->PlyHit_Projectiles || proj->Exp_Projectiles ||
       proj->Tch_Projectiles) {
		fwrite(GetEndianSwapped(proj->ProjUseangle),	sizeof(int),	1, fp);
		fwrite(GetEndianSwapped(proj->ProjAngle),	sizeof(int),	1, fp);
		fwrite(GetEndianSwapped(proj->ProjAmount),	sizeof(int),	1, fp);
		fwrite(GetEndianSwapped(proj->ProjSpread),	sizeof(float),	1, fp);
		fwrite(GetEndianSwapped(proj->ProjSpeed),	sizeof(int),	1, fp);
		fwrite(GetEndianSwapped(proj->ProjSpeedVar),	sizeof(float),	1, fp);

		SaveProjectile(proj->Projectile,fp);
	}

	
	// Projectile trail
	if(proj->Trail == TRL_PROJECTILE) {
		
		fwrite(GetEndianSwapped(proj->PrjTrl_UsePrjVelocity),	sizeof(int),	1, fp);
		fwrite(GetEndianSwapped(proj->PrjTrl_Delay),				sizeof(float),	1, fp);
		fwrite(GetEndianSwapped(proj->PrjTrl_Amount),			sizeof(int),	1, fp);
		fwrite(GetEndianSwapped(proj->PrjTrl_Speed),				sizeof(int),	1, fp);
		fwrite(GetEndianSwapped(proj->PrjTrl_SpeedVar),			sizeof(float),	1, fp);
		fwrite(GetEndianSwapped(proj->PrjTrl_Spread),			sizeof(float),	1, fp);

		SaveProjectile(proj->PrjTrl_Proj, fp);
	}

	return true;
}


#ifndef _CONSOLE

///////////////////
// Load the game script from a file (game)
int CGameScript::Load(const std::string& dir)
{
	
	FILE *fp;
	int n;	
	static std::string filename;

	filename = dir + "/script.lgs";
	sDirectory = dir;

	// Open it
	fp = OpenGameFile(filename,"rb");
	if(fp == NULL) {
		SetError("CGameScript::Load(): Could not load file %s",filename.c_str());
		return GSE_FILE;
	}

	// Header
	fread(&Header,sizeof(gs_header_t),1,fp);
	EndianSwap(Header.Version);
	
	// Check ID
	if(strcmp(Header.ID,"Liero Game Script") != 0) {
		fclose(fp);
		SetError("CGameScript::Load(): Bad script id");
		return GSE_BAD;
	}

	// Check version
	if(Header.Version != GS_VERSION) {
		fclose(fp);
		SetError("CGameScript::Load(): Bad script version");
		return GSE_VERSION;
	}

    // Clear an old mod file
    modLog("Loading game mod file %s",filename.c_str());
	//modLog("  ID = %s", Header.ID);
	//modLog("  Version = %i", Header.Version);

	fread(&NumWeapons,sizeof(int),1,fp);
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
		wpn->Projectile = NULL;

        wpn->Name = readString(fp);
		fread(&wpn->Type,           sizeof(int),    1,fp);
		EndianSwap(wpn->Type);
		
		// Special
		if(wpn->Type == WPN_SPECIAL) {
			fread(&wpn->Special,    sizeof(int),    1, fp);
			EndianSwap(wpn->Special);
			fread(&wpn->tSpecial,   sizeof(gs_special_t), 1, fp);
			EndianSwap(wpn->tSpecial.Thrust);
			fread(&wpn->Recharge,   sizeof(float),  1, fp);
			EndianSwap(wpn->Recharge);
			fread(&wpn->Drain,      sizeof(float),  1, fp);
			EndianSwap(wpn->Drain);
			fread(&wpn->ROF,        sizeof(float),  1, fp);
			EndianSwap(wpn->ROF);
			fread(&wpn->LaserSight, sizeof(int),    1, fp);
			EndianSwap(wpn->LaserSight);
		}

		// Beam
		if(wpn->Type == WPN_BEAM) {
			fread(&wpn->Recoil,     sizeof(int),    1, fp);
			EndianSwap(wpn->Recoil);
			fread(&wpn->Recharge,   sizeof(float),  1, fp);
			EndianSwap(wpn->Recharge);
			fread(&wpn->Drain,      sizeof(float),  1, fp);
			EndianSwap(wpn->Drain);
			fread(&wpn->ROF,        sizeof(float),  1, fp);
			EndianSwap(wpn->ROF);
			fread(&wpn->LaserSight, sizeof(int),    1, fp);
			EndianSwap(wpn->LaserSight);

			fread(wpn->Bm_Colour, sizeof(int),      3, fp);
			EndianSwap(wpn->Bm_Colour[0]);
			EndianSwap(wpn->Bm_Colour[1]);
			EndianSwap(wpn->Bm_Colour[2]);
			fread(&wpn->Bm_Damage, sizeof(int),     1, fp);
			EndianSwap(wpn->Bm_Damage);
			fread(&wpn->Bm_PlyDamage, sizeof(int),  1, fp);
			EndianSwap(wpn->Bm_PlyDamage);
			fread(&wpn->Bm_Length, sizeof(int),     1, fp);
			EndianSwap(wpn->Bm_Length);
		}

		// Projectile
		if(wpn->Type == WPN_PROJECTILE) {

			fread(&wpn->Class,sizeof(int),1,fp);
			EndianSwap(wpn->Class);
			fread(&wpn->Recoil,sizeof(int),1,fp);
			EndianSwap(wpn->Recoil);
			fread(&wpn->Recharge,sizeof(float),1,fp);
			EndianSwap(wpn->Recharge);
			fread(&wpn->Drain,sizeof(float),1,fp);
			EndianSwap(wpn->Drain);
			fread(&wpn->ROF,sizeof(float),1,fp);
			EndianSwap(wpn->ROF);
			fread(&wpn->ProjSpeed,sizeof(int),1,fp);
			EndianSwap(wpn->ProjSpeed);
			fread(&wpn->ProjSpeedVar,sizeof(float),1,fp);
			EndianSwap(wpn->ProjSpeedVar);
			fread(&wpn->ProjSpread,sizeof(float),1,fp);
			EndianSwap(wpn->ProjSpread);
			fread(&wpn->ProjAmount,sizeof(int),1,fp);
			EndianSwap(wpn->ProjAmount);
			fread(&wpn->LaserSight, sizeof(int), 1, fp);
			EndianSwap(wpn->LaserSight);

			fread(&wpn->UseSound,sizeof(int),1,fp);
			EndianSwap(wpn->UseSound);
			if(wpn->UseSound) {
                wpn->SndFilename = readString(fp);

				// Load the sample
				wpn->smpSample = LoadGSSample(dir,wpn->SndFilename);
				if(wpn->smpSample == 0)
					wpn->UseSound = false;
			}
		
			wpn->Projectile = LoadProjectile(fp);
		}

		wpn->ROF /= 1000.0f;
		wpn->Recharge /= 10.0f;
	}


	// Extra stuff


	// Ninja Rope
	fread(&RopeLength,sizeof(int),1,fp);
	EndianSwap(RopeLength);
	fread(&RestLength,sizeof(int),1,fp);
	EndianSwap(RestLength);
	fread(&Strength,sizeof(float),1,fp);
	EndianSwap(Strength);

	// Worm
	fread(&Worm, sizeof(gs_worm_t), 1, fp);
	EndianSwap(Worm.AngleSpeed);
	EndianSwap(Worm.GroundSpeed);
	EndianSwap(Worm.AirSpeed);
	EndianSwap(Worm.Gravity);
	EndianSwap(Worm.JumpForce);
	EndianSwap(Worm.AirFriction);
	EndianSwap(Worm.GroundFriction);


	fclose(fp);





	return GSE_OK;
}


///////////////////
// Load a projectile
proj_t *CGameScript::LoadProjectile(FILE *fp)
{
	proj_t *proj = new proj_t;
	if(proj == NULL)
		return NULL;
	

	fread(&proj->Type,			sizeof(int),  1,fp);
	EndianSwap(proj->Type);
	fread(&proj->Timer_Time,	sizeof(float),1,fp);
	EndianSwap(proj->Timer_Time);
	fread(&proj->Timer_TimeVar,	sizeof(float),1,fp);
	EndianSwap(proj->Timer_TimeVar);
	fread(&proj->Trail,			sizeof(int),  1,fp);
	EndianSwap(proj->Trail);
	fread(&proj->UseCustomGravity, sizeof(int), 1, fp);
	EndianSwap(proj->UseCustomGravity);
	if(proj->UseCustomGravity)
	{	
		fread(&proj->Gravity,	sizeof(int), 1, fp);
		EndianSwap(proj->Gravity);
	}	
	fread(&proj->Dampening,		sizeof(int),  1, fp);
	EndianSwap(proj->Dampening);

	proj->PrjTrl_Proj = NULL;
	proj->Projectile = NULL;
	proj->Hit_Projectiles = false;
	proj->Timer_Projectiles = false;
	proj->PlyHit_Projectiles = false;
	proj->Animating = false;
	proj->Rotating = 0;
	proj->RotIncrement = 0;
	proj->Timer_Shake = 0;
	
	if(proj->Type == PRJ_PIXEL) {
		fread(&proj->NumColours, sizeof(int), 1, fp);
		EndianSwap(proj->NumColours);
		fread(proj->Colour1,sizeof(int),3,fp);
		EndianSwap(proj->Colour1[0]);
		EndianSwap(proj->Colour1[1]);
		EndianSwap(proj->Colour1[2]);
		if(proj->NumColours == 2)
		{
			fread(proj->Colour2,sizeof(int),3,fp);
			EndianSwap(proj->Colour2[0]);
			EndianSwap(proj->Colour2[1]);
			EndianSwap(proj->Colour2[2]);
		}

	}
	else if(proj->Type == PRJ_IMAGE) {
        proj->ImgFilename = readString(fp);
		
		proj->bmpImage = LoadGSImage(sDirectory, proj->ImgFilename);
        if(!proj->bmpImage)
            modLog("Could not open image '%s'",proj->ImgFilename.c_str());
		
		// Set the colour key
		if(proj->bmpImage)
			SetColorKey(proj->bmpImage);

		fread(&proj->Rotating, sizeof(int), 1, fp);
		EndianSwap(proj->Rotating);
		fread(&proj->RotIncrement, sizeof(int), 1, fp);
		EndianSwap(proj->RotIncrement);
		fread(&proj->RotSpeed,sizeof(int),1,fp);
		EndianSwap(proj->RotSpeed);
		fread(&proj->UseAngle,sizeof(int),1,fp);
		EndianSwap(proj->UseAngle);
		fread(&proj->UseSpecAngle,sizeof(int),1,fp);
		EndianSwap(proj->UseSpecAngle);
		if(proj->UseAngle || proj->UseSpecAngle)
		{
			fread(&proj->AngleImages,sizeof(int),1,fp);
			EndianSwap(proj->AngleImages);
		}
		fread(&proj->Animating,sizeof(int),1,fp);
		EndianSwap(proj->Animating);
		if(proj->Animating) {
			fread(&proj->AnimRate,sizeof(float),1,fp);
			EndianSwap(proj->AnimRate);
			fread(&proj->AnimType,sizeof(int),1,fp);
			EndianSwap(proj->AnimType);
		}
	}


	//
	// Hit
	//
	fread(&proj->Hit_Type,sizeof(int),1,fp);
	EndianSwap(proj->Hit_Type);

	// Hit::Explode
	if(proj->Hit_Type == PJ_EXPLODE) {
		fread(&proj->Hit_Damage,		sizeof(int),1,fp);
		EndianSwap(proj->Hit_Damage);
		fread(&proj->Hit_Projectiles,	sizeof(int),1,fp);
		EndianSwap(proj->Hit_Projectiles);
		fread(&proj->Hit_UseSound,		sizeof(int),1,fp);
		EndianSwap(proj->Hit_UseSound);
		fread(&proj->Hit_Shake,			sizeof(int),1,fp);
		EndianSwap(proj->Hit_Shake);

		if(proj->Hit_UseSound) {
            proj->Hit_SndFilename = readString(fp);			
			
			// Load the sample
			proj->smpSample = LoadGSSample(sDirectory,proj->Hit_SndFilename);
			
            if(proj->smpSample == 0) {
				proj->Hit_UseSound = false;
				modLog("Could not open sound '%s'",proj->Hit_SndFilename.c_str());
            }
		}
	}

	// Hit::Bounce
	if(proj->Hit_Type == PJ_BOUNCE) {
		fread(&proj->Hit_BounceCoeff,	sizeof(float), 1, fp);
		EndianSwap(proj->Hit_BounceCoeff);
		fread(&proj->Hit_BounceExplode, sizeof(int),   1, fp);
		EndianSwap(proj->Hit_BounceExplode);
	}

	// Hit::Carve
	if(proj->Hit_Type == PJ_CARVE) {
		fread(&proj->Hit_Damage,		sizeof(int),1,fp);
		EndianSwap(proj->Hit_Damage);
	}





	//
	// Timer
	//
	if(proj->Timer_Time > 0) {
		fread(&proj->Timer_Type,sizeof(int),1,fp);
		EndianSwap(proj->Timer_Type);
		if(proj->Timer_Type == PJ_EXPLODE) {
			fread(&proj->Timer_Damage,sizeof(int),1,fp);
			EndianSwap(proj->Timer_Damage);
			fread(&proj->Timer_Projectiles,sizeof(int),1,fp);
			EndianSwap(proj->Timer_Projectiles);
			fread(&proj->Timer_Shake,sizeof(int),1,fp);
			EndianSwap(proj->Timer_Shake);
		}
	}



	//
	// Player hit
	//
	fread(&proj->PlyHit_Type,sizeof(int),1,fp);
	EndianSwap(proj->PlyHit_Type);

	// PlyHit::Explode || PlyHit::Injure
	if(proj->PlyHit_Type == PJ_INJURE || proj->PlyHit_Type == PJ_EXPLODE) {
		fread(&proj->PlyHit_Damage,sizeof(int),1,fp);
		EndianSwap(proj->PlyHit_Damage);
		fread(&proj->PlyHit_Projectiles,sizeof(int),1,fp);
		EndianSwap(proj->PlyHit_Projectiles);
	}

	// PlyHit::Bounce
	if(proj->PlyHit_Type == PJ_BOUNCE) {
		fread(&proj->PlyHit_BounceCoeff, sizeof(float), 1, fp);
		EndianSwap(proj->PlyHit_BounceCoeff);
	}


    //
    // Explode
    //
    fread(&proj->Exp_Type,     sizeof(int), 1, fp);
	EndianSwap(proj->Exp_Type);
    fread(&proj->Exp_Damage,   sizeof(int), 1, fp);
	EndianSwap(proj->Exp_Damage);
    fread(&proj->Exp_Projectiles, sizeof(int), 1, fp);
	EndianSwap(proj->Exp_Projectiles);
    fread(&proj->Exp_UseSound, sizeof(int), 1, fp);
	EndianSwap(proj->Exp_UseSound);
    if(proj->Exp_UseSound)
    {
        proj->Exp_SndFilename = readString(fp);
	}

    //
    // Touch
    //
    fread(&proj->Tch_Type,     sizeof(int), 1, fp);
	EndianSwap(proj->Tch_Type);
    fread(&proj->Tch_Damage,   sizeof(int), 1, fp);
	EndianSwap(proj->Tch_Damage);
    fread(&proj->Tch_Projectiles, sizeof(int), 1, fp);
	EndianSwap(proj->Tch_Projectiles);
    fread(&proj->Tch_UseSound, sizeof(int), 1, fp);
	EndianSwap(proj->Tch_UseSound);
    if(proj->Tch_UseSound)
        proj->Tch_SndFilename = readString(fp);


	if(proj->Timer_Projectiles || proj->Hit_Projectiles || proj->PlyHit_Projectiles || proj->Exp_Projectiles ||
       proj->Tch_Projectiles) {
		fread(&proj->ProjUseangle,	sizeof(int),	1, fp);
		EndianSwap(proj->ProjUseangle);
		fread(&proj->ProjAngle,		sizeof(int),	1, fp);
		EndianSwap(proj->ProjAngle);
		fread(&proj->ProjAmount,	sizeof(int),	1, fp);
		EndianSwap(proj->ProjAmount);
		fread(&proj->ProjSpread,	sizeof(float),	1, fp);
		EndianSwap(proj->ProjSpread);
		fread(&proj->ProjSpeed,		sizeof(int),	1, fp);
		EndianSwap(proj->ProjSpeed);
		fread(&proj->ProjSpeedVar,	sizeof(float),	1, fp);
		EndianSwap(proj->ProjSpeedVar);

		proj->Projectile = LoadProjectile(fp);
	}

	// Projectile trail
	if(proj->Trail == TRL_PROJECTILE) {
		
		fread(&proj->PrjTrl_UsePrjVelocity,	sizeof(int),	1, fp);
		EndianSwap(proj->PrjTrl_UsePrjVelocity);
		fread(&proj->PrjTrl_Delay,			sizeof(float),	1, fp);
		EndianSwap(proj->PrjTrl_Delay);
		fread(&proj->PrjTrl_Amount,			sizeof(int),	1, fp);
		EndianSwap(proj->PrjTrl_Amount);
		fread(&proj->PrjTrl_Speed,			sizeof(int),	1, fp);
		EndianSwap(proj->PrjTrl_Speed);
		fread(&proj->PrjTrl_SpeedVar,		sizeof(float),	1, fp);
		EndianSwap(proj->PrjTrl_SpeedVar);
		fread(&proj->PrjTrl_Spread,			sizeof(float),	1, fp);
		EndianSwap(proj->PrjTrl_Spread);

		// Change from milli-seconds to seconds
		proj->PrjTrl_Delay /= 1000.0f;

		proj->PrjTrl_Proj = LoadProjectile(fp);
	}

	return proj;
}


///////////////////
// Load an image
SDL_Surface *CGameScript::LoadGSImage(const std::string& dir, const std::string& filename)
{
	SDL_Surface *img = NULL;
	static std::string buf;

	// First, check the gfx directory in the mod dir
	buf = dir + "/gfx/" + filename;
	img = LoadImage(buf);

	if(img)
		return img;

	// Check the gfx directory in the data dir
	buf = std::string("data/gfx/") + filename;
	return LoadImage(buf);
}


///////////////////
// Load a sample
SoundSample* CGameScript::LoadGSSample(const std::string& dir, const std::string& filename)
{
	SoundSample* smp = NULL;
	static std::string buf;

	// First, check the sfx directory in the mod dir
	buf = dir + "/sfx/" + filename;
	smp = LoadSample(buf,10);
	
	if(smp)
		return smp;

	// Check the sounds directory in the data dir
	buf = "data/sounds/" + filename;
	return LoadSample(buf,10);
}

#endif  //  _CONSOLE



///////////////////
// Find a weapon based on its name
weapon_t *CGameScript::FindWeapon(const std::string& name)
{
	int n;

	// Go through each weapon
	weapon_t *wpn = Weapons;
	for(n=0;n<NumWeapons;n++,wpn++) {

		if(stringcasecmp(wpn->Name, name) == 0)
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


///////////////////
// Write a string in pascal format
void CGameScript::writeString(const std::string& szString, FILE *fp)
{
    if(szString == "") return;

	size_t length = szString.size();
	if(length > 255) {	
		printf("WARNING: i will cut the following string for writing: %s\n", szString.c_str());
		length = 255;
	}
	uchar len = (uchar)length;
	
    fwrite( &len, sizeof(uchar), 1, fp );
    fwrite( szString.c_str(),sizeof(char), length, fp );
}


///////////////////
// Read a string in pascal format
std::string CGameScript::readString(FILE *fp)
{
	static char buf[256];

    uchar length;
    fread( &length, sizeof(uchar), 1, fp );
    fread( buf,sizeof(char), length, fp );

    buf[length] = '\0';
	
    return buf;
}


///////////////////
// Shutdown the game script
void CGameScript::Shutdown(void)
{
	int n;

    // Close the log file
    if(pModLog) {
        fclose(pModLog);
        pModLog = NULL;
    }

	if(Weapons == NULL)
		return;

	// Go through each weapon
	weapon_t *wpn;
	for(n=0;n<NumWeapons;n++) {
		wpn = &Weapons[n];
		// Shutdown any projectiles
		ShutdownProjectile(wpn->Projectile);
	}

	delete[] Weapons;
	Weapons = NULL;
}


///////////////////
// Shutdown a projectile
void CGameScript::ShutdownProjectile(proj_t *prj)
{
	if(prj) {
		ShutdownProjectile(prj->Projectile);
		ShutdownProjectile(prj->PrjTrl_Proj);
		delete prj;
	}
}


///////////////////
// Check if a file is a valid liero game script
int CGameScript::CheckFile(const std::string& dir, std::string& name)
{
	name = "";
	std::string filename = dir + "/script.lgs";
	
	// Open it
	FILE *fp = OpenGameFile(filename,"rb");
	if(fp == NULL)
		return false;

	// Header
	gs_header_t head;
	memset(&head,0,sizeof(gs_header_t));
	fread(&head,sizeof(gs_header_t),1,fp);
	EndianSwap(head.Version);
	fclose(fp);

	// Check ID
	if(strcmp(head.ID,"Liero Game Script") != 0)
		return false;

	// Check version
	if(head.Version != GS_VERSION)
		return false;

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
void CGameScript::modLog(char *fmt, ...)
{
    static char    buf[1024];

    va_list	va;

	va_start(va,fmt);
	vsnprintf(buf,sizeof(buf),fmt,va);
	fix_markend(buf);
	va_end(va);

	printf("%s\n", buf);
	
	if(!pModLog) {
		pModLog = OpenGameFile("modlog.txt","wt");
		if(!pModLog)
			return;
		fprintf(pModLog,"Log file for mod:\n%s\n--------------------------------\n",Header.ModName);
	}
	
	fprintf(pModLog,"%s\n",buf);
}
