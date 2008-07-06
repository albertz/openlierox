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
#include <iostream>

#include "LieroX.h"
#include "AuxLib.h"
#include "Error.h"
#include "FindFile.h"
#include "StringUtils.h"


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


	if(ErrorFile == NULL) {
		ErrorFile = OpenGameFile("Error.txt","wt");
		if(ErrorFile == NULL)
			return;
		fprintf(ErrorFile,"%s error file\n----------------------\n",GetGameName().c_str());
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
	// TODO: uniform message system
	if (text.size() != 0)
		printf("SystemError: %s\n", text.c_str());

	// Shudown only when not already shutting down
	if (tLX)
		if (!tLX->bQuitGame)
			ShutdownLieroX();

#ifdef WIN32
	if (text.size() != 0)
		MessageBox(NULL,text.c_str(),GetGameName().c_str(),MB_OK | MB_ICONEXCLAMATION);
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

	// TODO: make this better
	std::cout << "GUI error: " << text << std::endl;

}

void LxSetLastError(const std::string& desc)
{
	LastError = desc;
}

std::string LxGetLastError(void)
{
	return LastError;
}

#ifdef _MSC_VER

///////////////////
// This callback function is called whenever an unhandled exception occurs
LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
{
	// Set the exception info for the minidump
	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId();
	eInfo.ExceptionPointers = pExInfo;
	eInfo.ClientPointers = FALSE;

	// Set the minidump info
	MINIDUMP_CALLBACK_INFORMATION cbMiniDump;
	cbMiniDump.CallbackRoutine = NULL;
	cbMiniDump.CallbackParam = 0;

	// Get the file name
	static std::string checkname;

	FILE *f = NULL;
	for (int i=1;1;i++)  {
		checkname = "bug_reports/report" + itoa(i) + ".dmp";
		f = OpenGameFile(checkname,"r");
		if (!f)
			break;
		else
			fclose(f);
	}


	// Open the file
	std::string wffn = GetWriteFullFileName(checkname,true);
	HANDLE hFile = CreateFile((LPCSTR)wffn.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);


	// Write the minidump
	if (hFile)
		MiniDumpWriteDump(GetCurrentProcess(),GetCurrentProcessId(),hFile,MiniDumpScanMemory,&eInfo,NULL,&cbMiniDump);

	// Close the file
	CloseHandle(hFile);

	// Quit SDL
	SDL_Quit();

	// Close all opened files
	fcloseall();

	// Notify the user
	char buf[1024];  // Static not needed here
	//sprintf(buf,"An error occured in OpenLieroX\n\nThe development team asks you for sending the crash report file.\nThis will help fixing this bug.\n\nPlease send the crash report file to karel.petranek@tiscali.cz.\n\nThe file is located in:\n %s",checkname);
	//MessageBox(0,buf,"An Error Has Occured",MB_OK);


	snprintf(buf,sizeof(buf),"\"%s\"",checkname); fix_markend(buf);
	//MessageBox(0,GetFullFileName("BugReport.exe"),"Debug",MB_OK);

	std::string ffn = GetFullFileName("BugReport.exe");
	ShellExecute(NULL,"open",ffn.c_str(),buf,NULL,SW_SHOWNORMAL);

	return EXCEPTION_EXECUTE_HANDLER;
}

////////////////////
// Initializes the unhandled exception filter
void InstallExceptionFilter(void)
{
	SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);
}
#endif
