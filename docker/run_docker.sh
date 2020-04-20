###
 # @Author: WCB
 # @Date: 2020-04-20 14:27:15
 # @LastEditors: WCB
 # @LastEditTime: 2020-04-20 14:27:15
 # @Description: file content
 ###
#!/bin/bash
sudo docker run -d --name $1 -p 8001:8001 -v data:/data -e NODE_TYPE=$2 ADDRESS_INFO=$3 sloongnet:0.10.2.412