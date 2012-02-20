/*
	OpenLieroX

	If you want to compile an extern tool and use the OLX source (except main.cpp),
	like many of our tools do, you need to define some extern vars.

	This is done in this file.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "CVec.h"
#include "CGameScript.h"
#include "ConfigHandler.h"
#include "SmartPointer.h"
#include "CrashHandler.h"
#include "Sounds.h"
#include "WeaponDesc.h"
#include "ProjectileDesc.h"

#include <cassert>
#include <setjmp.h>
#include <sstream> // for print_binary_string
#include <set>
#include <string>


#include "LieroX.h"
#include "IpToCountryDB.h"
#include "AuxLib.h"
#include "CClient.h"
#include "CServer.h"
#include "ConfigHandler.h"
#include "console.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "InputEvents.h"
#include "StringUtils.h"
#include "Entity.h"
#include "Error.h"
#include "DedicatedControl.h"
#include "Physics.h"
#include "Version.h"
#include "OLXG15.h"
#include "CrashHandler.h"
#include "Cursor.h"
#include "CssParser.h"
#include "FontHandling.h"
#include "Timer.h"
#include "CChannel.h"
#include "Cache.h"
#include "ProfileSystem.h"
#include "IRC.h"
#include "Music.h"
#include "Debug.h"
#include "TaskManager.h"
#include "CGameMode.h"
#include "ConversationLogger.h"
#include "StaticAssert.h"
#include "Command.h"

#include "DeprecatedGUI/CBar.h"
#include "DeprecatedGUI/Graphics.h"
#include "DeprecatedGUI/Menu.h"
#include "DeprecatedGUI/CChatWidget.h"

#include "SkinnedGUI/CGuiSkin.h"



// some dummies/stubs are following to be able to compile with OLX sources
ConversationLogger *convoLogger = NULL;
GameState currentGameState() { return S_INACTIVE; }
lierox_t	*tLX = NULL;
IpToCountryDB *tIpToCountryDB = NULL;
bool        bDisableSound = true;
bool		bDedicated = true;
bool		bJoystickSupport = false;
bool		bRestartGameAfterQuit = false;
keyboard_t	*kb = NULL;
void GotoLocalMenu(){};
void GotoNetMenu(){};
void QuittoMenu(){};
void doActionInMainThread(Action* act) {};
void doVideoFrameInMainThread() {};
void doSetVideoModeInMainThread() {};
void doVppOperation(Action* act) {};
FileListCacheIntf* modList = NULL;
FileListCacheIntf* skinList = NULL;
FileListCacheIntf* mapList = NULL;
TStartFunction startFunction = NULL;
void* startFunctionData = NULL;



void SetQuitEngineFlag(const std::string& reason) { }
#ifndef WIN32
sigjmp_buf longJumpBuffer;
#endif
void ShutdownLieroX() {}
void updateFileListCaches() {}
void SetCrashHandlerReturnPoint(const char* name) { }


void setBinaryDirAndName(char*) {}
const char* GetBinaryFilename() { return NULL; }
const char* GetLogFilename() { return "/dev/stdout"; }
