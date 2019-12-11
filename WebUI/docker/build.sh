#!/bin/bash
### 
# @Author: WCB
 # @Date: 2019-12-11 14:28:05
 # @LastEditors: WCB
 # @LastEditTime: 2019-12-11 15:32:11
 # @Description: file content
 ###
 
SCRIPTFOLDER=$(dirname $(readlink -f $0))
echo "ScriptFolder: "$SCRIPTFOLDER
# cd to current file folder
cd $SCRIPTFOLDER

VERSION_STR=$(cat ../version)

ptyhon3 ../setup.py sdist
cp ../dist/Sloonget-WebUI-$VERSION_STR.tar.gz ./output.tar.gz
sudo docker build -t sloong/sloongnet-webui:$VERSION_STR .