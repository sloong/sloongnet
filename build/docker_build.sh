#!/bin/bash
### 
# @Author: WCB
 # @Date: 2019-12-11 14:28:05
 # @LastEditors: Chuanbin Wang
 # @LastEditTime: 2021-09-15 13:39:03
 # @Description: file content
 ###
pwd
SCRIPTFOLDER=$(dirname $(readlink -f $0))
echo "ScriptFolder: "$SCRIPTFOLDER
# cd to current file folder
cd $SCRIPTFOLDER/../


build(){
    VERSION_STR=$(cat version)

    ./build/build.sh -c
    docker build -t sloong/sloongnet:$VERSION_STR -f $SCRIPTFOLDER/../Dockerfile $SCRIPTFOLDER/../
    docker tag sloong/sloongnet:$VERSION_STR sloong/sloongnet:latest
}


build_image() {
    docker build -t sloong/sloongnet_build:latest -f $SCRIPTFOLDER/Build.Dockerfile .
    docker push sloong/sloongnet_build
}


run_image() {
    docker build -t sloong/sloongnet_run -f $SCRIPTFOLDER/Run.Dockerfile .
    docker push sloong/sloongnet_run
}


if [ $# -eq 0 ]; then
	build
fi

if [ $# -eq 1 ]; then
	case $1 in 
		--build) 
			build_image
			;;
		--run) 
			run_image
			;;
		* ) 
			build
			;;
	esac
fi
