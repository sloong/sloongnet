###
 # @Author: WCB
 # @Date: 2020-04-20 12:26:04
 # @LastEditors: WCB
 # @LastEditTime: 2020-04-20 12:26:34
 # @Description: file content
 ###
#!/bin/bash
sudo apt-get update
# install run time library
sudo apt install -y libsqlite3-0 libprotobuf17 libuuid1 libglib2.0-0 libssl1.1 libboost-serialization1.67.0 liblua5.3-0 libjsoncpp1 zip
# install develop library
sudo apt install -y cmake gcc g++ gdb libsqlite3-dev libprotobuf-dev protobuf-compiler uuid-dev libglib2.0-dev libssl-dev libboost-dev libboost-serialization-dev liblua5.3-dev libjsoncpp-dev
