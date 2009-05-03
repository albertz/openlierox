#include "keys.h"

#include <console.h>
#include <string>
#include <vector>
#include <iostream>

#include <allegro.h>
#include <boost/static_assert.hpp>

using namespace std;

BOOST_STATIC_ASSERT(KEY_MAX == 127);
BOOST_STATIC_ASSERT(KEY_CAPSLOCK == 126);

int kName2Int(const string &name)
{
	for (int i = 0; i < KEY_MAX; ++i)
	{
		if(istrCmp( name, keyNames[i] ))
		{
			return i;
		}
	}
	return -1;
}

array<std::string, KEY_MAX+1> keyNames =
{
	"NULL",
	"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
	"0","1","2","3","4","5","6","7","8","9",
	"0_PAD","1_PAD","2_PAD","3_PAD","4_PAD","5_PAD","6_PAD","7_PAD","8_PAD","9_PAD",
	"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12",
	"ESC",
	"TILDE",
	"MINUS",
	"EQUALS",
	"BACKSPACE",
	"TAB",
	"OPENBRACE",
	"CLOSEBRACE",
	"ENTER",
	"COLON",
	"QUOTE",
	"BACKSLASH",
	"BACKSLASH2",
	"COMMA",
	"STOP",
	"SLASH",
	"SPACE",
	"INSERT",
	"DEL",
	"HOME",
	"END",
	"PGUP",
	"PGDN",
	"LEFT",
	"RIGHT",
	"UP",
	"DOWN",
	"SLASH_PAD",
	"ASTERISK",
	"MINUS_PAD",
	"PLUS_PAD",
	"DEL_PAD",
	"ENTER_PAD",
	"PRTSCR",
	"PAUSE",
	"ABNT_C1",
	"YEN",
	"KANA", //96
	"CONVERT",
	"NOCONVERT",
	"AT",
	"CIRCUMFLEX",
	"COLON2",
	"KANJI",
	"EQUALS_PAD",
	"BACKQUOTE",
	"SEMICOLON",
	"COMMAND",
	"UNKNOWN1",
	"UNKNOWN2",
	"UNKNOWN3",
	"UNKNOWN4",
	"UNKNOWN5",
	"UNKNOWN6",
	"UNKNOWN7",
	"UNKNOWN8",
	"LSHIFT", //115
	"RSHIFT",
	"LCONTROL",
	"RCONTROL",
	"ALT",
	"ALTGR",
	"LWIN",
	"RWIN",
	"MENU",
	"SCRLOCK",
	"NUMLOCK",
	"CAPSLOCK",
	"MAX"
};



