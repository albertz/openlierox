/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Profile system
// Created 13/8/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


profile_t	*tProfiles = NULL;


///////////////////
// Load the profiles
int LoadProfiles(void)
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
	char id[32];
	fread(id, sizeof(char), 32, fp);
	if(strcmp(id, "lx:profile") != 0) {

        // Add the default players
        AddDefaultPlayers();
		fclose(fp);
		return false;
	}

	// Check version
	int ver = 0;
	fread(&ver, sizeof(int), 1, fp);
	EndianSwap(ver);
	if(ver != PROFILE_VERSION) {

        // Add the default players
        AddDefaultPlayers();
		fclose(fp);
		return false;
	}

	// Get number of profiles
	int num = 0;
	fread(&num,	sizeof(int), 1, fp);
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

	return true;
}


///////////////////
// Add the default players to the list
void AddDefaultPlayers(void)
{
	short		i;
	static char	buf[32];

	// Pre-set cpu colours
	Uint32 cpuColours[] = { 255,0,0,  0,255,0,  0,0,255,  255,0,255,  0,255,255,  128,128,128,
							128,255,0,  0,128,255, 0,128,0 };

    // Pre-set cpu difficulties
    int Diff[] = {AI_EASY, AI_MEDIUM, AI_MEDIUM, AI_HARD, AI_XTREME, AI_MEDIUM, AI_EASY};

    // Add the default worm
    AddProfile("worm", "default.png", "", "", 100,100,255, PRF_HUMAN,0);

	// Add 7 ai players
	for(i=0; i<7; i++) {
		snprintf(buf, sizeof(buf), "CPU %d", i+1); fix_markend(buf);
		
		AddProfile(buf, "default.png", "", "", cpuColours[i*3], cpuColours[i*3+1], cpuColours[i*3+2], PRF_COMPUTER, Diff[i]);
	}
}


///////////////////
// Save the profiles
void SaveProfiles(void)
{
	profile_t	*p = tProfiles;
	profile_t	*pf;


	//
	// Open the file
	//
	FILE *fp = OpenGameFile("cfg/players.dat","wb");
	if(fp == NULL)
		return;

	// ID & Version
	static const char id[32] = {"lx:profile"};
	fwrite(id, sizeof(char), 32, fp);

	int ver = PROFILE_VERSION;
	fwrite(GetEndianSwapped(ver), sizeof(int), 1, fp);


	// Count how many profiles we have
	int Num=0;
	for(;p;p=p->tNext)
        Num++;

	fwrite(GetEndianSwapped(Num), sizeof(int), 1, fp);


	p = tProfiles;
	for(; p; p = pf) {
		pf = p->tNext;

		// Save the profile
		SaveProfile(fp, p);
	}

}


