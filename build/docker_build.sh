#!/bin/bash
### 
# @Author: WCB
 # @Date: 2019-12-11 14:28:05
 # @LastEditors  : WCB
 # @LastEditTime : 2019-12-26 11:22:04
 # @Description: file content
 ###
pwd
SCRIPTFOLDER=$(dirname $(readlink -f $0))
echo "ScriptFolder: "$SCRIPTFOLDER
# cd to current file folder
cd $SCRIPTFOLDER/../

VERSION_STR=$(cat version)

docker build -t sloongnet:$VERSION_STR -f $SCRIPTFOLDER/../Dockerfile $SCRIPTFOLDER/../
docker tag sloongnet:$VERSION_STR sloongnet:latest
