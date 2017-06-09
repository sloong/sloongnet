# Sloongnet - power by sloong.com
开源网络引擎 - sloong.com 团队.

***

## 概览
* 基于`linux` 统. 所有的测试在CentOS7 64bit系统中进行.
* 在编译本引擎之前,需要先编译并安装依赖模块 [library for sloong.com](https://git.sloong.com/public/library).

## 特性
* 使用 `多线程 & epoll` 技术.
* 框架基于 `lua script & c++ engine`.
* 支持消息优先级模式，高优先级的优先处理
* 使用消息标识区分每个消息，避免传统的一问一答模式，耗时消息将不会影响其他消息的处理和返回，配合消息优先级模式，让高优先级的操作不在“卡”

## 更新历史
[点击查看](https://git.sloong.com/public/sloongnet/src/master/ChangeLog.md)

## 如何编译
* 安装所需要的依赖包

```
yum install -y gcc gcc-c++ glib2 glib2-devel libuuid libuuid-devel mariadb mariadb-devel openssl-devel ImageMagick GraphicsMagick boost-devel readline-devel libjpeb-devel    
git clone https://git.sloong.com/public/library.git    
cd library/
./install.sh
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