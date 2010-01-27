#!/bin/bash

cd include

mv PrecompiledHeader.hpp PrecompiledHeader.hpp.old
grep -v "#include\|#endif // !defined(__cplusplus)" PrecompiledHeader.hpp.old >PrecompiledHeader.hpp
rm PrecompiledHeader.hpp.old

# SkinnedGUI seems broken
for h in *.h DeprecatedGUI/*.h ; do
echo "#include \"$h\""; done | \
grep -v "CWorld.h" | \
grep -v "UCString.h" | \
grep -v "_generated.h" | \
grep -v "win32memleakdebug.h" >> \
PrecompiledHeader.hpp
echo "#endif // !defined(__cplusplus)" >> PrecompiledHeader.hpp
