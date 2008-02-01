@echo off

if exist "%TEMP%\olx_ext.reg" del "%TEMP%\olx_ext.reg"

echo REGEDIT4                                                      >> "%TEMP%\olx_ext.reg"
echo [HKEY_CLASSES_ROOT\.OLXdemo]                                  >> "%TEMP%\olx_ext.reg"
echo @="OpenLieroX_DemoFile"                                       >> "%TEMP%\olx_ext.reg"
echo [HKEY_CLASSES_ROOT\OpenLieroX_DemoFile]                       >> "%TEMP%\olx_ext.reg"
echo @="OpenLieroX Demo File"                                      >> "%TEMP%\olx_ext.reg"
echo [HKEY_CLASSES_ROOT\OpenLieroX_DemoFile\DefaultIcon]           >> "%TEMP%\olx_ext.reg"
echo @="%CD:\=\\%\\OpenLieroX.exe,1"                               >> "%TEMP%\olx_ext.reg"
echo [HKEY_CLASSES_ROOT\OpenLieroX_DemoFile\Shell]                 >> "%TEMP%\olx_ext.reg"
echo @="Open"                                                      >> "%TEMP%\olx_ext.reg"
echo [HKEY_CLASSES_ROOT\OpenLieroX_DemoFile\Shell\Open]            >> "%TEMP%\olx_ext.reg"
echo @="Run"                                                       >> "%TEMP%\olx_ext.reg"
echo [HKEY_CLASSES_ROOT\OpenLieroX_DemoFile\Shell\Open\Command]    >> "%TEMP%\olx_ext.reg"
echo @="\"%CD:\=\\%\\OpenLieroX.exe\" \"%%1\""                     >> "%TEMP%\olx_ext.reg"

regedit /s "%TEMP%\olx_ext.reg"
