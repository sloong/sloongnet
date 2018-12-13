#!/bin/bash
#
show_help(){
	echo -e "param error! \n-r: to build release version \n-d: to build debug version \n-rz: build release to tar.gz\n-dz: build debug to tar.gz"
}

# default value is debug
VERSION_STR=$(cat ../version)
PROJECT=sloongnet
MAKEFLAG=debug
CMAKE_FILE_PATH=../$PROJECT

clean(){
	rm -rdf $MAKEFLAG
}

build(){
	if [ ! -d $MAKEFLAG  ];then
		mkdir $MAKEFLAG
	fi
	cd $MAKEFLAG
	cmake -DCMAKE_BUILD_TYPE=$MAKEFLAG ../$CMAKE_FILE_PATH
	make
	cd ../
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
