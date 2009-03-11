/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

/*
	Clipboard code grabbed from Battle for Wesnoth Project
	06-12-2007 albert
*/

/* $Id: clipboard.cpp 19552 2007-08-15 13:41:56Z mordante $ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#if !defined(DEDICATED_ONLY) && defined(X11) && !defined(__APPLE__)

#define CLIPBOARD_FUNCS_DEFINED

#include <X11/Xlib.h>
#include <unistd.h>
#include <SDL_events.h>
#include <SDL_syswm.h>

enum { false = 0, true = 1 };	// Hack hack

/**
 The following are two classes which wrap the SDL's interface to X, including
 locking/unlocking, and which manage the atom internment. They exist mainly to make
 the actual clipboard code somewhat readable
*/
struct XHelper
{
	SDL_SysWMinfo wmInf_;

	Atom          atomTable_[6];
	int           acquireCount_;
};

static struct XHelper s_instance_;
static int s_instance_inited = 0;

	static void acquire(void);
	static void release(void);
	static Display* dpy();

	static void XHelperConstructor( struct XHelper * this )
	{
		this->acquireCount_ = 0;
		acquire();

		//Intern some atoms;
		const char* atoms[] = {
			"CLIPBOARD",
			"TEXT",
			"COMPOUND_TEXT",
			"UTF8_STRING",
			"WESNOTH_PASTE",
			"TARGETS"
		};

		XInternAtoms(dpy(), (char**)atoms, 6, false, this->atomTable_);

		release();
	}

	static struct XHelper* XHelperInstance()
	{
		if (!s_instance_inited)
		{
			s_instance_inited = 1;
			XHelperConstructor( &s_instance_ );
		}
		return &s_instance_;
	}

	static Atom XA_CLIPBOARD()
	{
		return XHelperInstance()->atomTable_[0];
	}

	static Atom XA_TEXT()
	{
		return XHelperInstance()->atomTable_[1];
	}

	static Atom XA_COMPOUND_TEXT()
	{
		return XHelperInstance()->atomTable_[2];
	}

	static Atom UTF8_STRING()
	{
		return XHelperInstance()->atomTable_[3];
	}

	static Atom WES_PASTE()
	{
		return XHelperInstance()->atomTable_[4];
	}

	static Atom XA_TARGETS()
	{
		return XHelperInstance()->atomTable_[5];
	}

	static Display* dpy()
	{
		return XHelperInstance()->wmInf_.info.x11.display;
	}

	static Window window()
	{
		return XHelperInstance()->wmInf_.info.x11.window;
	}

	static void acquire(void)
	{
		++(XHelperInstance()->acquireCount_);
		if (XHelperInstance()->acquireCount_ == 1) {
			SDL_VERSION  (&(XHelperInstance()->wmInf_.version));
			SDL_GetWMInfo(&(XHelperInstance()->wmInf_));

			XHelperInstance()->wmInf_.info.x11.lock_func();
		}
	}

	static void release(void)
	{
		--(XHelperInstance()->acquireCount_);
		if (XHelperInstance()->acquireCount_ == 0)
			XHelperInstance()->wmInf_.info.x11.unlock_func();
	}


/**
 Note: unfortunately, SDL does not keep track of event timestamps.
 This means we are forced to use CurrentTime in many spots and are
 unable to perform many safety checks. Hence, the code below is
 not compliant to the ICCCM, and may ocassionally suffer from
 race conditions if an X client is connected to the server over
 a slow/high-latency link. This implementation is also very minimal.
 The text is assumed to be reasonably small as INCR transactions are not
 supported. MULTIPLE is not supported either.

 We provide UTF8_STRING, COMPOUND_TEXT, and TEXT, and
 try to grab all of them, plus STRING (which is latin1).
*/


/**
 We primarily. keep a copy of the string to response to data requests,
 but it also has an another function: in case we're both the source
 and destination, we just copy it accross; this is so that we
 don't have to handle SelectionRequest events while waiting for SelectionNotify.
 To make this work, however, this gets cleared when we loose CLIPBOARD
*/
static char clipboard_string[2048] = ""; // Should be enough for copypaste


