###
 # @Author: WCB
 # @Date: 2020-04-20 12:26:04
 # @LastEditors: WCB
 # @LastEditTime: 2020-04-20 12:26:34
 # @Description: file content
 ###
#!/bin/bash
apt-get update
# install run time library
apt install -y libsqlite3-0 libprotobuf17 libuuid1 libssl1.1  libjsoncpp1 mariadb-client libluajit-5.1-2
# install develop library
apt install -y cmake gcc g++ gdb libsqlite3-dev libprotobuf-dev protobuf-compiler uuid-dev libssl-dev libjsoncpp-dev libmariadbclient-dev libluajit-5.1-dev
