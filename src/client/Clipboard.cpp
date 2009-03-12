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


#include <algorithm>

#include "Clipboard.h"
#include "Unicode.h"  // for Utf8ToSystemNative
#include "Debug.h" // struct AbsTime in types.h conflicts with typedef AbsTime in system header X.h

#if !defined(DEDICATED_ONLY) && defined(X11) && !defined(__APPLE__)

#define CLIPBOARD_FUNCS_DEFINED

extern "C" void copy_to_clipboard_x11( const char * text );

extern "C" const char * copy_from_clipboard_x11();

extern "C" void handle_system_event_x11(const SDL_Event* event);

void copy_to_clipboard(const std::string& text)
{
	if (text.empty()) 
	{
		return;
	};
	copy_to_clipboard_x11( text.c_str() );
}


std::string copy_from_clipboard()
{
	std::string ret;
	const char * text = copy_from_clipboard_x11();
	if( text )
	{
		ret = text;
	};
	return ret;
}

void handle_system_event(const SDL_Event& event)
{
	handle_system_event_x11(&event);
}

#endif

#if !defined(DEDICATED_ONLY) && defined(WIN32)
#include <windows.h>
#include "StringUtils.h"

#define CLIPBOARD_FUNCS_DEFINED

void handle_system_event(const SDL_Event& )
{}

void copy_to_clipboard(const std::string& text)
{
	if(text.empty())
		return;
	if(!OpenClipboard(NULL))
		return;
	EmptyClipboard();

	//convert newlines
	std::string str;
	replace(text, "\n", "\r\n", str);
	replace(str, "\r\r", "\r", str);

	// Convert to system native encoding
	str = Utf8ToSystemNative(str);

	const HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE, (str.size() + 1) * sizeof(TCHAR));
	if(hglb == NULL) {
		CloseClipboard();
		return;
	}
	char* const buffer = reinterpret_cast<char* const>(GlobalLock(hglb));
	strcpy(buffer, str.c_str());
	GlobalUnlock(hglb);
	SetClipboardData(CF_TEXT, hglb);
	CloseClipboard();
}

std::string copy_from_clipboard()
{
	if(!IsClipboardFormatAvailable(CF_TEXT))
		return "";
	if(!OpenClipboard(NULL))
		return "";

	HGLOBAL hglb = GetClipboardData(CF_TEXT);
	if(hglb == NULL) {
		CloseClipboard();
		return "";
	}
	char const * buffer = reinterpret_cast<char*>(GlobalLock(hglb));
	if(buffer == NULL) {
		CloseClipboard();
		return "";
	}

	GlobalUnlock(hglb);
	CloseClipboard();
	return SystemNativeToUtf8(std::string(buffer));
}

#endif

#if !defined(DEDICATED_ONLY) && defined(__BEOS__)
#include <Clipboard.h>

#define CLIPBOARD_FUNCS_DEFINED

void copy_to_clipboard(const std::string& text)
{
	BMessage *clip;
	if (be_clipboard->Lock())
	{
		be_clipboard->Clear();
		if ((clip = be_clipboard->Data()))
		{
			clip->AddData("text/plain", B_MIME_TYPE, text.c_str(), text.size()+1);
			be_clipboard->Commit();
		}
		be_clipboard->Unlock();
	}
}

std::string copy_from_clipboard()
{
	const char* data;
	ssize_t size;
	BMessage *clip = NULL;
	if (be_clipboard->Lock())
	{
		clip = be_clipboard->Data();
		be_clipboard->Unlock();
	}
	if (clip != NULL && clip->FindData("text/plain", B_MIME_TYPE, (const void**)&data, &size) == B_OK)
		return (const char*)data;
	else
		return "";
}
#endif

#if !defined(DEDICATED_ONLY) && defined(__APPLE__)
#define CLIPBOARD_FUNCS_DEFINED

#include <Carbon/Carbon.h>

void copy_to_clipboard(const std::string& text)
{
	std::string new_str;
	new_str.reserve(text.size());
	for (size_t i = 0; i < text.size(); ++i)
	{
		if (text[i] == '\n')
		{
			new_str.push_back('\r');
		} else {
			new_str.push_back(text[i]);
		}
	}
	OSStatus err = noErr;
	ScrapRef scrap = kScrapRefNone;
	err = ClearCurrentScrap();
	if (err != noErr) return;
	err = GetCurrentScrap(&scrap);
	if (err != noErr) return;
	PutScrapFlavor(scrap, kScrapFlavorTypeText, kScrapFlavorMaskNone, text.size(), new_str.c_str());
}

std::string copy_from_clipboard()
{
	ScrapRef curscrap = kScrapRefNone;
	Size scrapsize = 0;
	OSStatus err = noErr;
	err = GetCurrentScrap(&curscrap);
	if (err != noErr) return "";
	err = GetScrapFlavorSize(curscrap, kScrapFlavorTypeText, &scrapsize);
	if (err != noErr) return "";
	std::string str;
	str.reserve(scrapsize);
	str.resize(scrapsize);
	err = GetScrapFlavorData(curscrap, kScrapFlavorTypeText, &scrapsize, const_cast<char*>(str.data()));
	if (err != noErr) return "";
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] == '\r') str[i] = '\n';
	}
	return str;
}

void handle_system_event(const SDL_Event& event)
{
}

#endif

#ifndef CLIPBOARD_FUNCS_DEFINED

void copy_to_clipboard(const std::string& text)
{
	warnings << "clipboard not implemented for this system" << endl;
}

std::string copy_from_clipboard()
{
	warnings << "clipboard not implemented for this system" << endl;
	return "";
}

void handle_system_event(const SDL_Event& event)
{
}

#endif

