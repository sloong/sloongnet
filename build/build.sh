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



clean(){
	cd $SCRIPTFOLDER/$MAKEFLAG
	rm -rdf $MAKEFLAG/$PROJECT
	rm -rdf CMakeFiles
	rm -f CMakeCache.txt
	rm -f Makefile
	rm -rdf modules
	rm cmake_install.cmake
	if [ -d $OUTPATH  ];then
		rm -rdf $OUTPATH
	fi
}

swtich_folder(){
	if [ ! -d $MAKEFLAG  ];then
		mkdir $MAKEFLAG
	fi
	cd $MAKEFLAG
}

build(){
	cmake -DCMAKE_TOOLCHAIN_FILE=$SCRIPTFOLDER/clang.cmake -DCMAKE_BUILD_TYPE=$CMAKEFLAG $CMAKE_FILE_PATH
	if [ $? -ne 0 ];then
		echo "Run cmake cmd return error. build stop."
		exit 1
	fi

	make -j$CPU_NUM
	if [ $? -ne 0 ];then
		echo "Run make cmd return error. build stop."
		exit 1
	fi
}

build_debug(){
	OUTPATH=$SCRIPTFOLDER/$PROJECT-debug
	MAKEFLAG=debug
	CMAKEFLAG=Debug
	swtich_folder
	# clean
	build
}

clean_all(){
	OUTPATH=$SCRIPTFOLDER/$PROJECT-debug
	MAKEFLAG=debug
	clean
	# OUTPATH=$SCRIPTFOLDER/$PROJECT-release
	# MAKEFLAG=release
	# clean
}

build_release(){
	OUTPATH=$SCRIPTFOLDER/$PROJECT-release
	MAKEFLAG=release
	CMAKEFLAG=Release
	clean
	build
}

zipfile(){
	mkdir -p $OUTPATH/modules/

	cp $CMAKE_FILE_PATH/referenced/libuniv/libuniv.so $OUTPATH/
	cp $SCRIPTFOLDER/$MAKEFLAG/$PROJECT $OUTPATH/
	cp $SCRIPTFOLDER/$MAKEFLAG/libcore.so $OUTPATH/
	cp $SCRIPTFOLDER/$MAKEFLAG/modules/*.so $OUTPATH/modules/

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
		-c)
			clean_all
			;;
		* ) 
			show_help
			;;
	esac
fi
