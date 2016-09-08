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
* ### v0.7.6
   * #### 新功能 
      * [#42 ](https://git.sloong.com/public/sloongnet/issues/42)GetThumbImage返回值问题 
      * [#43 ](https://git.sloong.com/public/sloongnet/issues/42)MD5校验失败后添加相关信息以便定位问题
* ### v0.7.5
   * #### 优化
      * [#38 ](https://git.sloong.com/public/sloongnet/issues/38)完善程序异常捕捉
   * #### 修复BUG
      * [#39 ](https://git.sloong.com/public/sloongnet/issues/39)GetThumbImage返回值问题
* ### v0.7.4
   * #### 新功能
      * [#20 ](https://git.sloong.com/public/sloongnet/issues/20)关闭连接时通知脚本
* ### v0.7.3
   * #### 新功能
      * [#37 ](https://git.sloong.com/public/sloongnet/issues/37)接收文件接口函数返回值支持指示成功文件数量以及md列表以及错误信息
* ### v0.7.2
   * #### 新功能
      * [#31 ](https://git.sloong.com/public/sloongnet/issues/31)获取缩率图接口函数支持自定义路径
      * [#35 ](https://git.sloong.com/public/sloongnet/issues/35)接收文件接口函数返回值支持指示成功文件数量以及错误信息
   * #### 修复BUG
      * [#36 ](https://git.sloong.com/public/sloongnet/issues/36)ReceiveFile接口问题
* ### v0.7.1
   * #### 新功能
      * [#30 ](https://git.sloong.com/public/sloongnet/issues/30)添加多文件接收功能
   * #### 修复BUG
      * [#32 ](https://git.sloong.com/public/sloongnet/issues/32)启用超时检测后异常崩溃问题
* ### v0.7.0
   * #### 新功能 
      * [#28 ](https://git.sloong.com/public/sloongnet/issues/28)添加文件接收（TCP）功能
   * #### 修复BUG
      * [#33 ](https://git.sloong.com/public/sloongnet/issues/33)请求图片文件时发送异常问题 
* ### v0.6.8
   * #### 优化
      * 配置文件中的RunType修改为DebugMode
* ### v0.6.7
   * #### 修复BUG
       * [#26 ](https://git.sloong.com/public/sloongnet/issues/26)服务模式下运行，log文件被写入了大量的帮助信息
      * [#27 ](https://git.sloong.com/public/sloongnet/issues/27)log配置不生效的问题
* ### v0.6.6
    * #### 修复BUG
        * [#23 ](https://git.sloong.com/public/sloongnet/issues/23) 添加超时检测后空闲时间CPU占用较高
        * [#22 ](https://git.sloong.com/public/sloongnet/issues/22) 在进行大规模并发的时候有问题
		* [#24 ](https://git.sloong.com/public/sloongnet/issues/24) 脚本执行插入,修改，删除这类只有影响行数没有返回结果的命令，返回0的问题
* ### v0.6.4
    * #### 新功能
        * userinfo添加ip和prot信息
        * 添加超时检测.可自由配置超时时间与检测间隔
    * #### 修复BUG
        * SQL连接失败的时候没有提示信息
        * 使用SendFile发送文件错误的问题
* ### v0.6.0
    * #### 新功能
        * 添加文件上传功能
        * 添加SQL运行时LOG配置项
    * #### 修复BUG
        * 程序退出时没有彻底退出所有线程的问题
        * 配置文件中数据库配置项不起作用的问题
* ### v0.5.0
    * #### 新功能
        * 使用新的发送文件方式.
        * 流水号和MD5校验支持可配置
* ### v0.4.2
    * #### 新功能
        * 为Lua脚本添加API.
        * 支持程序直接重新加载脚本
        * 修改Mysql返回值中,列分隔符为0x09,行分隔符为0x0a
* ### v0.4.0
    * #### 修复BUG
        * 接收数据过程中收到EAGAIN/EINTR中断后,之后所有接收的消息不完整的问题.
        * 稳定性提升
    * #### 新功能
        * 添加了ShowSendMessage和ShowReceiveMessage配置项,可以控制接收和发送的消息是否显示.
* ### v0.3.0
    * #### 修复BUG
        * 在大规模数据情况下,程序经常崩溃问题.
* ### v0.2.0
    * #### 新功能
        *  发送列表支持优先级控制
    * #### 修复BUG
        *  在发送数据之前,检查发送列表是不是空列表.
* ### v0.1.0
 * #### 第一个版本.

## 如何编译
* 安装所需要的依赖包
```
yum install -y gcc gcc-c++ glib2 glib2-devel libuuid libuuid-devel mariadb mariadb-devel openssl-devel ImageMagick GraphicsMagick
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