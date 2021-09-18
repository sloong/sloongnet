<!--
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2021-09-18 14:10:18
 * @LastEditTime: 2021-09-18 14:38:00
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/doc/wiki.md
 * Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
-->

# 数据包处理流程

一个数据包由其类型决定走向，类型即此包是普通的请求数据包，或者是时间通知的内部事件包。

## 普通请求数据包

只支持被动相应的形式。


## 事件通知包

事件分为在Core中定义的控制性事件，以及在各Module中自己定义的事件。

### 控制性事件

控制性事件是由Manager模块发出，用来控制其他模板行为的专用类型，通常在路由之前会由Core模块直接处理掉，所以通常不必理会。

### 自定义事件

对于类型为