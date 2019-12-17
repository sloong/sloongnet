#!/bin/bash
### 
# @Author: WCB
 # @Date: 2019-12-11 14:28:05
 # @LastEditors: WCB
 # @LastEditTime: 2019-12-17 16:00:26
 # @Description: file content
 ###
 
SCRIPTFOLDER=$(dirname $(readlink -f $0))
echo "ScriptFolder: "$SCRIPTFOLDER
# cd to current file folder
cd $SCRIPTFOLDER

VERSION_STR=$(cat ../version)

cd ..
zip output.zip ./* 
zip -d output.zip "docker"
cd docker
cp ../dist/Sloongnet-WebUI-$VERSION_STR.tar.gz ./output.tar.gz
sudo docker build -t sloong/sloongnet-webui:$VERSION_STR .