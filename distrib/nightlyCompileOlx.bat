@rem This script requires Cmake, grep and svn command-line tools
@rem Ideally it requires Bash and Cygwin, but that's optional

echo on
if exist x:\compileOlx.flag goto compile

exit

:compile

del x:\compileOlx.flag

echo ----------- Nightly build started at %DATE% > x:\nightlyCompileOlx.log

call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
echo on

cd c:\openlierox
svn log -r "BASE:HEAD" > svn.log
svn up
grep -i "\[Rebuild\]" svn.log

if errorlevel 1 goto end

echo ----------- Compiling >> x:\nightlyCompileOlx.log

cd "build\msvc 2005"

del ..\..\distrib\win32\OpenLieroX.exe

rem You can compile without Cmake, then substitute openlierox.vcproj with Game.vcproj for vcbuild
cmake -G "Visual Studio 8 2005" -D DEBUG=0 ../..

vcbuild openlierox.vcproj "RelWithDebInfo|Win32" /useenv /showenv >> x:\nightlyCompileOlx.log 2>&1

if exist ..\..\distrib\win32\OpenLieroX.exe goto commit

echo ----------- Compiling failed >> x:\nightlyCompileOlx.log

goto end

:commit

echo ----------- Committing to SVN >> x:\nightlyCompileOlx.log

cd ..\..

svn commit -m "Updated EXE (auto nightly build)"

bash -c "echo `grep -o 'r[0-9]*' optional-includes/generated/Version_generated.h` > revnum.txt"

bash -c "cmd /c save_debug_info.bat `cat revnum.txt`"
bash -c "zip -r debuginfo_`cat revnum.txt`.zip debuginfo/`cat revnum.txt`"
rem bash -c "echo Debug symbols attached | email --attach debuginfo_`cat revnum.txt`.zip --subject 'Debug symbols automated mail' --smtp-server 172.17.57.25 karel.petranek@tiscali.cz"

:end

echo ----------- Nightly build finished at %DATE% >> x:\nightlyCompileOlx.log

shutdown -s now
