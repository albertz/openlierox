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
# PROP Output_Dir "obj/Release"
# PROP Intermediate_Dir "obj/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "include" /I "blast" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBC" /out:"obj/Release/LieroX.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Game - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "obj/Debug"
# PROP Intermediate_Dir "obj/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib wsock32.lib /nologo /subsystem:windows /debug /debugtype:both /machine:I386 /nodefaultlib:"LIBC" /nodefaultlib:"msvcrt.lib" /MAPINFO:LINES /MAPINFO:EXPORTS
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "Game - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Game___Win32_Profile"
# PROP BASE Intermediate_Dir "Game___Win32_Profile"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/debug"
# PROP Intermediate_Dir "obj/debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /profile /machine:I386 /out:"obj/Release/LieroX.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /profile /machine:I386 /nodefaultlib:"LIBC" /out:"obj/Debug/LieroX.exe"

!ENDIF 

# Begin Target

# Name "Game - Win32 Release"
# Name "Game - Win32 Debug"
# Name "Game - Win32 Profile"
# Begin Group "System Files"

# PROP Default_Filter "cpp, h"
# Begin Source File

SOURCE=.\common\2xsai.cpp
# End Source File
# Begin Source File

SOURCE=.\include\2xsai.h
# End Source File
# Begin Source File

SOURCE=.\client\AuxLib.cpp
# End Source File
# Begin Source File

SOURCE=.\include\AuxLib.h
# End Source File
# Begin Source File

SOURCE=.\include\bass.h
# End Source File
# Begin Source File

SOURCE=.\client\Cache.cpp
# End Source File
# Begin Source File

SOURCE=.\include\Cache.h
# End Source File
# Begin Source File

SOURCE=.\common\CBytestream.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CBytestream.h
# End Source File
# Begin Source File

SOURCE=.\common\CChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CChannel.h
# End Source File
# Begin Source File

SOURCE=.\client\CFont.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CFont.h
# End Source File
# Begin Source File

SOURCE=.\client\CInput.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CInput.h
# End Source File
# Begin Source File

SOURCE=.\common\ConfigHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CrashRpt.h
# End Source File
# Begin Source File

SOURCE=.\include\CVec.h
# End Source File
# Begin Source File

SOURCE=.\include\defs.h
# End Source File
# Begin Source File

SOURCE=.\client\Error.cpp
# End Source File
# Begin Source File

SOURCE=.\include\Error.h
# End Source File
# Begin Source File

SOURCE=.\common\FindFile.cpp
# End Source File
# Begin Source File

SOURCE=.\include\FindFile.h
# End Source File
# Begin Source File

SOURCE=.\include\gd\gd.h
# End Source File
# Begin Source File

SOURCE=.\client\GfxPrimitives.cpp
# End Source File
# Begin Source File

SOURCE=.\include\GfxPrimitives.h
# End Source File
# Begin Source File

SOURCE=.\client\Graphics.cpp
# End Source File
# Begin Source File

SOURCE=.\include\Graphics.h
# End Source File
# Begin Source File

SOURCE=.\common\Ini.cpp
# End Source File
# Begin Source File

SOURCE=.\include\Ini.h
# End Source File
# Begin Source File

SOURCE=.\common\MathLib.cpp
# End Source File
# Begin Source File

SOURCE=.\include\MathLib.h
# End Source File
# Begin Source File

SOURCE=.\common\mmgr.cpp

!IF  "$(CFG)" == "Game - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Game - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Game - Win32 Profile"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\nl.h
# End Source File
# Begin Source File

SOURCE=.\include\RandomNumberList.h
# End Source File
# Begin Source File

SOURCE=.\common\Timer.cpp
# End Source File
# Begin Source File

SOURCE=.\include\Timer.h
# End Source File
# Begin Source File

SOURCE=.\include\types.h
# End Source File
# End Group
# Begin Group "Game files"

# PROP Default_Filter "cpp; h"
# Begin Source File

SOURCE=.\server\CBanList.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CBanList.h
# End Source File
# Begin Source File

SOURCE=.\common\CBonus.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CBonus.h
# End Source File
# Begin Source File

SOURCE=.\client\CChatBox.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CChatBox.h
# End Source File
# Begin Source File

SOURCE=.\common\CClient.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CClient.h
# End Source File
# Begin Source File

SOURCE=.\client\CClient_Draw.cpp
# End Source File
# Begin Source File

SOURCE=.\client\CClient_Game.cpp
# End Source File
# Begin Source File

SOURCE=.\client\CClient_Parse.cpp
# End Source File
# Begin Source File

SOURCE=.\client\CClient_Send.cpp
# End Source File
# Begin Source File

SOURCE=.\common\CGameScript.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CGameScript.h
# End Source File
# Begin Source File

SOURCE=.\common\CMap.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CMap.h
# End Source File
# Begin Source File

SOURCE=.\client\CNinjaRope.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CNinjaRope.h
# End Source File
# Begin Source File

SOURCE=.\common\CProjectile.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CProjectile.h
# End Source File
# Begin Source File

SOURCE=.\server\CServer.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CServer.h
# End Source File
# Begin Source File

SOURCE=.\server\CServer_Game.cpp
# End Source File
# Begin Source File

SOURCE=.\server\CServer_Parse.cpp
# End Source File
# Begin Source File

