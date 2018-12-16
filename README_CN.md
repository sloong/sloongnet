﻿# Sloongnet - power by sloong.com
高性能开源网络引擎 - sloong.com 团队.

***

## 概览
* 基于`linux`系统. 所有的测试在CentOS7 64bit系统中进行.
* 在编译本引擎之前,需要先安装依赖模块 [library for sloong.com](https://git.sloong.com/public/library).

## 特性
* 使用 `epoll` 技术来完成网络I/O部分。
* 使用 `线程池` 技术来完成系统内部的事件分发和处理。
* 整个系统基于`事件驱动`的理念进行设计，类似于windows的事件系统
* 业务系统将使用`lua`脚本语言实现。
* 支持消息优先级模式，高优先级请求将可以被优先处理
* 发送模式为异步，即发送后接收的消息并不是对应的，配合消息优先级功能，耗时请求将不会影响其他高优先级消息的处理和返回，让高优先级的操作不在“卡”

## 更新历史
[点击查看](https://git.sloong.com/public/sloongnet/src/master/ChangeLog.md)

## 直接安装
1. 安装依赖包：
```
yum install -y glib2 libuuid mariadb openssl ImageMagick GraphicsMagick boost readline libjpeg
```
2. 下载最新发布包
3. 解压并执行.

## 如何编译
* 安装gcc7以上版本
* 安装所需要的依赖包

```
yum install -y glib2 glib2-devel libuuid libuuid-devel mariadb mariadb-devel openssl-devel ImageMagick GraphicsMagick boost-devel readline-devel libjpeg-devel    
git clone https://git.sloong.com/public/library.git    
cd library/
./install.sh
```

```
for ubuntu

yum insatll -y glib2.0 uuid libmariadbclient18 ssl imagemagick graphicsmagick libboost 
```

* 开始编译

```
git clone https://git.sloong.com/public/sloongnet.git    
cd sloongnet    
./install.sh    
```


## 联系我们
如果你有任何问题,请联系我们.

* [Email](wcb@sloong.com)

## 所引用的库
use library in this project.

* [CImg](https://git.sloong.com/wcb/CImg) 
* [SloongLibrary](https://git.sloong.com/public/library)
* Openssl
* Glib



