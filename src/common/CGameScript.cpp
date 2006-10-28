/////////////////////////////////////////
//
//                  LieroX
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Game script class
// Created 7/2/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"


///////////////////
// Save the script (compiler)
int CGameScript::Save(char *filename)
{
	FILE *fp;
	int n;


	// Open it
	fp = fopen_i(filename,"wb");
	if(fp == NULL) {
		printf("Error: Could not open %s for writing\n",filename);
		return false;
	}

	Header.Version = GS_VERSION;
	strcpy(Header.ID,"Liero Game Script");


	// Header
	fwrite(&Header,sizeof(Header),1,fp);

	fwrite(&NumWeapons,sizeof(int),1,fp);

	// Weapons
	weapon_t *wpn = Weapons;
	
	for(n=0;n<NumWeapons;n++,wpn++) {

        writeString(wpn->Name, fp);
		fwrite(&wpn->Type,          sizeof(int),    1, fp);

		// Special
		if(wpn->Type == WPN_SPECIAL) {
			fwrite(&wpn->Special,   sizeof(int),    1, fp);
			fwrite(&wpn->tSpecial,  sizeof(wpn->tSpecial), 1, fp);
			fwrite(&wpn->Recharge,  sizeof(float),  1, fp);
			fwrite(&wpn->Drain,     sizeof(float),  1, fp);
			fwrite(&wpn->ROF,       sizeof(float),  1, fp);
			fwrite(&wpn->LaserSight,sizeof(int),    1, fp);
		}

		// Beam
		if(wpn->Type == WPN_BEAM) {
			fwrite(&wpn->Recoil,    sizeof(int),    1, fp);
			fwrite(&wpn->Recharge,  sizeof(float),  1, fp);
			fwrite(&wpn->Drain,     sizeof(float),  1, fp);
			fwrite(&wpn->ROF,       sizeof(float),  1, fp);
			fwrite(&wpn->LaserSight,sizeof(int),    1, fp);

			fwrite(wpn->Bm_Colour,  sizeof(int),    3, fp);
			fwrite(&wpn->Bm_Damage, sizeof(int),    1, fp);
			fwrite(&wpn->Bm_PlyDamage, sizeof(int), 1, fp);
			fwrite(&wpn->Bm_Length, sizeof(int),    1, fp);
		}

		// Projectile
		if(wpn->Type == WPN_PROJECTILE) {
			fwrite(&wpn->Class,     sizeof(int),    1, fp);
			fwrite(&wpn->Recoil,    sizeof(int),    1, fp);
			fwrite(&wpn->Recharge,  sizeof(float),  1, fp);
			fwrite(&wpn->Drain,     sizeof(float),  1, fp);
			fwrite(&wpn->ROF,       sizeof(float),  1, fp);
			fwrite(&wpn->ProjSpeed, sizeof(int),    1, fp);
			fwrite(&wpn->ProjSpeedVar,sizeof(float),1, fp);
			fwrite(&wpn->ProjSpread,sizeof(float),  1, fp);
			fwrite(&wpn->ProjAmount,sizeof(int),    1, fp);
			fwrite(&wpn->LaserSight,sizeof(int),    1, fp);

			fwrite(&wpn->UseSound,  sizeof(int),    1, fp);
			if(wpn->UseSound)
                writeString(wpn->SndFilename, fp);

			// Recursively save the projectile id's
			SaveProjectile(wpn->Projectile,fp);
		}
	}

	// Extra stuff


	// Ninja Rope
	fwrite(&RopeLength,sizeof(int),1,fp);
	fwrite(&RestLength,sizeof(int),1,fp);
	fwrite(&Strength,sizeof(float),1,fp);

	// Worm
	fwrite(&Worm, sizeof(gs_worm_t), 1, fp);


	fclose(fp);

	return true;
}


