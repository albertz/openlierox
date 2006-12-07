/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Error system
// Created 12/11/01
// By Jason Boettcher


#include "defs.h"


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
	vsprintf(ErrorMsg,fmt,va);
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
	char buf[512];
	va_list	va;

	va_start(va,fmt);
	vsprintf(buf,fmt,va);
	va_end(va);

	SDL_ShowCursor(SDL_ENABLE);	
	// TODO: uniform message system
	printf("SystemError: %s\n", buf);
	//MessageBox(NULL,buf,GetGameName(),MB_OK | MB_ICONEXCLAMATION);


	// Shutdown the game
	SDL_Quit();
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
	char buf[512];
	va_list	va;

	va_start(va,fmt);
	vsprintf(buf,fmt,va);
	va_end(va);

	iErrPointer++;

	// Too many errors, shift the list
	if(iErrPointer == 64)  {
		int i;
		for (i=0;i<62;i++)
			strcpy(GUIErrors[i],GUIErrors[i+1]);
		iErrPointer = 63;
	}

	// Copy the error
	strcpy(GUIErrors[iErrPointer],buf);

	// TODO: make this better
	printf("%s\r\n",buf);

}

void LxSetLastError(char *desc)
{
	if (strlen(desc) < 1024)
		sprintf(&LastError[0],"%s",desc);
}

char *LxGetLastError(void)
{
	return &LastError[0];
}
