ECHO OFF
IF NOT EXIST CHOICE GOTO DEVCPP_RUN
CHOICE Do you want to delete Dev-Cpp intermediate files (*.o)?
IF ERRORLEVEL == 0 GOTO MSVC_REL
:DEVCPP_RUN
ECHO Deleting Dev-Cpp intermediate files, please wait...
del ".\src\*.o" /S /Q

:MSVC_REL
IF NOT EXIST CHOICE GOTO MSVC_REL_RUN
CHOICE Do you want to delete VC++ intermediate release files (obj/Release)?
IF ERRORLEVEL == 0 GOTO MSVC_DBG
:MSVC_REL_RUN
ECHO Deleting VC++ intermediate release files, please wait...
del ".\build\msvc\obj\release\*.*" /Q
del ".\build\msvc 2005\obj\release\*.*" /Q
del ".\build\msvc 2008\obj\release\*.*" /Q

:MSVC_DBG
IF NOT EXIST CHOICE GOTO MSVC_DBG_RUN
CHOICE Do you want to delete VC++ intermediate debug files (obj/Debug)?
IF ERRORLEVEL == 0 GOTO END
:MSVC_DBG_RUN
ECHO Deleting VC++ intermediate debug files, please wait...
del ".\build\msvc\obj\debug\*.*" /Q
del ".\build\msvc 2005\obj\debug\*.*" /Q
del ".\build\msvc 2008\obj\debug\*.*" /Q

:END
ECHO Cleanup successful!
CD "%~dp0"
PAUSE
ECHO ON