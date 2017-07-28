#!/bin/sh

TARGET=$1

[ ! -d release ] && mkdir -p release
[ ! -d release/mx60 ] && mkdir -p release/mx60
[ ! -d release/om400 ] && mkdir -p release/om400
[ ! -d release/mx8 ] && mkdir -p release/mx8
[ ! -d release/wroc ] && mkdir -p release/wroc
[ ! -d release/x86 ] && mkdir -p release/x86

VERSION=`sed -n "/^\#define.*P2P_LIB_VERSION/s/.*\"\(.*\)\".*/\1/gp" include/common_util.h`
echo -e "building p2pnat lib version $VERSION"
LIB_FILE=libp2pnat.so

if [ "$TARGET" = "wroc" ];then
	export PATH=/opt/buildroot-gcc342/bin:$PATH
	make TARGET=wroc clean
	make TARGET=wroc
	if [ $? -eq 0 ];then
		cp ${LIB_FILE} release/wroc/
		echo -e "\n copy to release/wroc/\n"
	fi
elif [ "$TARGET" = "mx60" ];then
	make TARGET=mx60 clean
	make TARGET=mx60
	if [ $? -eq 0 ];then
		cp ${LIB_FILE} release/mx60/
		echo -e "\n copy to release/mx60/\n"
	fi
elif [ "$TARGET" = "om400" ];then
	export PATH=/opt/eabi/arm-2009q1/bin:/opt/eabi/arm-eabi-4.4.0/bin:$PATH
	make TARGET=om400 clean
	make TARGET=om400
	if [ $? -eq 0 ];then
		cp ${LIB_FILE} release/om400/
		echo -e "\n copy to release/om400/\n"
	fi
elif [ "$TARGET" = "mx8" ];then
	make TARGET=mx8 clean
	make TARGET=mx8
	if [ $? -eq 0 ];then
		cp ${LIB_FILE} release/mx8/
		echo -e "\n copy to release/mx8/\n"
	fi
elif [ "$TARGET" = "x86" ];then
	make TARGET=x86 clean
	make TARGET=x86
	if [ $? -eq 0 ];then
		cp ${LIB_FILE} release/x86/
		echo -e "\n copy to release/x86/\n"
	fi
elif [ "$TARGET" = "test" ];then
        ./p2pnat -s 54.223.244.5  -i test_p2p_client_S -c test_p2p_client_A -d
elif [ "$TARGET" = "upload__________" ];then
	VER=$2
	if [ -z ${VER} ];then
		echo -e "miss version number"
		exit
	fi
	if [ "${VER}" = "lib" ];then
		echo -e "upload lib"
		echo -e "bin\nha\nput lib/lib/mx60/libmqtt3as.so /CloudPlatform/mx60/cloudae/libmqtt3as.so\nbye\n" | ftp 192.168.20.164
		echo -e "bin\nha\nput lib/lib/om400/libmqtt3as.so /CloudPlatform/om400/cloudae/libmqtt3as.so\nbye\n" | ftp 192.168.20.164
		echo -e "bin\nha\nput lib/lib/mx8/libmqtt3as.so /CloudPlatform/mx8/cloudae/libmqtt3as.so\nbye\n" | ftp 192.168.20.164
		echo -e "bin\nha\nput lib/lib/gw4/libmqtt3as.so /CloudPlatform/cloudae/libmqtt3as.so\nbye\n" | ftp 192.168.20.164
		exit
	fi
	echo -e "bin\nha\nput release/mx8/cloudae_${VER} /CloudPlatform/mx8/cloudae/cloudae_${VER}\nbye\n" | ftp 192.168.20.164
	echo -e "bin\nha\nput release/mx60/cloudae_${VER} /CloudPlatform/mx60/cloudae/cloudae_${VER}\nbye\n" | ftp 192.168.20.164
	echo -e "bin\nha\nput release/om400/cloudae_${VER} /CloudPlatform/om400/cloudae/cloudae_${VER}\nbye\n" | ftp 192.168.20.164
	echo -e "bin\nha\nput release/wroc/cloudae_${VER} /CloudPlatform/cloudae/cloudae_${VER}\nbye\n" | ftp 192.168.20.164
elif [ "$TARGET" = "all" ];then
	$0 wroc
	$0 mx60
	$0 mx8
	$0 om400
	$0 x86
else
	echo -e "\nUsage:\n\t$0 [wroc|mx60|om400|mx8|all]\n"
	# echo -e "\t$0 upload <ver>\n"
fi


