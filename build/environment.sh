#!/bin/bash
###
 # @Author: WCB
 # @Date: 2020-04-20 12:26:04
 # @LastEditors: Chuanbin Wang
 # @LastEditTime: 2021-10-20 10:41:35
 # @Description: file content
 ###

# disable the Configuring tzdata
export DEBIAN_FRONTEND=noninteractive

update(){
    DEBIAN_FRONTEND=noninteractive apt update
    DEBIAN_FRONTEND=noninteractive apt install -y tzdata
}

install_run_env(){
    # install run time library
    # for debian 11/ubuntu 21.04
    apt install -y libprotobuf23 libjsoncpp24 libfmt7
    # for debian 10/ubuntu 20.04
    # apt install -y libprotobuf17 libjsoncpp1 libfmt6
    apt install -y libsqlite3-0 libuuid1 libssl1.1 mariadb-client liblua5.3-0 imagemagick
}


install_build_env(){
    # install develop library
    apt install -y cmake clang llvm libsqlite3-dev libprotobuf-dev protobuf-compiler uuid-dev libssl-dev libjsoncpp-dev libmariadb-dev liblua5.3-dev libfmt-dev libspdlog-dev
}

install_all(){
    update
    install_build_env
    install_run_env
}


if [ $# -eq 0 ]; then
	install_all
	exit
fi

if [ $# -eq 1 ]; then
	case $1 in 
		--build) 
            update
			install_build_env
			;;
		--run) 
            update
			install_run_env
			;;
		* ) 
			install_all
			;;
	esac
fi
