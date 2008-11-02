/*
 *  NotifyUser.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 25.09.08.
 *  code under LGPL
 *
 */

#include "NotifyUser.h"

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
	#include <X11/Xlib.h>
	#include <SDL.h>
	#include <SDL_syswm.h>

void x11_SetDemandsAttention( bool v ) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWMInfo(&info);

	info.info.x11.lock_func();

	// they will stay always the same as long as X11 is running
	static Atom demandsAttention = XInternAtom(info.info.x11.display, "_NET_WM_STATE_DEMANDS_ATTENTION", true);
	static Atom wmState = XInternAtom(info.info.x11.display, "_NET_WM_STATE", true);

	XEvent e;
	e.xclient.type = ClientMessage;
	e.xclient.message_type = wmState;
	e.xclient.display = info.info.x11.display;
	e.xclient.window = info.info.x11.wmwindow;
	e.xclient.format = 32;
	e.xclient.data.l[0] = v ? 1 : 0;
	e.xclient.data.l[1] = demandsAttention;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	if( v ) // ensure that the property is there (that is not guaranteed on all systems)
		XChangeProperty(info.info.x11.display, info.info.x11.wmwindow, wmState, XA_ATOM, 32, PropModeReplace, (unsigned char *)&demandsAttention, 1);

	XSendEvent(info.info.x11.display, DefaultRootWindow(info.info.x11.display), False, (SubstructureRedirectMask | SubstructureNotifyMask), &e);

	info.info.x11.unlock_func();
}

#endif

#include <iostream>

#include "InputEvents.h"
#include "Sounds.h"
#include "AuxLib.h"


using namespace std;

// HINT: I don't include LieroX.h directly as it declares Rect<>, which is already declared by Carbon
extern bool bDedicated;


// Make the sound, and blink the game window (platform-dependent)
void NotifyUserOnEvent()
{
	if( bDedicated || ApplicationHasFocus() )
		return;

	cout << "Notifying busy user" << endl;

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

#elif defined(WIN32)
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

	x11_SetDemandsAttention(true);

#else

	cout << "WARNING: NotifyUserOnEvent(): cannot notify windowmanager, unsupported system" << endl;

#endif

}

void ClearUserNotify() {
	if(bDedicated) return;
#if defined(__APPLE__)
#elif defined(WIN32)
#elif defined(X11)
	x11_SetDemandsAttention(false);
#endif
}

#endif
