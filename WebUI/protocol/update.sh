### 
# @Author: WCB
 # @Date: 2019-11-14 16:22:55
 # @LastEditors: WCB
 # @LastEditTime: 2019-11-14 16:22:56
 # @Description: file content
 ###
cd ../../sloongnet/protocol
bash rebuild.sh
cp protocol.proto ../../WebUI/protocol/protocol.proto
cp protocol_pb2.py ../../WebUI/protocol/protocol_pb2.py 