///////////////////
// Shutdown & save the profiles
void ShutdownProfiles(void)
{
	profile_t	*p = tProfiles;
	profile_t	*pf;


	//
	// Open the file
	//
	FILE *fp = OpenGameFile("cfg/players.dat","wb");
	if(fp == NULL)
		return;

	// ID & Version
	static const char id[32] = {"lx:profile"};
	fwrite(id, sizeof(char), 32, fp);

	int ver = PROFILE_VERSION;
	fwrite(GetEndianSwapped(ver), sizeof(int), 1, fp);


	// Count how many profiles we have
	int Num=0;
	for(;p;p=p->tNext)
        Num++;

	fwrite(GetEndianSwapped(Num), sizeof(int), 1, fp);


	p = tProfiles;
	for(; p; p = pf) {
		pf = p->tNext;

		// Save the profile
		SaveProfile(fp, p);

		// Free the surface
		if(p->bmpWorm)
			SDL_FreeSurface(p->bmpWorm);
		p->bmpWorm = NULL;

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

	p = new profile_t;
	if(p == NULL)
		return;

	p->iID = id;
	p->tNext = NULL;
    p->bmpWorm = NULL;


	// Name
	fread(p->sName,		sizeof(char),	32,	fp);
    fread(p->szSkin,    sizeof(char),   128,fp);
    fread(&p->iType,    sizeof(int),    1,  fp);
    EndianSwap(p->iType);
    fread(&p->nDifficulty,sizeof(int),  1,  fp);
	EndianSwap(p->nDifficulty);
	
	// Multiplayer
	fread(p->sUsername, sizeof(char),	16, fp);
	fread(p->sPassword, sizeof(char),	16, fp);

	// Colour
	fread(&p->R,		sizeof(Uint8),	1,	fp);
	EndianSwap(p->R);
	fread(&p->G,		sizeof(Uint8),	1,	fp);
	EndianSwap(p->G);
	fread(&p->B,		sizeof(Uint8),	1,	fp);
	EndianSwap(p->B);
	
	// Weapons
	for(int i=0; i<5; i++)
		fread(p->sWeaponSlots[i],	sizeof(char),	64,	fp);


	// Load the image
	LoadProfileGraphics(p);


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
	fwrite(p->sName,	sizeof(char),	32,	fp);
    fwrite(p->szSkin,   sizeof(char),   128,fp);
    fwrite(GetEndianSwapped(p->iType),   sizeof(int),    1,  fp);
    fwrite(GetEndianSwapped(p->nDifficulty),sizeof(int), 1,  fp);

	// Multiplayer
	fwrite(p->sUsername, sizeof(char),	16, fp);
	fwrite(p->sPassword, sizeof(char),	16, fp);

	// Colour
	fwrite(GetEndianSwapped(p->R),		sizeof(Uint8),	1,	fp);
	fwrite(GetEndianSwapped(p->G),		sizeof(Uint8),	1,	fp);
	fwrite(GetEndianSwapped(p->B),		sizeof(Uint8),	1,	fp);

	// Weapons		
	for(int i=0; i<5; i++)
		fwrite(p->sWeaponSlots[i],	sizeof(char),	64,	fp);
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

			// Free me image
			if(p->bmpWorm)
				SDL_FreeSurface(p->bmpWorm);

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
void AddProfile(char *name, char *skin, char *username, char *password,  int R, int G, int B, int type, int difficulty)
{
	profile_t	*p;

	p = new profile_t;
	if(p == NULL)
		return;

	// Find a free id
	int id = FindProfileID();

	p->iID = id;
	p->iType = type;
    p->nDifficulty = difficulty;
	p->tNext = NULL;
    p->bmpWorm = NULL;

	fix_strncpy(p->sName, name);
    fix_strncpy(p->szSkin, skin);
	p->R = R;
	p->G = G;
	p->B = B;

	fix_strncpy(p->sUsername, username);
	fix_strncpy(p->sPassword, password);


	// Default weapons
	strcpy(p->sWeaponSlots[0], "minigun");
	strcpy(p->sWeaponSlots[1], "super shotgun");
	strcpy(p->sWeaponSlots[2], "blaster");
	strcpy(p->sWeaponSlots[3], "gauss gun");
	strcpy(p->sWeaponSlots[4], "big nuke");


	// Load the image
	LoadProfileGraphics(p);


	// Add the profile onto the list
	if(tProfiles) {

		profile_t *pf = tProfiles;
		profile_t *prv = NULL;

		for(;pf;pf = pf->tNext) {

			// If we are human, we need to insert ourselves into the list before the ai players
			if(p->iType == PRF_HUMAN) {
				if(pf->iType == PRF_COMPUTER) {

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
int FindProfileID(void)
{
	profile_t *p = tProfiles;

	int id = -1;

	for(; p; p=p->tNext)
		id = p->iID;

	return id+1;
}


///////////////////
// Get the profiles
profile_t *GetProfiles(void)
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


///////////////////
// Load a worm's graphics
int LoadProfileGraphics(profile_t *p)
{
    // Free the old surface
    if(p->bmpWorm)
		SDL_FreeSurface(p->bmpWorm);

	p->bmpWorm = gfxCreateSurface(18,16);
	if(p->bmpWorm == NULL) {
		// Error
		return false;
	}
	SDL_SetColorKey(p->bmpWorm, SDL_SRCCOLORKEY, MakeColour(255,0,255));
    DrawRectFill(p->bmpWorm, 0,0,p->bmpWorm->w,p->bmpWorm->h, MakeColour(255,0,255));

    // Draw the preview pic
    SDL_Surface *w = LoadSkin(p->szSkin, p->R, p->G, p->B);
    if(w) {
        DrawImageAdv(p->bmpWorm, w, 134,2,0,0, 18,16);
        SDL_FreeSurface(w);
    }
	
	// Apply a little cpu pic on the worm pic on ai players
	SDL_Surface *ai = LoadImage("data/frontend/cpu.png", 16);
	if(ai) {
		SDL_SetColorKey(ai, SDL_SRCCOLORKEY, MakeColour(255,0,255));
		
        if(p->iType == PRF_COMPUTER)
            DrawImageAdv(p->bmpWorm, ai, p->nDifficulty*10,0, 0,p->bmpWorm->h - ai->h, 10,ai->h);
	}

	return true;
}


///////////////////
// General skin colouriser
SDL_Surface *LoadSkin(char *szSkin, int colR, int colG, int colB)
{
   static  char buf[512];

    // Load the skin
    snprintf(buf, sizeof(buf), "skins/%s",szSkin); fix_markend(buf);
    SDL_Surface *worm = LoadImage(buf,16);
    if( !worm ) {
        
        // If we can't load the skin, try the default skin
        worm = LoadImage("skins/default.png",16);
        if( !worm )
            return NULL;
    }

    SDL_Surface *skin = gfxCreateSurface(672,18);
    if( !skin )
        return NULL;

    // Set the pink colour key & fill it with pink
    SDL_SetColorKey(skin, SDL_SRCCOLORKEY, MakeColour(255,0,255));
    DrawRectFill(skin,0,0,skin->w,skin->h,MakeColour(255,0,255));


    // Set the colour of the worm
	int x,y;
	Uint8 r,g,b,a;
	Uint32 pixel, mask;
	float r2,g2,b2;

	for(y=0; y<18; y++) {
		for(x=0; x<skin->w; x++) {

			pixel = GetPixel(worm,x,y);
            mask = GetPixel(worm,x,y+18);
			GetColour4(pixel,worm,&r,&g,&b,&a);

            //
            // Use the mask to check what colours to ignore
            //
            
            // Black means to just copy the colour but don't alter it
            if( mask == MakeColour(0,0,0) ) {
                PutPixel(skin, x,y, pixel);
                continue;
            }

            // Pink means just ignore the pixel completely
            if( mask == MakeColour(255,0,255) )
                continue;

            // Must be white (or some over unknown colour)
			float dr, dg, db;

			dr = (float)r / 96.0f;
			dg = (float)g / 156.0f;
			db = (float)b / 252.0f;

			r2 = (float)colR * dr;
			g2 = (float)colG * dg;
			b2 = (float)colB * db;

			r2 = MIN((float)255,r2);
			g2 = MIN((float)255,g2);
			b2 = MIN((float)255,b2);


			// Bit of a hack to make sure it isn't completey pink (see through)
			if(MakeColour((int)r2, (int)g2, (int)b2) == MakeColour(255,0,255)) {
				r2=240;
				b2=240;
			}

            // Put the colourised pixel
			PutPixel(skin,x,y, MakeColour((int)r2, (int)g2, (int)b2));
		}
	}

    return skin;
}
