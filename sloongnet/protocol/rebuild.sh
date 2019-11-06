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
protoc --cpp_out=./ ./protocol.proto
protoc --csharp_out=./ ./protocol.proto