///////////////////
// Save a projectile
int CGameScript::SaveProjectile(proj_t *proj, FILE *fp)
{
	if(!proj)
		return false;

	fwrite(&proj->Type,			sizeof(int),1,fp);
	fwrite(&proj->Timer_Time,	sizeof(float),1,fp);
	fwrite(&proj->Timer_TimeVar,sizeof(float),1,fp);
	fwrite(&proj->Trail,		sizeof(int),1,fp);
	fwrite(&proj->UseCustomGravity, sizeof(int), 1, fp);
	
	if(proj->UseCustomGravity)
		fwrite(&proj->Gravity,	sizeof(int), 1, fp);

	fwrite(&proj->Dampening,	sizeof(int), 1, fp);

    // Pixel type
	if(proj->Type == PRJ_PIXEL) {
		fwrite(&proj->NumColours, sizeof(int), 1, fp);
		fwrite(proj->Colour1,   sizeof(int),3,fp);
		if(proj->NumColours == 2)
			fwrite(proj->Colour2,sizeof(int),3,fp);
	}

    // Image type
	else if(proj->Type == PRJ_IMAGE) {
        writeString(proj->ImgFilename, fp);
		fwrite(&proj->Rotating, sizeof(int), 1, fp);
		fwrite(&proj->RotIncrement, sizeof(int), 1, fp);
		fwrite(&proj->RotSpeed, sizeof(int),1,fp);
		fwrite(&proj->UseAngle, sizeof(int),1,fp);
		fwrite(&proj->UseSpecAngle,sizeof(int),1,fp);
		if(proj->UseAngle || proj->UseSpecAngle)
			fwrite(&proj->AngleImages,sizeof(int),1,fp);
		fwrite(&proj->Animating,sizeof(int),1,fp);
		if(proj->Animating) {
			fwrite(&proj->AnimRate,sizeof(float),1,fp);
			fwrite(&proj->AnimType,sizeof(int),1,fp);
		}
	}




	//
	// Hit
	//
	fwrite(&proj->Hit_Type,sizeof(int),1,fp);

	// Hit::Explode
	if(proj->Hit_Type == PJ_EXPLODE) {
		fwrite(&proj->Hit_Damage,		sizeof(int),1,fp);
		fwrite(&proj->Hit_Projectiles,	sizeof(int),1,fp);
		fwrite(&proj->Hit_UseSound,		sizeof(int),1,fp);
		fwrite(&proj->Hit_Shake,		sizeof(int),1,fp);
		if(proj->Hit_UseSound)
            writeString(proj->Hit_SndFilename, fp);
	}

	// Hit::Bounce
	if(proj->Hit_Type == PJ_BOUNCE) {
		fwrite(&proj->Hit_BounceCoeff,	sizeof(float),	1,fp);
		fwrite(&proj->Hit_BounceExplode,sizeof(int),	1,fp); 
	}

	// Hit::Carve
	if(proj->Hit_Type == PJ_CARVE) {
		fwrite(&proj->Hit_Damage,		sizeof(int),1,fp);
	}





	//
	// Timer
	//
	if(proj->Timer_Time > 0) {
		fwrite(&proj->Timer_Type,sizeof(int),1,fp);
		if(proj->Timer_Type == PJ_EXPLODE) {
			fwrite(&proj->Timer_Damage,		sizeof(int),1,fp);
			fwrite(&proj->Timer_Projectiles,sizeof(int),1,fp);
			fwrite(&proj->Timer_Shake,sizeof(int),1,fp);
		}
	}
    

	//
	// Player hit
	//
	fwrite(&proj->PlyHit_Type,sizeof(int),1,fp);

	// PlyHit::Explode || PlyHit::Injure
	if(proj->PlyHit_Type == PJ_EXPLODE || proj->PlyHit_Type == PJ_INJURE) {
		fwrite(&proj->PlyHit_Damage,sizeof(int),1,fp);
		fwrite(&proj->PlyHit_Projectiles,sizeof(int),1,fp);
	}

	// PlyHit::Bounce
	if(proj->PlyHit_Type == PJ_BOUNCE) {
		fwrite(&proj->PlyHit_BounceCoeff,sizeof(float),1,fp);
	}


    //
    // Explode
    //
    fwrite(&proj->Exp_Type,     sizeof(int), 1, fp);
    fwrite(&proj->Exp_Damage,   sizeof(int), 1, fp);
    fwrite(&proj->Exp_Projectiles, sizeof(int), 1, fp);
    fwrite(&proj->Exp_UseSound, sizeof(int), 1, fp);
    if(proj->Exp_UseSound)
        writeString(proj->Exp_SndFilename, fp);


    //
    // Touch
    //
    fwrite(&proj->Tch_Type,     sizeof(int), 1, fp);
    fwrite(&proj->Tch_Damage,   sizeof(int), 1, fp);
    fwrite(&proj->Tch_Projectiles, sizeof(int), 1, fp);
    fwrite(&proj->Tch_UseSound, sizeof(int), 1, fp);
    if(proj->Tch_UseSound)
        writeString(proj->Tch_SndFilename, fp);
        


	if(proj->Timer_Projectiles || proj->Hit_Projectiles || proj->PlyHit_Projectiles || proj->Exp_Projectiles ||
       proj->Tch_Projectiles) {
		fwrite(&proj->ProjUseangle,	sizeof(int),	1, fp);
		fwrite(&proj->ProjAngle,	sizeof(int),	1, fp);
		fwrite(&proj->ProjAmount,	sizeof(int),	1, fp);
		fwrite(&proj->ProjSpread,	sizeof(float),	1, fp);
		fwrite(&proj->ProjSpeed,	sizeof(int),	1, fp);
		fwrite(&proj->ProjSpeedVar,	sizeof(float),	1, fp);

		SaveProjectile(proj->Projectile,fp);
	}

	
	// Projectile trail
	if(proj->Trail == TRL_PROJECTILE) {
		
		fwrite(&proj->PrjTrl_UsePrjVelocity,	sizeof(int),	1, fp);
		fwrite(&proj->PrjTrl_Delay,				sizeof(float),	1, fp);
		fwrite(&proj->PrjTrl_Amount,			sizeof(int),	1, fp);
		fwrite(&proj->PrjTrl_Speed,				sizeof(int),	1, fp);
		fwrite(&proj->PrjTrl_SpeedVar,			sizeof(float),	1, fp);
		fwrite(&proj->PrjTrl_Spread,			sizeof(float),	1, fp);

		SaveProjectile(proj->PrjTrl_Proj, fp);
	}

	return true;
}


