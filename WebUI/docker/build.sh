#!/bin/bash
### 
# @Author: WCB
 # @Date: 2019-12-11 14:28:05
 # @LastEditors  : WCB
 # @LastEditTime : 2019-12-26 11:26:52
 # @Description: file content
 ###
sudo pwd
SCRIPTFOLDER=$(dirname $(readlink -f $0))
echo "ScriptFolder: "$SCRIPTFOLDER
# cd to current file folder
cd $SCRIPTFOLDER

VERSION_STR=$(cat ../../version)

cd ..
tar -zcvf output.tar.gz ./*
mv output.tar.gz docker/output.tar.gz
cd docker
sudo docker build -t sloongnet-webui:$VERSION_STR .
sudo docker tag sloongnet-webui:$VERSION_STR sloongnet-webui:latest