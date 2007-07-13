#!/bin/bash

# first of all, you need the following file:
# http://www.porpoisehead.net/mysw/downloads/svg-to-raster.scm
# put it into ~/.gimp-2.2/scripts
# further info: http://www.porpoisehead.net/mysw/index.php?pgid=gimp_svg

cd ../share
for f in *.svg; do
	echo -n "processing $f"
	for size in 16 32 64 128; do
		echo -n " ..."
		(
			echo "( svg-to-raster \"$f\" \"${f/svg/}$size.png\" 72 $size $size )"
			echo '(gimp-quit 0)'
		) | gimp --batch-interpreter plug_in_script_fu_eval -i -b - >/dev/null \
		&& echo -n " $size"
	done
	echo " done"
done

