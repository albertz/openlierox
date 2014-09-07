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
	extern "C" void mac__NotifyUserOnEvent();
	extern "C" void mac__ClearUserNotify();
#elif defined(WIN32)
	#include <windows.h>
	#include <SDL.h>
	#include <SDL_syswm.h>
#elif 0 // defined(X11) // TODO SDL2 support ...
	#define Font Font_Xlib // Hack to prevent name clash in precompiled header and system libs
	#include <X11/Xlib.h>
	#undef Font
	#include <SDL.h>
	#include <SDL_syswm.h>

// Should be called from main thread because SDL does not lock X11 display properly
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



#include "InputEvents.h"
#include "sound/SoundsBase.h"
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
	
	mac__NotifyUserOnEvent();
	
#elif defined(WIN32) || defined(__MINGW32_VERSION)
	FLASHWINFO flash;
	flash.cbSize = sizeof(flash);
	flash.hwnd = (HWND)GetWindowHandle();
	if (!flash.hwnd)
		return;

	flash.dwFlags = FLASHW_TIMERNOFG | FLASHW_TRAY;
	flash.uCount = 3;
	flash.dwTimeout = 0;
	FlashWindowEx( &flash );

#elif defined(X11)

	struct SetDemandsAttentionAction: public Action
	{
		Result handle()
		{
			x11_SetDemandsAttention(true);
			return true;
		} 
	};
	doActionInMainThread( new SetDemandsAttentionAction );

#else

	warnings << "NotifyUserOnEvent(): cannot notify windowmanager, unsupported system" << endl;

#endif

}

void ClearUserNotify() {
	if(bDedicated) return;
#if defined(__APPLE__)
	mac__ClearUserNotify();
#elif defined(WIN32)
#elif defined(X11)
	struct ClearDemandsAttentionAction: public Action
	{
		Result handle()
		{
			x11_SetDemandsAttention(false);
			return true;
		} 
	};
	doActionInMainThread( new ClearDemandsAttentionAction );
#endif
}

#endif
