mkdir debuginfo
mkdir debuginfo\%1
mkdir debuginfo\%1\sources
mkdir debuginfo\%1\sources\include
mkdir debuginfo\%1\sources\src
mkdir debuginfo\%1\sources\src\client
mkdir debuginfo\%1\sources\src\common
mkdir debuginfo\%1\sources\src\server
copy distrib\win32\OpenLieroX.exe debuginfo\%1\OpenLieroX.exe
copy distrib\win32\OpenLieroX.pdb debuginfo\%1\OpenLieroX.pdb
copy distrib\win32\OpenLieroX.map debuginfo\%1\OpenLieroX.map
copy distrib\win32\SDL.pdb debuginfo\%1\SDL.pdb
copy distrib\win32\SDL.dll debuginfo\%1\SDL.dll
copy distrib\win32\vc*.?db debuginfo\%1\
copy include\*.h debuginfo\%1\sources\include
copy src\*.cpp debuginfo\%1\sources\src
copy src\client\*.cpp debuginfo\%1\sources\src\client
copy src\common\*.cpp debuginfo\%1\sources\src\common
copy src\server\*.cpp debuginfo\%1\sources\src\server
copy src\client\*.c debuginfo\%1\sources\src\client
copy src\common\*.c debuginfo\%1\sources\src\common
copy src\server\*.c debuginfo\%1\sources\src\server
copy src\client\*.h debuginfo\%1\sources\src\client
copy src\common\*.h debuginfo\%1\sources\src\common
copy src\server\*.h debuginfo\%1\sources\src\server
echo Finished!