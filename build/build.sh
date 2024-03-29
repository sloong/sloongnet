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
CMAKEFLAG=Debug
CMAKE_FILE_PATH=$SCRIPTFOLDER/../src
CPU_NUM=`grep -c "model name" /proc/cpuinfo`

# default value is debug
VERSION_STR=$(cat $SCRIPTFOLDER/../version)

MAKE_CMD="make -j$CPU_NUM"

echo "System has $CPU_NUM cpus installed"

clean(){
	if [ -d $BUILD_FOLDER  ];then
		rm -rdf $BUILD_FOLDER
	fi
}

build_path(){
	BUILD_FOLDER=$SCRIPTFOLDER/cache/$MAKEFLAG
	OUTPUT_FOLDER=$SCRIPTFOLDER/$MAKEFLAG
	
}

check_folder(){
	if [ ! -d $BUILD_FOLDER  ];then
		mkdir -p $BUILD_FOLDER
	fi
	if [ ! -d $OUTPUT_FOLDER/modules ];then
		mkdir -p $OUTPUT_FOLDER/modules
	fi
}

build(){
	check_folder
	cd $BUILD_FOLDER
	cmake -DCMAKE_TOOLCHAIN_FILE=$SCRIPTFOLDER/clang.cmake -DCMAKE_BUILD_TYPE=$CMAKEFLAG $CMAKE_FILE_PATH
	if [ $? -ne 0 ];then
		echo "Run cmake cmd return error. build stop."
		exit 1
	fi

	
	if [ $? -ne 0 ];then
		echo "Run make cmd return error. build stop."
		exit 1
	fi
	echo $MAKE_CMD
	$MAKE_CMD
	cp $PROJECT $OUTPUT_FOLDER/
	cp libcore.so $OUTPUT_FOLDER/
	cp libmanager.so $OUTPUT_FOLDER/
	cp modules/*.so $OUTPUT_FOLDER/modules/
}

build_debug(){
	echo "Build DEBUG"
	OUTPATH=$SCRIPTFOLDER/$PROJECT-debug
	MAKEFLAG=debug
	CMAKEFLAG=Debug
	build_path
	# clean
	build
}


clean_cache(){
	if [ -d $SCRIPTFOLDER/cache  ];then
		rm -rdf $SCRIPTFOLDER/cache
	fi
}

clean_all(){
	clean_cache
	if [ -d $SCRIPTFOLDER/debug  ];then
		rm -rdf $SCRIPTFOLDER/debug
	fi
	if [ -d $SCRIPTFOLDER/release  ];then
		rm -rdf $SCRIPTFOLDER/release
	fi
}

build_release(){
	echo "Build RELEASE"
	OUTPATH=$SCRIPTFOLDER/$PROJECT-release
	MAKEFLAG=release
	CMAKEFLAG=Release
	build_path
	clean
	build
}

build_ci(){
	MAKE_CMD=make
	OUTPATH=$SCRIPTFOLDER/$PROJECT-debug
	MAKEFLAG=debug
	CMAKEFLAG=Debug
	build_path
	# clean
	build
}


copyfile(){
	mkdir -p $OUTPATH/modules/

	cp $SCRIPTFOLDER/$MAKEFLAG/$PROJECT $OUTPATH/
	cp $SCRIPTFOLDER/$MAKEFLAG/libcore.so $OUTPATH/
	cp $SCRIPTFOLDER/$MAKEFLAG/modules/*.so $OUTPATH/modules/
}

zipfile(){
	copyfile

	OUTFILE=$OUTPATH-v$VERSION_STR

	cd $OUTPATH
	tar -rv -f $OUTFILE.tar *.so
	tar -rv -f $OUTFILE.tar $PROJECT
	tar -rv -f $OUTFILE.tar modules/*.so 
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
		-ci) 
			build_ci
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
		-ca)
			clean_all
			;;
		-c)
			clean_cache
			;;
		* ) 
			show_help
			;;
	esac
fi
