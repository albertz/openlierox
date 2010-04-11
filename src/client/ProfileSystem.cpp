/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Profile system
// Created 13/8/02
// Jason Boettcher


#include <assert.h>

#include "LieroX.h"
#include "ProfileSystem.h"
#include "EndianSwap.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "FileUtils.h"

profile_t	*tProfiles = NULL;

profile_t* FindFirstHumanProfile() {
	for(profile_t *p = tProfiles; p; p = p->tNext) {
		if(p->iType == PRF_HUMAN->toInt())
			return p;
	}
	return NULL;
}

std::string FindFirstHumanProfileName() {
	profile_t* p = FindFirstHumanProfile();
	if(p) return p->sName;
	return "";
}

///////////////////
// Load the profiles
int LoadProfiles()
{
	int		i;

	//
	// Open the file
	//
	FILE *fp = OpenGameFile("cfg/players.dat","rb");
	if(fp == NULL) {
        // Add the default players
        AddDefaultPlayers();
		return false;
	}


	//
	// Header
	//

	// Check ID
	std::string id;
	fread_fixedwidthstr<32>(id, fp);
	if(id != "lx:profile") {
		errors << "Could not load profiles: \"" << id << "\" is not equal to \"lx:profile\"" << endl;

		// Add the default players
		AddDefaultPlayers();
		fclose(fp);
		return false;
	}

	// Check version
	int ver = 0;
	fread_compat(ver, sizeof(int), 1, fp);
	EndianSwap(ver);
	if(ver != PROFILE_VERSION) {
		std::string tmp = "Could not load profiles: \""+itoa(ver)+"\" is not equal to \""+itoa(PROFILE_VERSION)+"\"";
		errors << tmp << endl;
		
        // Add the default players
        AddDefaultPlayers();
		fclose(fp);
		return false;
	}

	// Get number of profiles
	int num = 0;
	fread_compat(num,	sizeof(int), 1, fp);
	EndianSwap(num);

	// Safety check
	if(num < 0) {
		// Just leave
		fclose(fp);
        // Add the default players
        AddDefaultPlayers();
		return true;
	}


	// Load the profiles
	for(i=0; i<num; i++)
		LoadProfile(fp, i);

	fclose(fp);

	if(FindFirstHumanProfile())
		// we must do this because most code which access the profile system expects that there is at least one human profile
		AddDefaultPlayers();

	return true;
}


///////////////////
// Add the default players to the list
void AddDefaultPlayers()
{
	short		i;
	std::string	buf;

	// Pre-set cpu colours
	Uint32 cpuColours[] = { 255,0,0,  0,255,0,  0,0,255,  255,0,255,  0,255,255,  128,128,128,
							128,255,0,  0,128,255, 0,128,0 };

    // Pre-set cpu difficulties
    int Diff[] = {AI_EASY, AI_MEDIUM, AI_MEDIUM, AI_HARD, AI_XTREME, AI_MEDIUM, AI_EASY};

    // Add the default worm
    AddProfile("worm", "default.png", "", "", 100,100,255, PRF_HUMAN->toInt(),0);

	// Add 7 ai players
	for(i=0; i<7; i++) {
		buf = "CPU "+itoa(i+1);
		
		AddProfile(buf, "default.png", "", "", cpuColours[i*3], cpuColours[i*3+1], cpuColours[i*3+2], PRF_COMPUTER->toInt(), Diff[i]);
	}
}


///////////////////
// Save the profiles
void SaveProfiles()
{
	profile_t	*p = tProfiles;
	profile_t	*pf;


	//
	// Open the file
	//
	FILE *fp = OpenGameFile("cfg/players.dat","wb");
	if(fp == NULL)  {
		errors << "Could not open cfg/players.dat for writing" << endl;
		return;
	}

	// ID & Version
	fwrite("lx:profile", 32, fp);

	int ver = PROFILE_VERSION;
	fwrite_endian_compat((ver), sizeof(int), 1, fp);


	// Count how many profiles we have
	int Num=0;
	for(;p;p=p->tNext)
        Num++;

	fwrite_endian_compat((Num), sizeof(int), 1, fp);


	p = tProfiles;
	for(; p; p = pf) {
		pf = p->tNext;

		// Save the profile
		SaveProfile(fp, p);
	}

}


///////////////////
// Shutdown & save the profiles
void ShutdownProfiles()
{
	if (!tProfiles)  // Profiles not loaded, don't write and empty file (and delete all user's prifiles!)
		return;

	profile_t	*p = tProfiles;
	profile_t	*pf = NULL;


	//
	// Open the file
	//
	FILE *fp = OpenGameFile("cfg/players.dat","wb");
	if(fp == NULL)  {
		errors << "Could not open cfg/players.dat for writing" << endl;
		return;
	}

	// ID & Version
	fwrite("lx:profile", 32, fp);
	
	
	int ver = PROFILE_VERSION;
	fwrite_endian_compat((ver), sizeof(int), 1, fp);


	// Count how many profiles we have
	int Num=0;
	for(;p;p=p->tNext)
        Num++;

	fwrite_endian_compat((Num), sizeof(int), 1, fp);


	p = tProfiles;
	for(; p; p = pf) {
		pf = p->tNext;

		// Save the profile
		SaveProfile(fp, p);

		// Free the actual profile
		assert(p);
		delete p;
	}


	fclose(fp);

	tProfiles = NULL;
}


