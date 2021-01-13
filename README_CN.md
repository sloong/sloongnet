﻿<!--
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-10-06 18:15:28
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/README_CN.md
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
-->
# Sloongnet - power by sloong.com
高性能开源网络引擎 - sloong.com 团队.

***

## 概览
* 只需要关心消息的处理，而无需关心底层部分网络的收发处理。快速简单实现一个高性能的网络服务。
* 支持Docker部署。
* 根据负载情况自动动态扩容。

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

```
2. 下载最新发布包
3. 解压并执行.


## 编译安装
1. 下载源代码
```
git clone https://git.sloong.com:8000/sloongnet/engine.git    
cd engine
```
2. 安装依赖
```
./build/environment.sh
```
3. 开始编译
```
./build/build.sh -r
```


## 联系我们
如果你有任何问题,请联系我们.

* [Email](admin@sloong.com)

## 所引用的库
use library in this project.

* [SloongLibrary](https://git.sloong.com/public/library)
* Openssl



