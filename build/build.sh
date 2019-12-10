#!/bin/bash
#
#echo '$0: '$0  
#echo "pwd: "`pwd`  
#echo "scriptPath1: "$(cd `dirname $0`; pwd)  
#echo "scriptPath2: "$(dirname $(readlink -f $0))

#WORKFOLDER=`pwd`
#echo "Workfolder: "$WORKFOLDER


show_help(){
	echo -e "build.sh [operation]
operation:
	-r: to build release version 
	-d: to build debug version 
	-rz: build release to zip file
	-dz: build debug to zip file"
}


SCRIPTFOLDER=$(dirname $(readlink -f $0))
echo "ScriptFolder: "$SCRIPTFOLDER
# cd to current file folder
cd $SCRIPTFOLDER
PROJECT=sloongnet
MAKEFLAG=debug
CMAKE_FILE_PATH=$SCRIPTFOLDER/../sloongnet

# default value is debug
VERSION_STR=$(cat $SCRIPTFOLDER/../version)



clean(){
	rm -rdf $MAKEFLAG/$PROJECT
}

build(){
	if [ ! -d $MAKEFLAG  ];then
		mkdir $MAKEFLAG
	fi
	cd $MAKEFLAG
	cmake -DCMAKE_BUILD_TYPE=$MAKEFLAG $CMAKE_FILE_PATH
	if [ $? -ne 0 ];then
		echo "Run cmake cmd return error. build stop."
		exit 1
	fi
	make
	if [ $? -ne 0 ];then
		echo "Run make cmd return error. build stop."
		exit 1
	fi

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

zipfile(){
	cd $CMAKE_FILE_PATH
	zip ../$OUTPATH.zip scripts/* 
	zip -gj ../$OUTPATH.zip include/*.so
	cd ..
	zip -gj $OUTPATH.zip $SCRIPTFOLDER/$MAKEFLAG/$PROJECT
}


# -eq 等于,
# -ne 不等于
# -gt 大于,
# -ge 大于等于,
# -lt 小于,
# -le 小于等于
if [ $# -eq 0 ]; then
	show_help
	exit
fi

if [ $# -eq 1 ]; then
	case $1 in 
		-r) 
			build_release
			;;
		-d) 
			build_debug
			;;
		-rz) 
			build_release
			zipfile
			;;
		-dz) 
			build_debug
			zipfile
			;;
		* ) 
			show_help
			;;
	esac
fi
