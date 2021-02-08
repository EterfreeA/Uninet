# 不重复短ID管理器
雪花算法与复用算法结合，支持分布式管理ID。

## 程序结构
管理器配接容器，而容器存放当前ID和回收ID。优先分配回收ID，其次分配当前ID。

类模板|说明
-|-
IDPointContainer|ID容器之一，以点形式存放回收ID。
IDLineContainer|ID容器之一，以线段（区间）形式存放回收ID。
IDManager|ID管理器，负责分配和回收ID。

## 项目结构
源码只含头文件，另附测试源文件。

文件|说明
-|-
[IDContainer.hpp](IDContainer.hpp)|定义ID容器类模板。
[IDManager.hpp](IDManager.cpp)|定义ID管理器类模板。
[test.cpp](test.cpp)|测试代码。

## 作者
name：许聪  
mailbox：2592419242@qq.com  
CSDN：https://blog.csdn.net/xucongyoushan  
gitee：https://gitee.com/solifree  
github：https://github.com/xucongandxuchong
