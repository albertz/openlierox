/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Auxiliary library 
// Created 12/11/01
// By Jason Boettcher

// this is the new file!

#include "defs.h"
#include <SDL/SDL_syswm.h>
#include "LieroX.h"
#include "2xsai.h"
//#include "corona.h"


// Game info
char		GameName[32];
int         nFocus = true;

// Config file
char		ConfigFile[64];

// Keyboard, Mouse, & Event
keyboard_t	Keyboard;
mouse_t		Mouse;
SDL_Event	Event;

SDL_Surface *bmpIcon=NULL;


///////////////////
// Initialize the standard Auxiliary Library
int InitializeAuxLib(char *gname, char *config, int bpp, int vidflags)
{
	// Set the game info
	strcpy(GameName,gname);

	strcpy(ConfigFile,config);

	// Solves problem with FPS in fullscreen
#ifdef WIN32
	// TODO: do it better
	SDL_putenv("SDL_VIDEODRIVER=directx");
#endif

	
	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) == -1) {
		SystemError("Failed to initialize the SDL system!\nErrorMsg: %s",SDL_GetError());
		return false;
	}

	bmpIcon = SDL_LoadBMP("data/icon.bmp");
	if(bmpIcon)
		SDL_WM_SetIcon(bmpIcon,NULL);


	if(!SetVideoMode())
		return false;

    // Enable the system events
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);


	// Initialize the 2xsai system
	Init_2xSaI(565);

    if( !nDisableSound ) {
	    // Initialize sound
		if(!InitSoundSystem(22050, 1, 512)) {
		    printf("Warning: Failed the initialize the sound system\n");
		}
    }

	if( tLXOptions->iSoundOn ) {
		StartSoundSystem();
		SetSoundVolume( tLXOptions->iSoundVolume );
	}


	// Give a seed to the random number generator
	srand((unsigned int)time(NULL));


	// Initialize the cache
	if(!InitializeCache())
		return false;

	

	// Initialize the keyboard & mouse
	ProcessEvents();
	for(int k = 0;k<SDLK_LAST;k++)
		Keyboard.KeyUp[k] = false;
	
	Mouse.Button = 0;
	Mouse.Up = 0;



	/*
		Temp thing to create a random number table
	*/
	/*FILE *fp = OpenGameFile("randomnum.cpp", "wt");

	fprintf(fp, "float RandomNumbers[] = { ");
	for(int i=0;i<256; i++) {
		if(i % 8 == 0 && i>0)
			fprintf(fp, "\n                          ");

		float f = GetRandomNum();
		if(f>=0)
			fprintf(fp, " ");
		fprintf(fp, "%ff", f );
		if(i!= 255)
			fprintf(fp, ", ");		
	}
	fprintf(fp, " };\n\n");
	fclose(fp);*/
	
	
	return true;
}


///////////////////
// Set the video mode
int SetVideoMode(void)
{
	// TODO: Use DOUBLEBUF and hardware surfaces
	int HardwareBuf = false;
	int DoubleBuf = false;
	int vidflags = 0;

	// Initialize the video
	if(tLXOptions->iFullscreen)
		vidflags |= SDL_FULLSCREEN;

	vidflags |= SDL_SWSURFACE;
	
	/*if(HardwareBuf)
		vidflags |= SDL_HWSURFACE;
	else
		vidflags |= SDL_SWSURFACE;

	if(DoubleBuf)
		vidflags |= SDL_DOUBLEBUF;*/


	if( SDL_SetVideoMode(640,480, 16,vidflags) == NULL) {
		SystemError("Failed to set the video mode %dx%dx%d\nErrorMsg: %s", 640, 480, 16,SDL_GetError());
		return false;
	}


	SDL_EnableUNICODE(1);
	SDL_WM_SetCaption(GameName,NULL);
	SDL_ShowCursor(SDL_DISABLE);

	return true;
}


