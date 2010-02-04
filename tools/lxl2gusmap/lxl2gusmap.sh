#!/bin/zsh

function usage() {
	echo "usage: $1 lxl-file"
	exit 1
}

lxl="$1"
if [ "$lxl" = "" -o ! -e "$lxl" ]; then
	usage $0
fi

cp $lxl . 2>/dev/null && rmlxl=true || rmlxl=false
lxl=$(basename $lxl)

if [ ! -e $lxl ]; then
	echo "cannot copy $lxl to $(pwd)"
	exit 1
fi

leveldecomp=$(dirname $0)/../LevelDecompiler/bin/LevelDecompiler
$leveldecomp $lxl 2>&1
$rmlxl && rm $lxl

lxlbase=$lxl:r

front=${lxlbase}_front.bmp
back=${lxlbase}_back.bmp
mat=${lxlbase}_mat.bmp

for f in $front $back $mat; do
	if [ ! -e $f ]; then
		echo "decompile failed, did not find $f"
		exit 1
	fi
done

convert $front $front:r.png && rm $front && front=$front:r.png
convert $back $back:r.png && rm $back && back=$back:r.png

# LevelDecompiler:
# 128 - rock
# 0 - background
# 255 - dirt
# Gusanos indexes:
# 0 - rock
# 1 - background
# 2 - dirt


#-fill "rgb(1,1,1)" -opaque "rgb(0,0,0)" \
#-fill "rgb(0,0,0)" -opaque "rgb(128,128,128)" \
#-fill "rgb(2,2,2)" -opaque "rgb(255,255,255)" \
#-type palette \
#-colors 256 \
#-colorspace Gray \

mogrify -fill "rgb(1,1,1)" -opaque "rgb(0,0,0)" $mat
mogrify -fill "rgb(2,2,2)" -opaque "rgb(255,255,255)" $mat
mogrify -fill "rgb(0,0,0)" -opaque "rgb(128,128,128)" $mat

# DAMN
convert $mat \
-colorspace Gray \
-define png:color-type=indexed \
-colors 256 \
\
$mat:r.png \
&& rm $mat && mat=$mat:r.png



mkdir $lxlbase 2>/dev/null
mv $front $lxlbase/level.png
mv $back $lxlbase/paralax.png
mv $mat $lxlbase/material.png
touch $lxlbase/config.cfg

