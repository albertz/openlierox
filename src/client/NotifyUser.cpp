/*
 *  NotifyUser.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 25.09.08.
 *  code under LGPL
 *
 */

#include "NotifyUser.h"


#if defined(__APPLE__)
#include <Carbon/Carbon.h>
#elif defined(WIN32)
#include <windows.h>
#elif defined(HAVEX11)
#include <X11/Xlib.h>	
#endif

#include <SDL.h>
#include <SDL_syswm.h>

#include "InputEvents.h"
#include "Sounds.h"


// Make the sound, and blink the game window (platform-dependent)
void NotifyUserOnEvent()
{
/*	static float lastNotification = -9999.0f;
	if( tLX->fCurTime - lastNotification < 3.0f )
		return;		
	lastNotification = tLX->fCurTime;
	*/
	
	//printf("NotifyUserOnEvent() %i\n", ApplicationHasFocus());

	if( ApplicationHasFocus() )
		return;
	
	PlaySoundSample(sfxGeneral.smpNotify);
	
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if( SDL_GetWMInfo(&info) != 1 )
		return;

#if defined(__APPLE__)

#elif defined(WIN32)
	
	FLASHWINFO flash;
	flash.cbSize = sizeof(flash);
	flash.hwnd = info.window;
	flash.dwFlags = FLASHW_ALL;
	flash.uCount = 3;
	flash.dwTimeout = 0;
	FlashWindowEx( &flash );

#elif defined(HAVEX11)

	info.info.x11.lock_func();
	//XLockDisplay( info.info.x11.display );
	
	// It fails anyway, whatever I've tried
	//Status status = XRaiseWindow( info.info.x11.display, info.info.x11.window );
	//if( status != Success )
	//	printf("XRaiseWindow() fails %i\n", status);
	
	//XUnlockDisplay( info.info.x11.display );
	info.info.x11.unlock_func();

#else

	cout << "WARNING: NotifyUserOnEvent(): cannot notify windowmanager, unsupported system" << endl;
	
#endif

};
