#!/bin/bash

cd include

mv PrecompiledHeader.hpp PrecompiledHeader.hpp.old
grep -v "#include" PrecompiledHeader.hpp.old >PrecompiledHeader.hpp
rm PrecompiledHeader.hpp.old

for h in *.h; do echo "#include \"$h\""; done | \
grep -v "CWorld.h" | \
grep -v "UCString.h" | \
grep -v "win32memleakdebug.h" >> \
PrecompiledHeader.hpp
