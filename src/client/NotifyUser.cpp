/*
 *  NotifyUser.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 25.09.08.
 *  code under LGPL
 *
 */

#include "NotifyUser.h"
#include "Debug.h"

#ifdef DEDICATED_ONLY

void NotifyUserOnEvent() {}
void ClearUserNotify() {}

#else

#if defined(__APPLE__)
	#include <Carbon/Carbon.h>
#elif defined(WIN32)
	#include <windows.h>
	#include <SDL.h>
	#include <SDL_syswm.h>
#elif defined(X11)

extern "C" void x11_SetDemandsAttention( int v );

#endif



#include "InputEvents.h"
#include "Sounds.h"
#include "AuxLib.h"
#include "LieroX.h"

#ifdef __MINGW32_VERSION
#include <windows.h>
#endif


// Make the sound, and blink the game window (platform-dependent)
void NotifyUserOnEvent()
{
	if( bDedicated || ApplicationHasFocus() )
		return;

	notes << "Notifying busy user" << endl;

	PlaySoundSample(sfxGeneral.smpNotify);

#if defined(__APPLE__)

	NMRecPtr	notePtr;
	OSErr		err;
	notePtr = (NMRecPtr) NewPtr ( sizeof ( NMRec ) );

	notePtr->qType = nmType; // standard queue type for NM
	notePtr->nmMark = 1;
	notePtr->nmIcon = NULL;
	notePtr->nmSound = NULL;  //(Handle) -1L; // use system alert snd
	notePtr->nmStr = NULL; // (StringPtr) NewPtr ( sizeof ( Str255 ) ); BlockMoveData( notice, notePtr->nmStr, notice[0] + 1 );
	notePtr->nmResp = NULL; // NewNMProc( MyResponse );
	notePtr->nmRefCon = 0; // fFatal;

	err = NMInstall( notePtr );

#elif defined(WIN32) || defined(__MINGW32_VERSION)
	FLASHWINFO flash;
	flash.cbSize = sizeof(flash);
	flash.hwnd = GetWindowHandle();
	if (!flash.hwnd)
		return;

	flash.dwFlags = FLASHW_TIMERNOFG | FLASHW_TRAY;
	flash.uCount = 3;
	flash.dwTimeout = 0;
	FlashWindowEx( &flash );

#elif defined(X11)

	x11_SetDemandsAttention(1);

#else

	warnings << "NotifyUserOnEvent(): cannot notify windowmanager, unsupported system" << endl;

#endif

}

void ClearUserNotify() {
	if(bDedicated) return;
#if defined(__APPLE__)
#elif defined(WIN32)
#elif defined(X11)
	x11_SetDemandsAttention(0);
#endif
}

#endif
