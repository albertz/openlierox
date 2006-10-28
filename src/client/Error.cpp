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
		ErrorFile = fopen_i("Error.txt","wt");
		if(ErrorFile == NULL)
			return;
		fprintf(ErrorFile,"%s error file\n----------------------\n",GetGameName());
	}
	
	fprintf(ErrorFile,"%s\n",ErrorMsg);
}


///////////////////
// Show the error
void ShowError(void)
{
	SDL_ShowCursor(SDL_ENABLE);

	// TODO: uniform message system

	if(GotError) {
		printf("SDL: Error: %s\n", ErrorMsg);
		//MessageBox(NULL,ErrorMsg,"Liero Xtreme",MB_OK | MB_ICONEXCLAMATION);
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
	//MessageBox(NULL,buf,"Liero Xtreme",MB_OK | MB_ICONEXCLAMATION);


	// Shutdown the game
	SDL_Quit();
	exit(1);
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
