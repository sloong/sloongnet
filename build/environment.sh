#!/bin/bash
###
 # @Author: WCB
 # @Date: 2020-04-20 12:26:04
 # @LastEditors: Chuanbin Wang
 # @LastEditTime: 2021-08-26 14:13:33
 # @Description: file content
 ###

# disable the Configuring tzdata
export DEBIAN_FRONTEND=noninteractive

apt-get update
# install run time library
# for debian 11/ubuntu 21.04
apt install -y libprotobuf23 libjsoncpp24
# for debian 10/ubuntu 20.04
apt install -y libprotobuf17 libjsoncpp1
apt-get install -y libsqlite3-0 libuuid1 libssl1.1 mariadb-client liblua5.3-0 imagemagick
# install develop library
apt-get install -y cmake clang llvm libsqlite3-dev libprotobuf-dev protobuf-compiler uuid-dev libssl-dev libjsoncpp-dev libmariadb-dev liblua5.3-dev libfmt-dev
