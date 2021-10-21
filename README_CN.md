﻿# Sloongnet - 只需实现业务的服务框架
Copyright 2015-2021 Sloong.com. All Rights Reserved

[![Build Status](https://drone.sloong.com:8000/api/badges/sloongnet/engine/status.svg)](https://drone.sloong.com:8000/sloongnet/engine)

***

# Language 
[English](https://git.sloong.com:8000/sloongnet/engine/src/branch/develop/README.md)


# 概览

这是一个只需要实现自己的业务，即可拥有一套现代化的高性能服务端。
使用这套系统框架，无需关心其他与业务无关的细节。通过自带的模块组件进行组合，可快速实现各种功能。
而在默认组件无法满足的情况，根据规则编写自己的模块组件之后，也可很方便的嵌入到系统中，实现自己特有的需求。


# 特性
* 支持使用脚本语言编写业务系统
* 通讯协议基于Protobuf
* 支持Docker部署
* 根据负载情况自动动态扩容
* 基于`事件驱动`的理念进行设计
* 支持消息优先级，高优先级请求将被优先处理
* 异步通讯模式，发送后接收的消息并不是对应的，配合消息优先级功能，低优先级的耗时请求将不会影响高优先级消息的处理和返回


## 联系我们
如果你有任何问题,请联系我们.

* [Email](admin@sloong.com)

## 所引用的库
use library in this project.

* [SloongLibrary](https://git.sloong.com/public/library)
* Openssl
* libfmt
* libspdlog
* libprotobuf



