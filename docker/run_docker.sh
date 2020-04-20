###
 # @Author: WCB
 # @Date: 2020-04-20 14:27:15
 # @LastEditors: WCB
 # @LastEditTime: 2020-04-20 19:23:44
 # @Description: file content
 ###
#!/bin/bash
docker run -d --name $1 -p 8001:8001 -v data:/data -e NODE_TYPE=$2 -e ADDRESS_INFO=$3 engine:latest