///////////////////
// Process the events
void ProcessEvents(void)
{
    // Clear the queue
    Keyboard.queueLength = 0;

	// Reset mouse wheel
	Mouse.WheelScrollUp = 0;
	Mouse.WheelScrollDown = 0;
	
	while(SDL_PollEvent(&Event)) {

        // Quit event
		if(Event.type == SDL_QUIT) {
			// Quit out in a very ugly way
			tLX->iQuitGame = true;
			tLX->iQuitEngine = true;
			//tMenu->iMenuRunning = false;
		}

		// Mouse wheel scroll
		if(Event.type == SDL_MOUSEBUTTONDOWN)  {
			switch(Event.button.button){
				case SDL_BUTTON_WHEELUP:
					Mouse.WheelScrollUp = true;
					break;
				case SDL_BUTTON_WHEELDOWN:
					Mouse.WheelScrollDown  = true;
					break;
			}  // switch 
		 }  // if

        
#ifdef WIN32        
        // System events
        if(Event.type == SDL_SYSWMEVENT) {
            SDL_SysWMmsg *msg = Event.syswm.msg;

            switch(msg->msg) {

                // Lose focus event
                case WM_KILLFOCUS:
			        nFocus = false;
                    break;

                // Gain focus event
                case WM_SETFOCUS:
                    nFocus = true;
                    break;

            }
        }
#else
	// TODO: ignore it?
#endif

        // Keyboard events
		if(Event.type == SDL_KEYUP || Event.type == SDL_KEYDOWN) {

			// Check the characters
			if(Event.key.state == SDL_PRESSED || Event.key.state == SDL_RELEASED) {
				
				char input = (char)(Event.key.keysym.unicode & 0x007F);
				if (input == 0)
					switch (Event.key.keysym.sym) {
					case SDLK_HOME:
						input = 2;
						break;
					case SDLK_END:
						input = 3;
						break;
					case SDLK_KP0:
					case SDLK_KP1:
					case SDLK_KP2:
					case SDLK_KP3:
					case SDLK_KP4:
					case SDLK_KP5:
					case SDLK_KP6:
					case SDLK_KP7:
					case SDLK_KP8:
					case SDLK_KP9:
					case SDLK_KP_MULTIPLY:
					case SDLK_KP_MINUS:
					case SDLK_KP_PLUS:
					case SDLK_KP_EQUALS:
						input = (char) (Event.key.keysym.sym - 208);
						break;
					case SDLK_KP_PERIOD:
					case SDLK_KP_DIVIDE:
						input = (char) (Event.key.keysym.sym - 220);
						break;
					case SDLK_KP_ENTER:
						input = '\r';
						break;
				}  // switch

                // If we're going to over the queue length, shift the list down and remove the oldest key
                if(Keyboard.queueLength+1 >= MAX_KEYQUEUE) {
                    for(int i=0; i<Keyboard.queueLength-1; i++)
                        Keyboard.keyQueue[i] = Keyboard.keyQueue[i+1];
                    Keyboard.queueLength--;
                }

                // Key down
                if(Event.type == SDL_KEYDOWN) {
                    Keyboard.keyQueue[Keyboard.queueLength++] = input;
                }

				// Key up
				if(Event.type == SDL_KEYUP || Event.key.state == SDL_RELEASED)
                    Keyboard.keyQueue[Keyboard.queueLength++] = -input;

                
            }
        }
	}


    // If we don't have focus, don't update as often
    /*if(!nFocus)
        SDL_Delay(14);*/


	// Mouse
	Mouse.Button = SDL_GetMouseState(&Mouse.X,&Mouse.Y);
	Mouse.Up = 0;
    Mouse.FirstDown = 0;
	
	// Left Mouse Button Up event
	if(!(Mouse.Button & SDL_BUTTON(SDL_BUTTON_LEFT)) && Mouse.Down & SDL_BUTTON(SDL_BUTTON_LEFT))
		Mouse.Up |= SDL_BUTTON(SDL_BUTTON_LEFT);

	// Right Mouse Button Up event
	if(!(Mouse.Button & SDL_BUTTON(SDL_BUTTON_RIGHT)) && Mouse.Down & SDL_BUTTON(SDL_BUTTON_RIGHT))
		Mouse.Up |= SDL_BUTTON(SDL_BUTTON_RIGHT);

	// First down
    for( int i=0; i<3; i++ ) {
        if( !(Mouse.Down & SDL_BUTTON(i)) && (Mouse.Button & SDL_BUTTON(i)) )
            Mouse.FirstDown |= SDL_BUTTON(i);
    }
		
	Mouse.Down = Mouse.Button;

    // SAFETY HACK: If we get any mouse presses, we must have focus
    if(Mouse.Down)
        nFocus = true;



	// Keyboard
	Keyboard.keys = SDL_GetKeyState(NULL);

	// Update the key up's
	for(int k=0;k<SDLK_LAST;k++) {
		Keyboard.KeyUp[k] = false;
		
		if(!Keyboard.keys[k] && Keyboard.KeyDown[k])
			Keyboard.KeyUp[k] = true;
		Keyboard.KeyDown[k] = Keyboard.keys[k];        
	}
}


///////////////////
// Flip the screen
void FlipScreen(SDL_Surface *psScreen)
{
    // Take a screenshot?
    // We do this here, because there are so many graphics loops, but this function is common
    // to all of them
    if( cTakeScreenshot.isDownOnce() )
        TakeScreenshot();

    SDL_Flip( psScreen );
}


///////////////////
// Shutdown the standard Auxiliary Library
void ShutdownAuxLib(void)
{
	if(bmpIcon)
		SDL_FreeSurface(bmpIcon);

	QuitSoundSystem();

	// Shutdown the error system
	EndError();

	// Shutdown the cache
	ShutdownCache();

	// Shutdown the SDL system
	SDL_Quit();
}



