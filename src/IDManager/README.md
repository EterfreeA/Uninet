# 唯一短ID管理器
雪花算法与复用算法结合，支持分布式管理ID。

## 功能
1. 指定十进制ID位数、雪花算法的额外计数、二进制额外计数位数，获得有效ID数量。
2. 提供获取ID、回收ID、检测ID有效性等接口。
3. 支持序列化与反序列化，提供备份与还原接口。

## 规则
1. 优先分配回收ID，其次分配当前ID。
2. 在回收ID与当前ID连续之时，删除回收ID并且回调当前ID。

# 版本
当前版本：v1.0.1  
语言标准：C++20  
创建时间：2021年02月08日  
更新时间：2022年07月13日

## 变化
**v1.0.1**
1. 删除互斥锁。

# 作者
name：许聪  
mailbox：solifree@qq.com  
CSDN：https://blog.csdn.net/xucongyoushan  
gitee：https://gitee.com/solifree  
github：https://github.com/SolifreeXu
