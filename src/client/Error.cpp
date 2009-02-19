/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Error system
// Created 12/11/01
// By Jason Boettcher


#include <stdarg.h>

#include "LieroX.h"
#include "Debug.h"
#include "AuxLib.h"
#include "Error.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "Version.h"


int		GotError = false;
char	ErrorMsg[1024]; // HINT: if we change this to std::string, check other code here; sizeof(ErrorMsg) is used for example
std::string	LastError;

FILE *ErrorFile = NULL;




///////////////////
// Sets the error message
void SetError(const std::string& text)
{
	if (text.size() == 0)
		return;

	GotError = true;

	errors << "SetError: " << text << endl;

	if(ErrorFile == NULL) {
		ErrorFile = OpenGameFile("Error.txt","wt");
		if(ErrorFile == NULL)
			return;
		fprintf(ErrorFile, "%s", GetFullGameName());
		fprintf(ErrorFile, " error file\n----------------------\n");
	}

	fprintf(ErrorFile,"%s\n", text.c_str());
	//printf("Error: %s\n", ErrorMsg);

}


///////////////////
// Show the error
void ShowError(void)
{
	SDL_ShowCursor(SDL_ENABLE);

	// TODO: uniform message system

	if(GotError) {
		printf("SDL: Error: %s\n", ErrorMsg);
		//MessageBox(NULL,ErrorMsg,GetGameName(),MB_OK | MB_ICONEXCLAMATION);
	}
	else {
		printf("SDL: Sad: unknown error\n");
		//MessageBox(NULL,"Unkown Error",GetGameName(),MB_OK | MB_ICONEXCLAMATION);
	}

	SDL_ShowCursor(SDL_DISABLE);
}


///////////////////
// End the error system
void EndError(void)
{
	if(ErrorFile)
		fclose(ErrorFile);
	ErrorFile = NULL;
}


///////////////////
// Show a system error
void SystemError(const std::string& text)
{

	// SDL_ShowCursor(SDL_ENABLE);	// Commented out because of a bug in SDL that causes a crash when SDL_SetVideoMode fails
	if (text.size() != 0) {
		errors << "SystemError: " << text << endl;
	}
	
	// Shudown only when not already shutting down
	if (tLX)
		if (!tLX->bQuitGame)
			ShutdownLieroX();

#ifdef WIN32
	if (text.size() != 0)
		MessageBox(NULL,text.c_str(), GetGameName(),MB_OK | MB_ICONEXCLAMATION);
#endif


	// Shutdown the game
	exit(-1);
}

// List of GUI errors & warnings
std::list<std::string> GUIErrors;

/////////////////////
// Show a window informing about skin error
void GuiSkinError(const std::string& text)
{

	GUIErrors.push_back(text);

	errors << "GuiSkin: " << text << endl;

}

void LxSetLastError(const std::string& desc)
{
	LastError = desc;
}

std::string LxGetLastError(void)
{
	return LastError;
}
