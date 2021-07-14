#!/bin/bash
###
 # @Author: WCB
 # @Date: 2020-04-20 12:26:04
 # @LastEditors: Chuanbin Wang
 # @LastEditTime: 2021-04-25 16:54:17
 # @Description: file content
 ###

# disable the Configuring tzdata
export DEBIAN_FRONTEND=noninteractive

apt-get update
# install run time library
apt-get install -y libsqlite3-0 libprotobuf17 libuuid1 libssl1.1  libjsoncpp1 mariadb-client liblua5.3 graphicsmagick
# install develop library
apt-get install -y cmake clang llvm libsqlite3-dev libprotobuf-dev protobuf-compiler uuid-dev libssl-dev libjsoncpp-dev libmariadbclient-dev liblua5.3
