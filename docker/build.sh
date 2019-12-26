#!/bin/bash
### 
# @Author: WCB
 # @Date: 2019-12-11 14:28:05
 # @LastEditors  : WCB
 # @LastEditTime : 2019-12-26 11:22:04
 # @Description: file content
 ###
sudo pwd
SCRIPTFOLDER=$(dirname $(readlink -f $0))
echo "ScriptFolder: "$SCRIPTFOLDER
# cd to current file folder
cd $SCRIPTFOLDER

VERSION_STR=$(cat ../version)

../build/build.sh -rz
PROJECT=sloongnet

mv ../build/$PROJECT-v$VERSION_STR.tar output.tar

sudo docker build -t sloongnet:$VERSION_STR .
sudo docker tag sloongnet:$VERSION_STR sloongnet:latest