# Sloongnet - power by sloong.com
开源网络引擎 - sloong.com 团队.

***

## 概览
* 基于`linux` 统. 所有的测试在CentOS7 64bit系统中进行.
* 在编译本引擎之前,需要先编译并安装依赖模块 [library for sloong.com](https://git.sloong.com/public/library).

## 特性
* 使用 `多线程 & epoll` 技术.
* 框架基于 `lua script + c++ engine`.

## 历史
* ###v0.3.0
 * ####修复BUG
   * 在大规模数据情况下,程序经常崩溃问题.
* ###v0.2.0
 * #####新功能
   *  发送列表支持优先级控制
 * ####修复BUG
   *  在发送数据之前,检查发送列表是不是空列表.
* ###v0.1.0
 * ####第一个版本.

## 如何编译
* 安装所需要的依赖包
```
yum install -y gcc gcc-c++ glib2 glib2-devel libuuid libuuid-devel mariadb mariadb-devel openssl-devel  
git clone http://git.sloong.com:3333/public/library.git  
cd library/lua
make linux
make install
cd ../univ
make update
```
* 开始编译
```
git clone http://git.sloong.com:3333/public/sloongnet.git
cd sloongnet
make
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