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
	Clipboard code grabbed from YAG2002
	06-12-2007 albert
*/

/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 ****************************************************************/

/*###############################################################
 # common utilities
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "Clipboard.h"

#ifndef WIN32
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <glob.h>
 #include <dirent.h>
 #include <spawn.h>
 #include <SDL_syswm.h>
 #include <SDL.h>
#endif



#ifndef WIN32

// used for Copy & Paste
static Display* SDL_Display = NULL;
static Window   SDL_Window;
static void ( *Lock_Display   )( void );
static void ( *Unlock_Display )( void );
static bool copy_paste_initialised = false;

// init stuff for Copy & Paste
void initCopyPaste()
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo( &info ) )
    {
        if ( info.subsystem == SDL_SYSWM_X11 )
        {
            SDL_Display    = info.info.x11.display;
            SDL_Window     = info.info.x11.window;
            Lock_Display   = info.info.x11.lock_func;
            Unlock_Display = info.info.x11.unlock_func;

            SDL_EventState( SDL_SYSWMEVENT, SDL_ENABLE );
            //SDL_SetEventFilter( clipboardFilter );
        }
        else
        {
            SDL_SetError("SDL is not running on X11");
        }
    }
    
    copy_paste_initialised = true;
}

#endif

void copy_to_clipboard( const std::string& text )
{
	if(!copy_paste_initialised) initCopyPaste();
	
#ifdef WIN32

    if ( !OpenClipboard( NULL ) )
        return;

    // copy to clipboard data
    EmptyClipboard();
    HGLOBAL hmem = GlobalAlloc( GMEM_MOVEABLE, text.length() + 1 );
    char*   p_text = ( char* )GlobalLock( hmem );
    memcpy( p_text, text.c_str(), text.length() );
    p_text[ text.length() ] = ( char )0;
    GlobalUnlock( hmem );
    SetClipboardData( CF_TEXT, hmem );
    CloseClipboard();

#else // not WIN32

    if ( !SDL_Display )
       initCopyPaste();

    char* p_dst = ( char* )malloc( text.length() );
    strcpy( p_dst, text.c_str() );

    Lock_Display();
    if ( XGetSelectionOwner( SDL_Display, XA_PRIMARY ) != SDL_Window )
        XSetSelectionOwner( SDL_Display, XA_PRIMARY, SDL_Window, CurrentTime );

    XChangeProperty( SDL_Display, DefaultRootWindow( SDL_Display ), XA_CUT_BUFFER0, XA_STRING, 8, PropModeReplace, ( unsigned char* )p_dst, strlen( p_dst ) );
    Unlock_Display();

    free( p_dst );

#endif
}

std::string copy_from_clipboard()
{
	if(!copy_paste_initialised) initCopyPaste();
	std::string text;
		
#ifdef WIN32
    if ( !OpenClipboard( NULL ) )
       return "";

    HANDLE data  = GetClipboardData( CF_TEXT );
    if ( !data )
       return "";

    char* p_text = NULL;
    p_text = ( char* )GlobalLock( data );
    GlobalUnlock( data );
    CloseClipboard();
    text = p_text;
#else // not WIN32

    if ( !SDL_Display )
      initCopyPaste();

    Window         owner;
    Atom           selection;
    Atom           selntype;
    int            selnformat;
    unsigned long  nbytes;
    unsigned long  overflow;
    char*          p_src;

    Lock_Display();
    owner = XGetSelectionOwner( SDL_Display, XA_PRIMARY );
    Unlock_Display();

    if ( ( owner == None ) || ( owner == SDL_Window ) )
    {
        owner = DefaultRootWindow( SDL_Display );
        selection = XA_CUT_BUFFER0;
    }
    else
    {
        owner = SDL_Window;
        selection = XInternAtom( SDL_Display, "SDL_SELECTION", False );
        Lock_Display();
        XConvertSelection( SDL_Display, XA_PRIMARY, XA_STRING, selection, owner, CurrentTime );
        Unlock_Display();
     }

    Lock_Display();
    if ( XGetWindowProperty( SDL_Display, owner, selection, 0, INT_MAX/4, False, XA_STRING, &selntype, &selnformat,
         &nbytes, &overflow, ( unsigned char ** )&p_src ) == Success )
    {
        if ( selntype == XA_STRING )
            text = p_src;

        XFree( p_src );
    }
    Unlock_Display();

#endif

    return text;
}
