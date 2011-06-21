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


int		GotError = false; // Specifies whether any error occured.
char	ErrorMsg[1024]; // HINT: if we change this to std::string, check other code here; sizeof(ErrorMsg) is used for example
std::string	LastError; //Holds the description of the error that occured most recently

FILE *ErrorFile = NULL;


#ifdef WIN32
#include <windows.h>
#endif


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
	//errors << ErrorMsg << endl;

}


///////////////////
// Show the error
void ShowError()
{
	EnableSystemMouseCursor(true);

	// TODO: uniform message system

	if(GotError) {
		errors << "SDL: Error: " << ErrorMsg << endl;
		//MessageBox(NULL,ErrorMsg,GetGameName(),MB_OK | MB_ICONEXCLAMATION);
	}
	else {
		errors << "SDL: Sad: unknown error" << endl;
		//MessageBox(NULL,"Unkown Error",GetGameName(),MB_OK | MB_ICONEXCLAMATION);
	}

	EnableSystemMouseCursor(false);
}


///////////////////
// End the error system
void EndError()
{
	if(ErrorFile)
		fclose(ErrorFile);
	ErrorFile = NULL;
}


///////////////////
// Show a system error
void SystemError(const std::string& text)
{
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

///////////////////
// Sets the last error description that occured to its dedicated variable.
void LxSetLastError(const std::string& desc)
{
	LastError = desc;
}

///////////////////
// Returns the last occuring error.
std::string LxGetLastError()
{
	return LastError;
}
