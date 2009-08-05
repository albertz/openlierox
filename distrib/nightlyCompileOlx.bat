@rem This file contains host config like directories, the main work is done in Bash script

@echo off
if exist x:\compileOlx.flag goto compile

exit

:compile

del x:\compileOlx.flag

echo ----------- Nightly build started at %DATE% > x:\nightlyCompileOlx.log
if not -%1==- echo Do not doing shutdown

call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
echo on

cd c:\openlierox

bash -c "c:/openlierox/distrib/nightlyCompileOlx.sh x:/nightlyCompileOlx.log c:/lxalliance.net.cookie x:/releases"

echo ----------- Nightly build finished at %DATE% >> x:\nightlyCompileOlx.log

if -%1==- shutdown -s now
