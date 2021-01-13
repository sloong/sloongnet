###
 # @Author: WCB
 # @Date: 2020-04-20 12:26:04
 # @LastEditors: Chuanbin Wang
 # @LastEditTime: 2020-08-17 10:42:47
 # @Description: file content
 ###
#!/bin/bash
apt-get update
# install run time library
apt install -y libsqlite3-0 libprotobuf17 libuuid1 libssl1.1  libjsoncpp1 mariadb-client libluajit-5.1-2 graphicsmagick
# install develop library
apt install -y cmake clang llvm libsqlite3-dev libprotobuf-dev protobuf-compiler uuid-dev libssl-dev libjsoncpp-dev libmariadbclient-dev libluajit-5.1-dev