SOURCE=.\server\CServer_Send.cpp
# End Source File
# Begin Source File

SOURCE=.\common\CShootList.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CShootList.h
# End Source File
# Begin Source File

SOURCE=.\client\CViewport.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CViewport.h
# End Source File
# Begin Source File

SOURCE=.\client\CWeather.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CWeather.h
# End Source File
# Begin Source File

SOURCE=.\common\CWorm.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CWorm.h
# End Source File
# Begin Source File

SOURCE=.\common\CWorm_AI.cpp

!IF  "$(CFG)" == "Game - Win32 Release"

!ELSEIF  "$(CFG)" == "Game - Win32 Debug"

# SUBTRACT CPP /WX

!ELSEIF  "$(CFG)" == "Game - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common\CWorm_SendRecv.cpp
# End Source File
# Begin Source File

SOURCE=.\common\CWorm_Simulate.cpp
# End Source File
# Begin Source File

SOURCE=.\common\CWpnRest.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CWpnRest.h
# End Source File
# Begin Source File

SOURCE=.\client\Entity.cpp
# End Source File
# Begin Source File

SOURCE=.\include\Entity.h
# End Source File
# Begin Source File

SOURCE=.\include\GameRules.h
# End Source File
# Begin Source File

SOURCE=.\client\ProfileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\include\ProfileSystem.h
# End Source File
# Begin Source File

SOURCE=.\include\Protocol.h
# End Source File
# Begin Source File

SOURCE=.\include\SVClient.h
# End Source File
# End Group
# Begin Group "Frontend"

# PROP Default_Filter "cpp; h"
# Begin Source File

SOURCE=.\client\CBrowser.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CBrowser.h
# End Source File
# Begin Source File

SOURCE=.\client\CButton.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CButton.h
# End Source File
# Begin Source File

SOURCE=.\client\CCheckbox.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CCheckbox.h
# End Source File
# Begin Source File

SOURCE=.\client\CCombobox.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CCombobox.h
# End Source File
# Begin Source File

SOURCE=.\client\CGuiLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CGuiLayout.h
# End Source File
# Begin Source File

SOURCE=.\common\CGUISkin.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CGUISkin.h
# End Source File
# Begin Source File

SOURCE=.\client\CInputBox.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CInputBox.h
# End Source File
# Begin Source File

SOURCE=.\include\CLabel.h
# End Source File
# Begin Source File

SOURCE=.\client\CListview.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CListview.h
# End Source File
# Begin Source File

SOURCE=.\client\CMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CMenu.h
# End Source File
# Begin Source File

SOURCE=.\client\CScrollbar.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CScrollbar.h
# End Source File
# Begin Source File

SOURCE=.\client\CSlider.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CSlider.h
# End Source File
# Begin Source File

SOURCE=.\client\CTextbox.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CTextbox.h
# End Source File
# Begin Source File

SOURCE=.\client\CTitleButton.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CTitleButton.h
# End Source File
# Begin Source File

SOURCE=.\client\CWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\include\CWidget.h
# End Source File
# Begin Source File

SOURCE=.\include\Menu.h
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Local.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Main.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Mapeditor.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Net.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Net_Favourites.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Net_Host.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Net_Internet.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Net_Join.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Net_Lan.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Net_Main.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Net_News.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Options.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Menu_Player.cpp
# End Source File
# Begin Source File

SOURCE=.\client\MenuSystem.cpp
# End Source File
# End Group
# Begin Group "Libraries"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=.\libs\bass.lib
# End Source File
# Begin Source File

SOURCE=.\libs\HawkNL.lib
# End Source File
# Begin Source File

SOURCE=.\libs\NLstatic.lib
# End Source File
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

SOURCE="..\..\..\..\Program Files\Microsoft Visual Studio\VC98\Lib\SHLWAPI.LIB"
# End Source File
# Begin Source File

SOURCE=.\libs\CrashRpt.lib
# End Source File
# Begin Source File

SOURCE=.\libs\bgd.lib
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "*"
# Begin Source File

SOURCE=.\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Group
# Begin Group "Blast"

# PROP Default_Filter "cpp, h"
# End Group
# Begin Group "Game Core"

# PROP Default_Filter "cpp, h"
# Begin Source File

SOURCE=.\common\Command.cpp
# End Source File
# Begin Source File

SOURCE=.\include\con_cmd.h
# End Source File
# Begin Source File

SOURCE=.\common\Console.cpp
# End Source File
# Begin Source File

SOURCE=.\include\Console.h
# End Source File
# Begin Source File

SOURCE=.\client\main.cpp
# End Source File
# Begin Source File

SOURCE=.\include\main.h
# End Source File
# Begin Source File

SOURCE=.\common\Misc.cpp
# End Source File
# Begin Source File

SOURCE=.\include\Misc.h
# End Source File
# Begin Source File

SOURCE=.\common\Networking.cpp
# End Source File
# Begin Source File

SOURCE=.\include\Networking.h
# End Source File
# Begin Source File

SOURCE=.\client\Options.cpp
# End Source File
# Begin Source File

SOURCE=.\include\Options.h
# End Source File
# Begin Source File

SOURCE=.\client\Sounds.cpp
# End Source File
# Begin Source File

SOURCE=.\include\Sounds.h
# End Source File
# End Group
# End Target
# End Project
