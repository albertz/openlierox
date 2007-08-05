# Microsoft Developer Studio Project File - Name="Game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Game - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Game.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Game.mak" CFG="Game - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Game - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Game - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Game - Win32 Profile" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Game - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../distrib/win32"
# PROP Intermediate_Dir "obj/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /Oy- /I "../../include" /I "blast" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WITH_MEDIAPLAYER" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"obj/debug/Game.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBC" /out:"../../distrib/win32/OpenLieroX.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Game - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../share/gamedir/"
# PROP Intermediate_Dir "obj/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /Ze /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "WITH_MEDIAPLAYER" /D "_AI_DEBUG" /D "DEBUG" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"obj/debug/Game.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib wsock32.lib /nologo /subsystem:console /pdb:"obj/debug/Game.pdb" /debug /machine:I386 /nodefaultlib:"LIBC" /nodefaultlib:"msvcrt.lib" /out:"../../distrib/win32/OpenLieroX_debug.exe" /MAPINFO:LINES /MAPINFO:EXPORTS
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Game - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Game___Win32_Profile"
# PROP BASE Intermediate_Dir "Game___Win32_Profile"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/profile"
# PROP Intermediate_Dir "obj/profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WITH_MEDIAPLAYER" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /profile /machine:I386 /out:"obj/Release/LieroX.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib /nologo /subsystem:windows /profile /debug /machine:I386 /nodefaultlib:"LIBC" /out:"../../share/gamedir/LieroX_profile.exe"

!ENDIF 

# Begin Target

# Name "Game - Win32 Release"
# Name "Game - Win32 Debug"
# Name "Game - Win32 Profile"
# Begin Group "System Files"

# PROP Default_Filter "cpp, h"
# Begin Source File

SOURCE=..\..\src\client\AuxLib.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\AuxLib.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Cache.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Cache.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CBytestream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CBytestream.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CChannel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CChannel.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CFont.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CFont.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CInput.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CInput.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CMediaPlayer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CMediaPlayer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CMediaPlayer_GUI.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\ConfigHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CssParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CssParser.h
# End Source File
# Begin Source File

SOURCE=..\..\include\CsvReader.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Cursor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Cursor.h
# End Source File
# Begin Source File

SOURCE=..\..\include\CVec.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Error.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Error.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\FindFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\FindFile.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\GfxPrimitives.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\GfxPrimitives.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Graphics.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Graphics.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\InputEvents.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\IpToCountryDB.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\IpToCountryDB.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\MathLib.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\MathLib.h
# End Source File
# Begin Source File

SOURCE=..\..\include\RandomNumberList.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ReadWriteLock.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\sex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\sex.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\StringUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\StringUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\Timer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Timer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\types.h
# End Source File
# Begin Source File

SOURCE=..\..\include\UCString.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\Unicode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Unicode.h
# End Source File
# End Group
# Begin Group "Game files"

# PROP Default_Filter "cpp; h"
# Begin Source File

SOURCE=..\..\src\server\CBanList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CBanList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CBar.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CBonus.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CBonus.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CChatBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CChatBox.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CClient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CClient.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CClient_Draw.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CClient_Game.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CClient_Parse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CClient_Send.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CGameScript.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CGameScript.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CMap.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CNinjaRope.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CNinjaRope.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CProjectile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CProjectile.h
# End Source File
# Begin Source File

SOURCE=..\..\src\server\CServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CServer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\server\CServer_Game.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\server\CServer_Parse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\server\CServer_Send.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CShootList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CShootList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CViewport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CViewport.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CWeather.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CWeather.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CWorm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CWorm.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CWorm_AI.cpp

!IF  "$(CFG)" == "Game - Win32 Release"

!ELSEIF  "$(CFG)" == "Game - Win32 Debug"

# SUBTRACT CPP /WX

!ELSEIF  "$(CFG)" == "Game - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\common\CWorm_SendRecv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CWorm_Simulate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\CWpnRest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CWpnRest.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Entity.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Entity.h
# End Source File
# Begin Source File

SOURCE=..\..\include\InputEvents.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\ProfileSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\ProfileSystem.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Protocol.h
# End Source File
# End Group
# Begin Group "Frontend"

# PROP Default_Filter "cpp; h"
# Begin Source File

SOURCE=..\..\src\client\CAnimation.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CAnimation.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CBox.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CBrowser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CBrowser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CButton.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CCheckbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CCheckbox.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CCombobox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CCombobox.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CGuiLayout.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CGuiLayout.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CImage.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CInputBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CInputBox.h
# End Source File
# Begin Source File

SOURCE=..\..\include\CLabel.h
# End Source File
# Begin Source File

SOURCE=..\..\include\CLine.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CListview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CListview.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\include\CProgressbar.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CScrollbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CScrollbar.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CSlider.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CSlider.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CTextbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CTextbox.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CTitleButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CTitleButton.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CWidget.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CWidget.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\CWidgetList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\CWidgetList.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Menu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Local.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Mapeditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Net.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Net_Favourites.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Net_Host.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Net_Internet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Net_Join.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Net_Lan.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Net_Main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Net_News.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Options.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Menu_Player.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\client\MenuSystem.cpp
# End Source File
# End Group
# Begin Group "Libraries"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=.\libs\pthreadVCE.lib
# End Source File
# Begin Source File

SOURCE=.\libs\SDL.lib
# End Source File
# Begin Source File

SOURCE=.\libs\SDL_image.lib
# End Source File
# Begin Source File

SOURCE=.\libs\SDLmain.lib
# End Source File
# Begin Source File

SOURCE=.\libs\zlibstat.lib
# End Source File
# Begin Source File

SOURCE="C:\Program Files\Microsoft Visual Studio\VC98\Lib\SHLWAPI.LIB"
# End Source File
# Begin Source File

SOURCE=.\libs\bgd.lib
# End Source File
# Begin Source File

SOURCE=.\libs\libxml2.lib
# End Source File
# Begin Source File

SOURCE=.\libs\NLstatic.lib
# End Source File
# Begin Source File

SOURCE=.\libs\SDL_mixer.lib
# End Source File
# Begin Source File

SOURCE=.\libs\lua5.1.lib
# End Source File
# Begin Source File

SOURCE=.\libs\dbghelp.lib
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "*"
# Begin Source File

SOURCE=..\..\share\OpenLieroX.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Group
# Begin Group "Game Core"

# PROP Default_Filter "cpp, h"
# Begin Source File

SOURCE=..\..\src\common\Command.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\con_cmd.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\Console.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Console.h
# End Source File
# Begin Source File

SOURCE=..\..\include\LieroX.h
# End Source File
# Begin Source File

SOURCE=..\..\src\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\Misc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\Networking.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Networking.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Options.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Options.h
# End Source File
# Begin Source File

SOURCE=..\..\src\client\Sounds.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Sounds.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Utils.h
# End Source File
# End Group
# End Target
# End Project
