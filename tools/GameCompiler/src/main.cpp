/////////////////////////////////////////
//
//         LieroX Game Script Compiler
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////

// OpenLieroX
// code under LGPL


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





// Prototypes
int		CheckArgs(int argc, char *argv[]);


///////////////////
// Main entry point
int main(int argc, char *argv[])
{
	notes << "Liero Xtreme Game Script Compiler" << endl;
	notes << "(c) ..-2002  Auxiliary Software " << endl;
	notes << "    2002-..  OpenLieroX team" << endl;
	notes << "Version: " << GetGameVersion().asString() << endl;
	notes << "GameScript Version: " << GS_VERSION << endl << endl << endl;


	if( !CheckArgs(argc, argv) ) {
		return 1;
	}

	CGameScript	*Game = new CGameScript;
	if(Game == NULL) {
		errors << "GameCompiler: Out of memory while creating gamescript" << endl;
		return false;
	}

	// Compile
	bool comp = Game->Compile(argv[1]);

	// Only save if the compile went ok
	if(comp) {
		notes << endl << "Saving..." << endl;
		Game->Save(argv[2]);
	}

	if(comp)
		notes << endl <<
				"Info:" << endl <<
				"Weapons: " << Game->GetNumWeapons() << endl <<
				"Projectiles: " << Game->getProjectileCount() << endl;

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
		notes << "Usage:" << endl;
		notes << d << " [Mod dir] [filename]" << endl;
		notes << endl << "Example:" << endl;
		notes << d << " Base script.lgs" << endl << endl;
		return false;
	}

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

bool IsFileAvailable(const std::string& f, bool absolute) {
	// stub
	FILE * ff = fopen(f.c_str(), "r");
	if( !ff )
		return false;
	fclose(ff);
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

class GameOptions;
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

class CClient;
CClient* cClient = NULL;
