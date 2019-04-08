#!/bin/bash
#
#echo '$0: '$0  
#echo "pwd: "`pwd`  
#echo "scriptPath1: "$(cd `dirname $0`; pwd)  
#echo "scriptPath2: "$(dirname $(readlink -f $0))

#WORKFOLDER=`pwd`
#echo "Workfolder: "$WORKFOLDER


show_help(){
	echo -e "build.sh [module] [operation]
module: proxy|control|firewall|process|data
operation:
	-r: to build release version 
	-d: to build debug version 
	-rz: build release to tar.gz
	-dz: build debug to tar.gz"
}


SCRIPTFOLDER=$(dirname $(readlink -f $0))
echo "ScriptFolder: "$SCRIPTFOLDER
# cd to current file folder
cd $SCRIPTFOLDER

# default value is debug
VERSION_STR=$(cat $SCRIPTFOLDER/../version)
MODULE=unknown

init(){
	MODULE=$1
	PROJECT=sloongnet_$MODULE
	MAKEFLAG=debug
	CMAKE_FILE_PATH=$SCRIPTFOLDER/../sloongnet/$MODULE
}

clean(){
	rm -rdf $MAKEFLAG/$PROJECT
}

build(){
	if [ $MODULE = "unknown" ]; then
		echo -e "Module error.\n"
		show_help
		exit
	fi
	if [ ! -d $MAKEFLAG  ];then
		mkdir $MAKEFLAG
	fi
	cd $MAKEFLAG
	if [ ! -d $PROJECT  ];then
		mkdir $PROJECT
	fi
	cd $PROJECT
	cmake -DCMAKE_BUILD_TYPE=$MAKEFLAG $CMAKE_FILE_PATH
	make
	cd ../../
}

build_debug(){
	OUTPATH=$PROJECT-debug-v$VERSION_STR
	MAKEFLAG=debug
	clean
	build
}

build_release(){
	OUTPATH=$PROJECT-v$VERSION_STR
	MAKEFLAG=release
	clean
	build
}

zip(){
	tar -cf $OUTPATH.tar -C $MAKEFLAG/ $PROJECT
	tar -rf $OUTPATH.tar -C $CMAKE_FILE_PATH/ scripts
	tar -rf $OUTPATH.tar -C $CMAKE_FILE_PATH/../ default.conf
	tar -rf $OUTPATH.tar install.sh
}


if [ $# -lt 1 ]; then
	show_help
	exit
fi

if [ $# -lt 2 ]; then
	case $1 in 
		proxy) init proxy;;
		control) init control;;
		firewall) init firewall;;
		process) init process;;
		data) init data;;
		* ) show_help;;
	esac
	build
else
	case $2 in 
		-r) build_release;;
		-d) build_debug;;
		-rz) 
			build_release
			zip;;
		-dz) 
			build_debug
			zip;;
		* ) show_help;;
	esac
fi
