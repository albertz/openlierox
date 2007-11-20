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


#ifndef __PROFILESYSTEM_H__
#define __PROFILESYSTEM_H__

#include <SDL.h>
#include <string>
#include <stdlib.h>

#define		PROFILE_VERSION		5

#define		PRF_HUMAN			0
#define		PRF_COMPUTER		1

// AI Difficulty
#define		AI_EASY			0
#define		AI_MEDIUM		1
#define		AI_HARD			2
#define		AI_XTREME		3



// Player profile structure
class profile_t { public:
	int				iID;

	int				iType;
	std::string		sName;
	std::string     szSkin;
    int             nDifficulty;
	std::string		sUsername;
	std::string		sPassword;
	Uint8			R,G,B;
	std::string		sWeaponSlots[5];
	int				iTeam;
	SDL_Surface		*bmpWorm;

	profile_t *tNext;
};



int		LoadProfiles(void);
void	SaveProfiles(void);
void	ShutdownProfiles(void);

void    AddDefaultPlayers(void);
void	SaveProfile(FILE *fp, profile_t *p);
void    AddProfile(const std::string& name, const std::string& skin, const std::string& username, const std::string& password,  int R, int G, int B, int type, int difficulty);
void	LoadProfile(FILE *fp, int id);
int		FindProfileID(void);
void	DeleteProfile(int id);
int		LoadProfileGraphics(profile_t *p);

profile_t *GetProfiles(void);
profile_t *FindProfile(int id);

// General function for all to use
SDL_Surface *LoadSkin(const std::string& szSkin, int colR, int colG, int colB);




#endif  //  __PROFILESYSTEM_H__