void handle_system_event_x11(const union SDL_Event* event)
{
	XEvent* xev = &event->syswm.msg->event.xevent;
	if (xev->type == SelectionRequest) {
		//UseX x11;
		acquire();

		//Since wesnoth does not notify us of selections,
		//we set both selection + clipboard when copying.
		if ((xev->xselectionrequest.owner     == window()) &&
		    ((xev->xselectionrequest.selection == XA_PRIMARY) ||
		     (xev->xselectionrequest.selection == XA_CLIPBOARD()))) {
			XEvent responseEvent;
			responseEvent.xselection.type      = SelectionNotify;
			responseEvent.xselection.display   = dpy();
			responseEvent.xselection.requestor = xev->xselectionrequest.requestor;
			responseEvent.xselection.selection = xev->xselectionrequest.selection;
			responseEvent.xselection.target    = xev->xselectionrequest.target;
			responseEvent.xselection.property  = None; //nothing available, by default
			responseEvent.xselection.time      = xev->xselectionrequest.time;

			//notes<<"Request for target:"<<XGetAtomName(x11->dpy(), xev->xselectionrequest.target)<<"\n";

			//### presently don't handle XA_STRING as it must be latin1

			if (xev->xselectionrequest.target == XA_TARGETS()) {
				responseEvent.xselection.property = xev->xselectionrequest.property;

				Atom supported[] = {
					XA_TEXT(),
					XA_COMPOUND_TEXT(),
					UTF8_STRING(),
					XA_TARGETS()
				};

				XChangeProperty(dpy(), responseEvent.xselection.requestor,
					xev->xselectionrequest.property, XA_ATOM, 32, PropModeReplace,
					(unsigned char*)supported, 4);
			}

			//The encoding of XA_TEXT and XA_COMPOUND_TEXT is not specified
			//by the ICCCM... So we assume wesnoth native/utf-8 for simplicity.
			//modern apps are going to use UTF8_STRING anyway
			if (xev->xselectionrequest.target == XA_TEXT() ||
			    xev->xselectionrequest.target == XA_COMPOUND_TEXT() ||
			    xev->xselectionrequest.target == UTF8_STRING()) {
				responseEvent.xselection.property = xev->xselectionrequest.property;

				XChangeProperty(dpy(), responseEvent.xselection.requestor,
					xev->xselectionrequest.property,
					xev->xselectionrequest.target, 8, PropModeReplace,
					(const unsigned char*) clipboard_string, strlen(clipboard_string));
			}

			XSendEvent(dpy(), xev->xselectionrequest.requestor, False, NoEventMask,
			   &responseEvent);
		}
		release();
	}

	if (xev->type == SelectionClear) {
		//UseX x11;
		acquire();

		if (xev->xselectionclear.selection == XA_CLIPBOARD())
			clipboard_string[0] = 0; //We no longer own the clipboard, don't try in-process C&P
		release();
	}
}

void copy_to_clipboard_x11(const char * text)
{
	strncpy(clipboard_string, text, sizeof(clipboard_string));
	clipboard_string[sizeof(clipboard_string)-1] = 0;

	//UseX x11;
	acquire();
	XSetSelectionOwner(dpy(), XA_PRIMARY, window(), CurrentTime);
	XSetSelectionOwner(dpy(), XA_CLIPBOARD(), window(), CurrentTime);
	release();
}


//Tries to grab a given target. Returns true if successful, false otherwise
static int try_grab_target(Atom target, char * ret)
{
	//UseX x11;
	acquire();

	//Cleanup previous data
	XDeleteProperty(dpy(), window(), WES_PASTE());
	XSync          (dpy(), False);

	//notes<<"We request target:"<<XGetAtomName(x11->dpy(), target)<<"\n";

	//Request information
	XConvertSelection(dpy(), XA_CLIPBOARD(), target,
	                  WES_PASTE(), window(), CurrentTime);

	//Wait (with timeout) for a response SelectionNotify
	int attempt;
	for (attempt = 0; attempt < 15; attempt++) {
		if (XPending(dpy())) {
			XEvent selectNotify;
			while (XCheckTypedWindowEvent(dpy(), window(), SelectionNotify, &selectNotify)) {
				if (selectNotify.xselection.property == None)
				{
					//Not supported. Say so.
					release();
					return false;
				}
				else if (selectNotify.xselection.property == WES_PASTE() &&
				         selectNotify.xselection.target   == target) {
					//The size
					unsigned long length = 0;
					unsigned char* data;

					//these 3 XGetWindowProperty returns but we don't use
					Atom         typeRet;
					int          formatRet;
					unsigned long remaining;

//					notes<<"Grab:"<<XGetAtomName(x11->dpy(), target)<<"\n";

					//Grab the text out of the property
					XGetWindowProperty(dpy(), window(),
					                   selectNotify.xselection.property,
					                   0, 65535/4, True, target,
					                   &typeRet, &formatRet, &length, &remaining, &data);

					if (data && length) {
						strncpy(ret, (const char*)data, sizeof(clipboard_string));
						ret[sizeof(clipboard_string)-1] = 0;
						XFree(data);
						release();
						return true;
					} else {
						release();
						return false;
					}
				}
			}
		}

		usleep(10000);
	}

	//Timed out -- return empty string
	release();
	return false;
}

const char * copy_from_clipboard_x11()
{
	if (clipboard_string[0] != 0)
		return clipboard_string; //in-wesnoth copy-paste

	static char ret[sizeof(clipboard_string)] = "";

	acquire();
	//UseX x11;

	if (try_grab_target(UTF8_STRING(), ret))
	{
		release();
		return ret;
	}

	if (try_grab_target(XA_COMPOUND_TEXT(), ret))
	{
		release();
		return ret;
	}

	if (try_grab_target(XA_TEXT(), ret))
	{
		release();
		return ret;
	}

	if (try_grab_target(XA_STRING, ret)) //acroread only provides this
	{
		release();
		return ret;
	}

	release();
	return "";
}

#endif

