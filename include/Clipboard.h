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
	Clipboard code
	06-12-2007 albert
*/

#ifndef __CLIPBOARD_H__
#define __CLIPBOARD_H__

#include <string>

void copy_to_clipboard(const std::string& text);
std::string copy_from_clipboard();

#endif

