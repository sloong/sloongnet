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
#pip3 install protobuf
protoc --python_out=./ ./protocol.proto
#protoc --python3_out=./ ./protocol.proto
#sudo apt install -y go
#go get -u github.com/golang/protobuf/{protoc-gen-go,proto}
protoc --go_out=./ ./protocol.proto
#sudo apt install rust
#cargo install protobuf
#protoc --rust_out . protocol.proto