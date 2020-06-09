#!/bin/bash
###
 # @Author: WCB
 # @Date: 2019-12-25 10:04:10
 # @LastEditors: WCB
 # @LastEditTime: 2020-04-20 19:21:26
 # @Description: file content# 
 ###
 # check db file 
if [ ! -e /data/configuation.db ];then
    cp /usr/local/bin/configuation.db /data/configuation.db
fi
 # start app
/usr/local/bin/sloongnet $NODE_TYPE $ADDRESS_INFO