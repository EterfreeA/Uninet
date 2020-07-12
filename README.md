﻿Uninet
	微服务集群架构
	采用C++开发启动程序，以lua为中心，控制业务逻辑和管理资源，以C++模块扩展底层功能。

## 组件
负载均衡器
    服务器：由单个固定负载均衡服务和多个负载均衡器组成的星形结构服务集群。负载均衡服务计算负载均衡器权重，向客户端提供负载均衡器动态列表。负载均衡器连接负载均衡服务，向客户端提供代理/服务器动态列表。
    客户端：从负载均衡服务获取负载均衡器列表，根据权重依次从负载均衡器获取代理/服务器动态列表，直到获取成功或者遍历完成为止。
动态选择器
    根据列表权重依次连接代理/服务器，直到连接成功或者遍历完成为止。
字节流缓存池
    制定拆包机制，解决粘包问题。前4字节指定数据长度，根据长度划分数据段，之后放入缓冲池，供上层应用读取。
