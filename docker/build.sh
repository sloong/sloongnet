#!/bin/bash
### 
# @Author: WCB
 # @Date: 2019-12-11 14:28:05
 # @LastEditors: WCB
 # @LastEditTime: 2019-12-17 16:00:26
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

mv ../$PROJECT-v$VERSION_STR.tar.gz output.tar.gz

sudo docker build -t sloong/sloongnet:$VERSION_STR .