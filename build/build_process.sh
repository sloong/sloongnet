#!/bin/bash
#
echo '$0: '$0  
echo "pwd: "`pwd`  
echo "scriptPath1: "$(cd `dirname $0`; pwd)  
echo "scriptPath2: "$(dirname $(readlink -f $0))


show_help(){
	echo -e "param error! \n-r: to build release version \n-d: to build debug version \n-rz: build release to tar.gz\n-dz: build debug to tar.gz"
}

WORKFOLDER=`pwd`
echo "Workfolder: "$WORKFOLDER
# cd to current file folder
cd `dirname $0`


# default value is debug
VERSION_STR=$(cat $WORKFOLDER/version)
MODULE=process
PROJECT=sloongnet_$MODULE
MAKEFLAG=debug
CMAKE_FILE_PATH=$WORKFOLDER/../sloongnet/$MODULE

clean(){
	rm -rdf $MAKEFLAG/$PROJECT
}

build(){
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


if [ $# -lt 1 ]
then
	build
else
	case $1 in 
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
