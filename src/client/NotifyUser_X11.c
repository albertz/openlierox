/*
 *  NotifyUser.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 25.09.08.
 *  code under LGPL
 *
 */


#if defined(X11)
	#include <X11/Xlib.h>
	#include <SDL.h>
	#include <SDL_syswm.h>

void x11_SetDemandsAttention( int v ) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWMInfo(&info);

	info.info.x11.lock_func();

	// they will stay always the same as long as X11 is running
	Atom demandsAttention = XInternAtom(info.info.x11.display, "_NET_WM_STATE_DEMANDS_ATTENTION", 1);
	Atom wmState = XInternAtom(info.info.x11.display, "_NET_WM_STATE", 1);

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

