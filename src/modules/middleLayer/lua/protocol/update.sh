#!/bin/bash
#
### 
# @Author: WCB
 # @Date: 2019-11-05 08:59:19
 # @LastEditors: WCB
 # @LastEditTime: 2019-11-06 17:06:01
 # @Description: file content
 ###
SCRIPTFOLDER=$(dirname $(readlink -f $0))
#echo "ScriptFolder: "$SCRIPTFOLDER
# cd to current file folder
cd $SCRIPTFOLDER
protoc -o base.pb base.proto
protoc -o core.pb core.proto
protoc -o manager.pb manager.proto
protoc -o processer.pb processer.proto