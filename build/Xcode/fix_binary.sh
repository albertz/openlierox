#!/bin/bash

cd build

for rel in *; do
	cd $rel
	if [ -e OpenLieroX.app ]; then

		echo ">>> copying gamedir"
		rsync -avP --exclude=gmon.out --exclude=.svn ../../../../share/gamedir OpenLieroX.app/Contents/Resources/

		for framework in \
			GD.framework/Versions/2.0/GD \
			UnixImageIO.framework/Versions/A/UnixImageIO \
			UnixImageIO.framework/Versions/B/UnixImageIO \
			FreeType.framework/Versions/2.3/FreeType;
		do
			echo ">>> fixing $framework"
			install_name_tool -change /Library/Frameworks/$framework @executable_path/../Frameworks/$framework \
				OpenLieroX.app/Contents/MacOS/OpenLieroX

			fr_path=$(echo $framework | cut -d / -f 1,2,3 -) 
			files="OpenLieroX.app/Contents/Frameworks/$framework"
			[ -e OpenLieroX.app/Contents/Frameworks/$fr_path/Libraries ] && \
				files=$(echo $files OpenLieroX.app/Contents/Frameworks/$fr_path/Libraries/*)
			files=${files//"OpenLieroX.app\/Contents\/Frameworks"//Library/Frameworks}
			files=$(echo $files; \
				otool -L OpenLieroX.app/Contents/Frameworks/$framework | \
				grep "/Library/Frameworks/" | grep -v "/System/Library" | \
				cut -d " " -f 1)
			for lib in $files
			do
				echo "> fixing link $lib"
				for file in $files; do
					install_name_tool -change $lib ${lib/\/Library\/Frameworks/@executable_path/../Frameworks} \
						${file/\/Library\/Frameworks/OpenLieroX.app/Contents/Frameworks}
				done
			done
		done
	fi
	cd ..
done
