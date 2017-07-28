#!/bin/sh
[! -d release ] && mkdir -p release
[! -d release/mx60] && mkdir -p release/mx60
[! -d release/om400] && mkdir -p release/om400
[! -d release/mx8] && mkdir -p release/mx8
[! -d release/wroc] && mkdir -p release/wroc

export PATH=/tmp:$PATH

if [ "$1" == "wroc"]
then
export PATH=$PATH:/opt/buildroot-gcc342/bin
elif ["$1" == "mx8"]
then
export PATH=/opt/eldk3/usr/bin:$PATH
elif ["$1" = "mx120"];then
export PATH=/opt/eldk4/usr/bin:$PATH
elif ["$1" = "om400"];then
export PATH=/opt/eabi/arm-2009q1/bin:/opt/eabi/arm-eabi-4.4.0/bin:$PATH
fi

echo 'scons'
if [ "$TARGET" = "upload" ];then
VER=$2
if [ -z ${VER} ];then
echo -e "miss version number"
exit
fi
echo -e "bin\nha\nput release/FSP/fspd/wroc/fspd_${VER}.bz2 /FSP/fspd/wroc/fspd_${VER}.bz2\nbye\n" | ftp 192.168.20.164
fi