#ifndef _CONSOLE

///////////////////
// Load the game script from a file (game)
int CGameScript::Load(char *dir)
{
	
	FILE *fp;
	int n;	
	char filename[128];

	sprintf(filename,"%s/script.lgs",dir);
	strcpy(sDirectory, dir);

	// Open it
	fp = fopen_i(filename,"rb");
	if(fp == NULL) {
		SetError("CGameScript::Load(): Could not load file %s",filename);
		return GSE_FILE;
	}

	// Header
	fread(&Header,sizeof(Header),1,fp);

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
    modLog("Loading %s\n",filename);


	fread(&NumWeapons,sizeof(int),1,fp);


	// Do Allocations
	Weapons = new weapon_t[NumWeapons];
	if(Weapons == NULL) {
		SetError("SGameScript::Load(): Out of memory");
		return GSE_MEM;
	}

	// Weapons
	weapon_t *wpn = Weapons;
	
	for(n=0;n<NumWeapons;n++,wpn++) {

		wpn->ID = n;
		wpn->Projectile = NULL;

        readString(wpn->Name, fp);
		fread(&wpn->Type,           sizeof(int),    1,fp);

		// Special
		if(wpn->Type == WPN_SPECIAL) {
			fread(&wpn->Special,    sizeof(int),    1, fp);
			fread(&wpn->tSpecial,   sizeof(wpn->tSpecial), 1, fp);
			fread(&wpn->Recharge,   sizeof(float),  1, fp);
			fread(&wpn->Drain,      sizeof(float),  1, fp);
			fread(&wpn->ROF,        sizeof(float),  1, fp);
			fread(&wpn->LaserSight, sizeof(int),    1, fp);
		}

		// Beam
		if(wpn->Type == WPN_BEAM) {
			fread(&wpn->Recoil,     sizeof(int),    1, fp);
			fread(&wpn->Recharge,   sizeof(float),  1, fp);
			fread(&wpn->Drain,      sizeof(float),  1, fp);
			fread(&wpn->ROF,        sizeof(float),  1, fp);
			fread(&wpn->LaserSight, sizeof(int),    1, fp);

			fread(wpn->Bm_Colour, sizeof(int),      3, fp);
			fread(&wpn->Bm_Damage, sizeof(int),     1, fp);
			fread(&wpn->Bm_PlyDamage, sizeof(int),  1, fp);
			fread(&wpn->Bm_Length, sizeof(int),     1, fp);
		}

		// Projectile
		if(wpn->Type == WPN_PROJECTILE) {

			fread(&wpn->Class,sizeof(int),1,fp);
			fread(&wpn->Recoil,sizeof(int),1,fp);
			fread(&wpn->Recharge,sizeof(float),1,fp);
			fread(&wpn->Drain,sizeof(float),1,fp);
			fread(&wpn->ROF,sizeof(float),1,fp);
			fread(&wpn->ProjSpeed,sizeof(int),1,fp);
			fread(&wpn->ProjSpeedVar,sizeof(float),1,fp);
			fread(&wpn->ProjSpread,sizeof(float),1,fp);
			fread(&wpn->ProjAmount,sizeof(int),1,fp);
			fread(&wpn->LaserSight, sizeof(int), 1, fp);

			fread(&wpn->UseSound,sizeof(int),1,fp);
			if(wpn->UseSound) {
                readString(wpn->SndFilename, fp);

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
	fread(&RestLength,sizeof(int),1,fp);
	fread(&Strength,sizeof(float),1,fp);

	// Worm
	fread(&Worm, sizeof(gs_worm_t), 1, fp);


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
	fread(&proj->Timer_Time,	sizeof(float),1,fp);
	fread(&proj->Timer_TimeVar,	sizeof(float),1,fp);
	fread(&proj->Trail,			sizeof(int),  1,fp);
	fread(&proj->UseCustomGravity, sizeof(int), 1, fp);
	if(proj->UseCustomGravity)
		fread(&proj->Gravity,	sizeof(int), 1, fp);
	fread(&proj->Dampening,		sizeof(int),  1, fp);

	proj->PrjTrl_Proj = NULL;
	proj->Projectile = NULL;
	proj->Hit_Projectiles = false;
	proj->Timer_Projectiles = false;
	proj->PlyHit_Projectiles = false;
	proj->Animating = false;

	if(proj->Type == PRJ_PIXEL) {
		fread(&proj->NumColours, sizeof(int), 1, fp);
		fread(proj->Colour1,sizeof(int),3,fp);
		if(proj->NumColours == 2)
			fread(proj->Colour2,sizeof(int),3,fp);
	}

	else if(proj->Type == PRJ_IMAGE) {
        readString(proj->ImgFilename, fp);		
		
		proj->bmpImage = LoadGSImage(sDirectory, proj->ImgFilename);
        if(!proj->bmpImage)
            modLog("Could not open image '%s'",proj->ImgFilename);
		
		// Set the colour key
		if(proj->bmpImage)
			SDL_SetColorKey(proj->bmpImage, SDL_SRCCOLORKEY, MakeColour(255,0,255));

		fread(&proj->Rotating, sizeof(int), 1, fp);
		fread(&proj->RotIncrement, sizeof(int), 1, fp);
		fread(&proj->RotSpeed,sizeof(int),1,fp);
		fread(&proj->UseAngle,sizeof(int),1,fp);
		fread(&proj->UseSpecAngle,sizeof(int),1,fp);
		if(proj->UseAngle || proj->UseSpecAngle)
			fread(&proj->AngleImages,sizeof(int),1,fp);
		fread(&proj->Animating,sizeof(int),1,fp);
		if(proj->Animating) {
			fread(&proj->AnimRate,sizeof(float),1,fp);
			fread(&proj->AnimType,sizeof(int),1,fp);
		}
	}


	//
	// Hit
	//
	fread(&proj->Hit_Type,sizeof(int),1,fp);

	// Hit::Explode
	if(proj->Hit_Type == PJ_EXPLODE) {
		fread(&proj->Hit_Damage,		sizeof(int),1,fp);
		fread(&proj->Hit_Projectiles,	sizeof(int),1,fp);
		fread(&proj->Hit_UseSound,		sizeof(int),1,fp);
		fread(&proj->Hit_Shake,			sizeof(int),1,fp);

		if(proj->Hit_UseSound) {
            readString(proj->Hit_SndFilename, fp);			
			
			// Load the sample
			proj->smpSample = LoadGSSample(sDirectory,proj->Hit_SndFilename);
			
            if(proj->smpSample == 0) {
				proj->Hit_UseSound = false;
                modLog("Could not open sound '%s'",proj->Hit_SndFilename);
            }
		}
	}

	// Hit::Bounce
	if(proj->Hit_Type == PJ_BOUNCE) {
		fread(&proj->Hit_BounceCoeff,	sizeof(float), 1, fp);
		fread(&proj->Hit_BounceExplode, sizeof(int),   1, fp);
	}

	// Hit::Carve
	if(proj->Hit_Type == PJ_CARVE) {
		fread(&proj->Hit_Damage,		sizeof(int),1,fp);
	}





	//
	// Timer
	//
	if(proj->Timer_Time > 0) {
		fread(&proj->Timer_Type,sizeof(int),1,fp);
		if(proj->Timer_Type == PJ_EXPLODE) {
			fread(&proj->Timer_Damage,sizeof(int),1,fp);
			fread(&proj->Timer_Projectiles,sizeof(int),1,fp);
			fread(&proj->Timer_Shake,sizeof(int),1,fp);
		}
	}



	//
	// Player hit
	//
	fread(&proj->PlyHit_Type,sizeof(int),1,fp);

	// PlyHit::Explode || PlyHit::Injure
	if(proj->PlyHit_Type == PJ_INJURE || proj->PlyHit_Type == PJ_EXPLODE) {
		fread(&proj->PlyHit_Damage,sizeof(int),1,fp);
		fread(&proj->PlyHit_Projectiles,sizeof(int),1,fp);
	}

	// PlyHit::Bounce
	if(proj->PlyHit_Type == PJ_BOUNCE) {
		fread(&proj->PlyHit_BounceCoeff, sizeof(float), 1, fp);
	}


    //
    // Explode
    //
    fread(&proj->Exp_Type,     sizeof(int), 1, fp);
    fread(&proj->Exp_Damage,   sizeof(int), 1, fp);
    fread(&proj->Exp_Projectiles, sizeof(int), 1, fp);
    fread(&proj->Exp_UseSound, sizeof(int), 1, fp);
    if(proj->Exp_UseSound)
        readString(proj->Exp_SndFilename, fp);


    //
    // Touch
    //
    fread(&proj->Tch_Type,     sizeof(int), 1, fp);
    fread(&proj->Tch_Damage,   sizeof(int), 1, fp);
    fread(&proj->Tch_Projectiles, sizeof(int), 1, fp);
    fread(&proj->Tch_UseSound, sizeof(int), 1, fp);
    if(proj->Tch_UseSound)
        readString(proj->Tch_SndFilename, fp);


	if(proj->Timer_Projectiles || proj->Hit_Projectiles || proj->PlyHit_Projectiles || proj->Exp_Projectiles ||
       proj->Tch_Projectiles) {
		fread(&proj->ProjUseangle,	sizeof(int),	1, fp);
		fread(&proj->ProjAngle,		sizeof(int),	1, fp);
		fread(&proj->ProjAmount,	sizeof(int),	1, fp);
		fread(&proj->ProjSpread,	sizeof(float),	1, fp);
		fread(&proj->ProjSpeed,		sizeof(int),	1, fp);
		fread(&proj->ProjSpeedVar,	sizeof(float),	1, fp);

		proj->Projectile = LoadProjectile(fp);
	}

	// Projectile trail
	if(proj->Trail == TRL_PROJECTILE) {
		
		fread(&proj->PrjTrl_UsePrjVelocity,	sizeof(int),	1, fp);
		fread(&proj->PrjTrl_Delay,			sizeof(float),	1, fp);
		fread(&proj->PrjTrl_Amount,			sizeof(int),	1, fp);
		fread(&proj->PrjTrl_Speed,			sizeof(int),	1, fp);
		fread(&proj->PrjTrl_SpeedVar,		sizeof(float),	1, fp);
		fread(&proj->PrjTrl_Spread,			sizeof(float),	1, fp);

		// Change from milli-seconds to seconds
		proj->PrjTrl_Delay /= 1000.0f;

		proj->PrjTrl_Proj = LoadProjectile(fp);
	}

	return proj;
}


///////////////////
// Load an image
SDL_Surface *CGameScript::LoadGSImage(char *dir, char *filename)
{
	SDL_Surface *img = NULL;
	char buf[256];

	// First, check the gfx directory in the mod dir
	sprintf(buf,"%s/gfx/%s",dir,filename);
	img = LoadImage(buf,SDL_GetVideoSurface()->format->BitsPerPixel);

	if(img)
		return img;

	// Check the gfx directory in the data dir
	sprintf(buf,"data/gfx/%s",filename);
	return LoadImage(buf,SDL_GetVideoSurface()->format->BitsPerPixel);
}


///////////////////
// Load a sample
HSAMPLE CGameScript::LoadGSSample(char *dir, char *filename)
{
	HSAMPLE smp = 0;
	char buf[256];

	// First, check the sfx directory in the mod dir
	sprintf(buf,"%s/sfx/%s",dir,filename);
	smp = LoadSample(buf,10);
	
	if(smp)
		return smp;

	// Check the sounds directory in the data dir
	sprintf(buf,"data/sounds/%s",filename);
	return LoadSample(buf,10);
}

#endif  //  _CONSOLE



///////////////////
// Find a weapon based on its name
weapon_t *CGameScript::FindWeapon(char *name)
{
	int n;

    // Safety check
    if( !name )
        return Weapons;     // Just return the first weapon

	// Go through each weapon
	weapon_t *wpn = Weapons;
	for(n=0;n<NumWeapons;n++,wpn++) {

		if(stricmp(wpn->Name, name) == 0)
			return wpn;
	}

	// Instead of returning NULL, just return the first weapon
	return Weapons;
}


///////////////////
// Returns true if the weapon is in the game script
bool CGameScript::weaponExists(char *szName)
{
    if(!szName)
        return false;

    // Go through each weapon
	weapon_t *wpn = Weapons;
	for(int n=0;n<NumWeapons;n++,wpn++) {

		if(stricmp(wpn->Name, szName) == 0)
			return true;
	}

    // Not found
    return false;
}


///////////////////
// Write a string in pascal format
void CGameScript::writeString(char *szString, FILE *fp)
{
    assert( szString );

    char length = strlen(szString);

    fwrite( &length, sizeof(char), 1, fp );
    fwrite( szString,sizeof(char), length, fp );
}


///////////////////
// Read a string in pascal format
char *CGameScript::readString(char *szString, FILE *fp)
{
    assert( szString );

    char length;
    fread( &length, sizeof(char), 1, fp );
    fread( szString,sizeof(char), length, fp );

    szString[length] = '\0';

    return szString;
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
	weapon_t *wpn = Weapons;
	for(n=0;n<NumWeapons;n++,wpn++) {

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
int CGameScript::CheckFile(char *dir, char *name)
{
	char filename[256];

	sprintf(filename,"%s/script.lgs",dir);

	// Open it
	FILE *fp = fopen_i(filename,"rb");
	if(fp == NULL)
		return false;

	memset(&Header,0,sizeof(gs_header_t));


	// Header
	fread(&Header,sizeof(Header),1,fp);

	// Check ID
	if(strcmp(Header.ID,"Liero Game Script") != 0) {
		fclose(fp);
		return false;
	}

	// Check version
	if(Header.Version != GS_VERSION) {
		fclose(fp);
		return false;
	}

	strcpy(name, Header.ModName);

	fclose(fp);

	return true;
}


///////////////////
// Return an error message based on code
char *CGameScript::getError(char *text, int code)
{
	strcpy(text, "Undefined error");

	switch(code) {

		case GSE_MEM:
			strcpy(text, "Out of memory");
			break;

		case GSE_VERSION:
			strcpy(text, "Incorrect version");
			break;

		case GSE_FILE:
			strcpy(text, "Could not open file");
			break;

		case GSE_BAD:
			strcpy(text, "Bad file format");
			break;
	}

	return text;
}


///////////////////
// Write info to a mod log file
void CGameScript::modLog(char *fmt, ...)
{
    char    buf[1024];

    va_list	va;

	va_start(va,fmt);
	vsprintf(buf,fmt,va);
	va_end(va);

	if(!pModLog) {
		pModLog = fopen_i("modlog.txt","wt");
		if(!pModLog)
			return;
		fprintf(pModLog,"Log file for mod:\n%s\n--------------------------------\n",Header.ModName);
	}
	
	fprintf(pModLog,"%s\n",buf);
}