///////////////////
// Load a profile
void LoadProfile(FILE *fp, int id)
{
	profile_t	*p;

	p = new profile_t();
	if(p == NULL)
		return;

	p->iID = id;
	p->tNext = NULL;


	// Name
	p->sName = freadfixedcstr(fp, 32);
	p->cSkin.Change(freadfixedcstr(fp, 128));
    fread_compat(p->iType,    sizeof(int),    1,  fp);
    EndianSwap(p->iType);
    fread_compat(p->nDifficulty,sizeof(int),  1,  fp);
	EndianSwap(p->nDifficulty);

	if (p->iType == PRF_COMPUTER->toInt())
		p->cSkin.setBotIcon(p->nDifficulty);
	
	// Multiplayer
	p->sUsername = freadfixedcstr(fp,16);
	p->sPassword = freadfixedcstr(fp,16);

	// Colour
	fread_compat(p->R,		sizeof(Uint8),	1,	fp);
	EndianSwap(p->R);
	fread_compat(p->G,		sizeof(Uint8),	1,	fp);
	EndianSwap(p->G);
	fread_compat(p->B,		sizeof(Uint8),	1,	fp);
	EndianSwap(p->B);

	p->cSkin.setDefaultColor(Color(p->R, p->G, p->B));
	p->cSkin.Colorize(p->cSkin.getDefaultColor());
	
	// Weapons
	for(int i=0; i<5; i++)
		p->sWeaponSlots[i] = freadfixedcstr(fp,64);

	// Add the profile onto the list
	if(tProfiles) {
		profile_t *pf = tProfiles;
		for(;pf;pf = pf->tNext) {
			if(pf->tNext == NULL) {
				pf->tNext = p;
				break;
			}
		}
	}
	else
		tProfiles = p;
}


///////////////////
// Save a profile
void SaveProfile(FILE *fp, profile_t *p)
{
	// Name & Type
	fwrite(p->sName,	32,	fp);
	fwrite(p->cSkin.getFileName(),    128,fp);
    fwrite_endian_compat((p->iType),   sizeof(int),    1,  fp);
    fwrite_endian_compat((p->nDifficulty),sizeof(int), 1,  fp);

	// Multiplayer
	fwrite(p->sUsername, 	16, fp);
	fwrite(p->sPassword, 	16, fp);

	// Colour
	fwrite(&p->R, 1,	1,	fp);
	fwrite(&p->G, 1,	1,	fp);
	fwrite(&p->B, 1,	1,	fp);

	// Weapons		
	for(int i=0; i<5; i++)
		fwrite(p->sWeaponSlots[i],		64,	fp);
}


///////////////////
// Delete a profile
void DeleteProfile(int id)
{
	profile_t	*prv = NULL;
	profile_t	*p = tProfiles;

	// Find it's previous
	for(; p; p=p->tNext) {

		if(p->iID == id) {
			
			// Set the previous profiles next to my next one
			// Thus removing me from the list
			if(prv)
				prv->tNext = p->tNext;
			else
				tProfiles = p->tNext;
			
			// Free me
			delete p;

			break;
		}

		prv = p;
	}


	// Reset the id's
	p = tProfiles;
	int i=0;
	for(;p;p = p->tNext)
		p->iID = i++;
}


///////////////////
// Add a profile to the list
void AddProfile(const std::string& name, const std::string& skin, const std::string& username, const std::string& password,  int R, int G, int B, int type, int difficulty)
{
	profile_t	*p;

	p = new profile_t();
	if(p == NULL)
		return;

	// Find a free id
	int id = FindProfileID();

	p->iID = id;
	p->iType = type;
    p->nDifficulty = difficulty;
	p->tNext = NULL;

	p->sName = name;
	p->cSkin.Change(skin);
	p->R = R;
	p->G = G;
	p->B = B;
	p->cSkin.Colorize(Color(R, G, B));

	p->sUsername = username;
	p->sPassword = password;


	// Default weapons
	p->sWeaponSlots[0] = "minigun";
	p->sWeaponSlots[1] = "super shotgun";
	p->sWeaponSlots[2] = "blaster";
	p->sWeaponSlots[3] = "gauss gun";
	p->sWeaponSlots[4] = "big nuke";


	// Add the profile onto the list
	if(tProfiles) {

		profile_t *pf = tProfiles;
		profile_t *prv = NULL;

		for(;pf;pf = pf->tNext) {

			// If we are human, we need to insert ourselves into the list before the ai players
			if(p->iType == PRF_HUMAN->toInt()) {
				if(pf->iType == PRF_COMPUTER->toInt()) {

					p->tNext = pf;
					if(prv)
						prv->tNext = p;
					else
						// Must be first one
						tProfiles = p;

					break;
				}
			}


			if(pf->tNext == NULL) {
				
				pf->tNext = p;
				break;
			}

			prv = pf;
		}
	}
	else
		tProfiles = p;


	// Reset the id's
	p = tProfiles;
	int i=0;
	for(;p;p = p->tNext)
		p->iID = i++;
}


///////////////////
// Find a free profile id
int FindProfileID()
{
	profile_t *p = tProfiles;

	int id = -1;

	for(; p; p=p->tNext)
		id = p->iID;

	return id+1;
}


///////////////////
// Get the profiles
profile_t *GetProfiles()
{
	return tProfiles;
}


///////////////////
// Find a profile based on id
profile_t *FindProfile(int id)
{
	profile_t *p = tProfiles;

	for(;p;p=p->tNext) {
		if(p->iID == id)
			return p;
	}

	return NULL;
}

profile_t *FindProfile(const std::string& name) {
	profile_t *p = tProfiles;

	for(;p;p=p->tNext) {
		if(p->sName == name)
			return p;
	}

	return NULL;
}


std::string FindFirstCPUProfileName() {
	profile_t *p = tProfiles;
	
	for(;p;p=p->tNext) {
		if(p->iType == PRF_COMPUTER->toInt())
			return p->sName;
	}
	
	return "";
}