///////////////////
// Return the game name
char *GetGameName(void)
{
	return GameName;
}

///////////////////
// Return the keyboard structure
keyboard_t *GetKeyboard(void)
{
	return &Keyboard;
}

///////////////////
// Return the mouse structure
mouse_t *GetMouse(void)
{
	return &Mouse;
}

///////////////////
// Return the config filename
char *GetConfigFile(void)
{
	return ConfigFile;
}


///////////////////
// Return the event
SDL_Event *GetEvent(void)
{
	return &Event;
}


///////////////////
// Get text from the clipboard
// Returns the length of the text (0 for no text)
int GetClipboardText(char *szText, int nMaxLength)
{
    if( !szText )
        return 0;

#ifdef WIN32
    // Get the window handle
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if(!SDL_GetWMInfo(&info))
		return 0;

	HWND    WindowHandle = info.window;
    HANDLE  CBDataHandle; // handle to the clipboard data
    LPSTR   CBDataPtr;    // pointer to data to send

    // Windows version
    if( IsClipboardFormatAvailable(CF_TEXT) ) {
            
        if( OpenClipboard(WindowHandle) ) {
            CBDataHandle = GetClipboardData(CF_TEXT);
                
            if(CBDataHandle) {
                CBDataPtr = (LPSTR)GlobalLock(CBDataHandle);
                int TextSize = strlen(CBDataPtr);

                strncpy(szText, CBDataPtr, nMaxLength);
                if( TextSize < nMaxLength )
                    szText[TextSize] = '\0';
                else
                    szText[nMaxLength-1] = '\0';

                GlobalUnlock(CBDataHandle);
                CloseClipboard();

                return TextSize < nMaxLength ? TextSize : nMaxLength;
            }
        }
    }
#else
	// TODO: how to do on linux?
#endif

    return 0;
}

///////////////////
// Set text to the clipboard
// Returns the length of the text (0 for no text)
int SetClipboardText(char *szText)
{
    if( !szText )
        return 0;

#ifdef WIN32    
    // Get the window handle
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if(!SDL_GetWMInfo(&info))
		return 0;

	HWND    WindowHandle = info.window;

	// Open clipboard
	if(!OpenClipboard(WindowHandle))
		return 0;

	// Empty clipboard
	EmptyClipboard();


	LPTSTR  lptstrCopy;
	int cch = strlen(szText);
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (cch + 1) * sizeof(TCHAR)); 
    if (hglbCopy == NULL) 
    { 
        CloseClipboard(); 
        return 0; 
    } 

    lptstrCopy = (char *) GlobalLock(hglbCopy); 
    memcpy(lptstrCopy, szText, cch * sizeof(TCHAR)); 
    lptstrCopy[cch] = (TCHAR) 0;    // null character 
    GlobalUnlock(hglbCopy); 

	SetClipboardData(CF_TEXT, hglbCopy); 

    return strlen(szText);
#else
	// TODO: what is with linux here?
	return 0;
#endif
}


///////////////////
// Take a screenshot
void TakeScreenshot(void)
{
	char		picname[80]; 
	char		checkname[255];
	char		extension[5];
	int			i;
	FILE		*f;

	// Set the extension
	switch (tLXOptions->iScreenshotFormat)  {
	case FMT_BMP: strcpy(extension,".bmp"); break;
	case FMT_PNG: strcpy(extension,".png"); break;
	case FMT_JPG: strcpy(extension,".jpg"); break;
	case FMT_GIF: strcpy(extension,".gif"); break;
	default: strcpy(extension,".png");
	}

    // Create the 'scrshots' directory if it doesn't exist
    strcpy(checkname, GetHomeDir());
    strcat(checkname, "/scrshots");
    mkdir(checkname, 0);

	// Create the file name
    for(i=0; 1; i++) {
		sprintf(picname,"%s%d%s","lierox",i,extension);
		sprintf(checkname, "scrshots/%s", picname);
		f = OpenGameFile(checkname, "rb");
		if (!f)
			break;	// file doesn't exist
		fclose(f);
	}

	// Save the surface
	switch (tLXOptions->iScreenshotFormat)  {
	case FMT_BMP: SDL_SaveBMP( SDL_GetVideoSurface(), checkname ); break;
	case FMT_PNG: SavePNG(SDL_GetVideoSurface(),checkname); break;
	case FMT_JPG: SaveJPG(SDL_GetVideoSurface(),checkname, tLXOptions->iJpegQuality); break;
	case FMT_GIF: SaveGIF(SDL_GetVideoSurface(),checkname); break;
	default: SavePNG(SDL_GetVideoSurface(),checkname);
	}
}

