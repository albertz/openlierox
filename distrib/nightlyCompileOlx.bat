echo on
if exist x:\compileOlx.flag goto compile

exit

:compile

del x:\compileOlx.flag

echo Building OLX at %DATE% > x:\nightlyCompileOlx.log

call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
echo on

cd c:\openlierox
svn up

cd "build\msvc 2005"

del ..\..\distrib\win32\OpenLieroX.exe

rem You can compile without Cmake, then substitute openlierox.vcproj with Game.vcproj for vcbuild
cmake -G "Visual Studio 8 2005" -D DEBUG=0 ../..

vcbuild openlierox.vcproj "RelWithDebInfo|Win32" /useenv >> x:\nightlyCompileOlx.log 2>&1

if exist ..\..\distrib\win32\OpenLieroX.exe goto commit

echo Failed to build OLX >> x:\nightlyCompileOlx.log

goto end

:commit

cd ..\..

svn commit -m "Updated EXE (auto nightly build)"

rem Requires Cygwin!
bash -c "echo `grep -o 'r[0-9]*' optional-includes/generated/Version_generated.h` > revnum.txt"

bash -c "cmd /c save_debug_info.bat `cat revnum.txt`"
bash -c "zip -r debuginfo_`cat revnum.txt`.zip debuginfo/`cat revnum.txt`"
rem bash -c "echo Debug symbols attached | email --attach debuginfo_`cat revnum.txt`.zip --subject 'Debug symbols automated mail' --smtp-server 172.17.57.25 karel.petranek@tiscali.cz"

:end

shutdown -s now
