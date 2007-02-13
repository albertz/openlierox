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


#include "defs.h"
#include "LieroX.h"


int		GotError = false;
char	ErrorMsg[128];
char	LastError[1024];

FILE *ErrorFile = NULL;


///////////////////
// Sets the error message
void SetError(char *fmt, ...)
{
	va_list	va;

	va_start(va,fmt);
	vsnprintf(ErrorMsg,sizeof(ErrorMsg),fmt,va);
	fix_markend(ErrorMsg);
	va_end(va);
	GotError = true;


	if(ErrorFile == NULL) {
		ErrorFile = OpenGameFile("Error.txt","wt");
		if(ErrorFile == NULL)
			return;
		fprintf(ErrorFile,"%s error file\n----------------------\n",GetGameName());
	}
	
	fprintf(ErrorFile,"%s\n",ErrorMsg);
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
void SystemError(char *fmt, ...)
{
	static char buf[512];
	va_list	va;

	va_start(va,fmt);
	vsnprintf(buf,sizeof(buf),fmt,va);
	fix_markend(buf);
	va_end(va);

	SDL_ShowCursor(SDL_ENABLE);	
	// TODO: uniform message system
	printf("SystemError: %s\n", buf);

	// Quit the SDL
	SDL_Quit();
#ifdef WIN32
	MessageBox(NULL,buf,GetGameName(),MB_OK | MB_ICONEXCLAMATION);
#endif


	// Shutdown the game
	exit(1);
}

// List of GUI errors & warnings
char GUIErrors[64][64];
// Points to first free slot for error/warning
int iErrPointer = 0;

/////////////////////
// Show a window informing about skin error
void GuiSkinError(char *fmt, ...)
{
	static char buf[512];
	va_list	va;

	va_start(va,fmt);
	vsnprintf(buf,sizeof(buf),fmt,va);
	fix_markend(buf);
	va_end(va);

	iErrPointer++;

	// Too many errors, shift the list
	if(iErrPointer >= 64)  {
		int i;
		for (i=0;i<62;i++)
			fix_strncpy(GUIErrors[i],GUIErrors[i+1]);
		iErrPointer = 63;
	}

	// Copy the error
	fix_strncpy(GUIErrors[iErrPointer],buf);

	// TODO: make this better
	printf("%s\r\n",buf);

}

void LxSetLastError(char *desc)
{
	fix_strncpy(LastError,desc);
}

char *LxGetLastError(void)
{
	return &LastError[0];
}

#ifdef WIN32
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
	static char checkname[256];

	FILE *f = NULL;
	for (int i=1;1;i++)  {
		snprintf(checkname,sizeof(checkname),"bug_reports/report%i.dmp",i);
		f = OpenGameFile(checkname,"r");
		if (!f)
			break;
		else
			fclose(f);
	}


	// Open the file
	HANDLE hFile = CreateFile((LPCSTR)GetWriteFullFileName(checkname,true).c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);


	// Write the minidump
	if (hFile)
		MiniDumpWriteDump(GetCurrentProcess(),GetCurrentProcessId(),hFile,MiniDumpScanMemory,&eInfo,NULL,&cbMiniDump);

	// Close the file
	CloseHandle(hFile);

	// Quit SDL
	// TODO: is it safe to call ShutdownLieroX()?
	SDL_Quit();

	// Notify the user
	static char buf[1024];
	//sprintf(buf,"An error occured in OpenLieroX\n\nThe development team asks you for sending the crash report file.\nThis will help fixing this bug.\n\nPlease send the crash report file to karel.petranek@tiscali.cz.\n\nThe file is located in:\n %s",checkname);
	//MessageBox(0,buf,"An Error Has Occured",MB_OK);


	snprintf(buf,sizeof(buf),"\"%s\"",checkname); fix_markend(buf);
	//MessageBox(0,GetFullFileName("BugReport.exe"),"Debug",MB_OK);

	ShellExecute(NULL,"open",GetFullFileName("BugReport.exe").c_str(),buf,NULL,SW_SHOWNORMAL);

	return EXCEPTION_EXECUTE_HANDLER;
}

////////////////////
// Initializes the unhandled exception filter
void InstallExceptionFilter(void)
{
	SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);
}
#endif
