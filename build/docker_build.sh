#!/bin/bash
### 
# @Author: WCB
 # @Date: 2019-12-11 14:28:05
 # @LastEditors: Chuanbin Wang
 # @LastEditTime: 2021-08-26 13:55:08
 # @Description: file content
 ###
pwd
SCRIPTFOLDER=$(dirname $(readlink -f $0))
echo "ScriptFolder: "$SCRIPTFOLDER
# cd to current file folder
cd $SCRIPTFOLDER/../

VERSION_STR=$(cat version)

./build/build.sh -c
docker build -t sloongnet:$VERSION_STR -f $SCRIPTFOLDER/../Dockerfile $SCRIPTFOLDER/../
docker tag sloongnet:$VERSION_STR sloongnet:latest
