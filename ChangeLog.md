# 更新历史
***
### v0.8.0
* 在这个版本中调整了消息包的长度返回格式，不兼容以前的版本！！
#### BUG修复
* [#47 ](https://git.sloong.com/public/sloongnet/issues/47)修改返回数据时，长度的返回样式和接收统一
### v0.7.7
#### 新增特性 
* [#44 ](https://git.sloong.com/public/sloongnet/issues/44)添加扩展配置文件支持 
#### BUG修复
* [#45 ](https://git.sloong.com/public/sloongnet/issues/45)配置文件中配置不全时，程序崩溃
#### 功能改进
* [#29 ](https://git.sloong.com/public/sloongnet/issues/29)脚本有错误的情况下程序直接启动失败
### v0.7.6
#### 新增特性 
* [#42 ](https://git.sloong.com/public/sloongnet/issues/42)GetThumbImage返回值问题 
* [#43 ](https://git.sloong.com/public/sloongnet/issues/42)MD5校验失败后添加相关信息以便定位问题
#### 功能改进
* [#40 ](https://git.sloong.com/public/sloongnet/issues/40)优先级标志位使用char型的数字优先级
### v0.7.5
#### 功能改进
* [#38 ](https://git.sloong.com/public/sloongnet/issues/38)完善程序异常捕捉
#### BUG修复
* [#39 ](https://git.sloong.com/public/sloongnet/issues/39)GetThumbImage返回值问题
### v0.7.4
#### 新增特性
* [#20 ](https://git.sloong.com/public/sloongnet/issues/20)关闭连接时通知脚本
### v0.7.3
#### 新增特性
* [#37 ](https://git.sloong.com/public/sloongnet/issues/37)接收文件接口函数返回值支持指示成功文件数量以及md列表以及错误信息
### v0.7.2
#### 新增特性
* [#31 ](https://git.sloong.com/public/sloongnet/issues/31)获取缩率图接口函数支持自定义路径
* [#35 ](https://git.sloong.com/public/sloongnet/issues/35)接收文件接口函数返回值支持指示成功文件数量以及错误信息
#### BUG修复
* [#36 ](https://git.sloong.com/public/sloongnet/issues/36)ReceiveFile接口问题
### v0.7.1
#### 新增特性
* [#30 ](https://git.sloong.com/public/sloongnet/issues/30)添加多文件接收功能
#### BUG修复
* [#32 ](https://git.sloong.com/public/sloongnet/issues/32)启用超时检测后异常崩溃问题
### v0.7.0
#### 新增特性 
* [#28 ](https://git.sloong.com/public/sloongnet/issues/28)添加文件接收（TCP）功能
#### BUG修复
* [#33 ](https://git.sloong.com/public/sloongnet/issues/33)请求图片文件时发送异常问题 
### v0.6.8
#### 功能改进
* 配置文件中的RunType修改为DebugMode
### v0.6.7
#### BUG修复
* [#26 ](https://git.sloong.com/public/sloongnet/issues/26)服务模式下运行，log文件被写入了大量的帮助信息
* [#27 ](https://git.sloong.com/public/sloongnet/issues/27)log配置不生效的问题
### v0.6.6
#### BUG修复
* [#23 ](https://git.sloong.com/public/sloongnet/issues/23) 添加超时检测后空闲时间CPU占用较高
* [#22 ](https://git.sloong.com/public/sloongnet/issues/22) 在进行大规模并发的时候有问题
* [#24 ](https://git.sloong.com/public/sloongnet/issues/24) 脚本执行插入,修改，删除这类只有影响行数没有返回结果的命令，返回0的问题
### v0.6.4
#### 新增特性
* userinfo添加ip和prot信息
* 添加超时检测.可自由配置超时时间与检测间隔
#### BUG修复
* SQL连接失败的时候没有提示信息
* 使用SendFile发送文件错误的问题
### v0.6.0
#### 新增特性
* 添加文件上传功能
* 添加SQL运行时LOG配置项
#### BUG修复
* 程序退出时没有彻底退出所有线程的问题
* 配置文件中数据库配置项不起作用的问题
### v0.5.0
#### 新增特性
* 使用新的发送文件方式.
* 流水号和MD5校验支持可配置
### v0.4.2
#### 新增特性
* 为Lua脚本添加API.
* 支持程序直接重新加载脚本
* 修改Mysql返回值中,列分隔符为0x09,行分隔符为0x0a
### v0.4.0
#### BUG修复
* 接收数据过程中收到EAGAIN/EINTR中断后,之后所有接收的消息不完整的问题.
* 稳定性提升
#### 新增特性
* 添加了ShowSendMessage和ShowReceiveMessage配置项,可以控制接收和发送的消息是否显示.
### v0.3.0
#### BUG修复
* 在大规模数据情况下,程序经常崩溃问题.
### v0.2.0
#### 新增特性
*  发送列表支持优先级控制
#### BUG修复
*  在发送数据之前,检查发送列表是不是空列表.
### v0.1.0
#### 第一个版本.