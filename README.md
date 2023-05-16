DeftRPC:A Corutine based RPC framework
===================================
DeftRPC是一个基于C++编写的Rpc框架。

![](https://img.shields.io/badge/License-Apache-green)
![](https://img.shields.io/badge/Release-V1.1-orange)
![](https://img.shields.io/badge/CMake-V3.26.3-blue)
![](https://img.shields.io/badge/Cpp-17-blue)

****

| 作者          | github                   |
|-------------|--------------------------|
| qfingfu liu | [github](www.sdfasd.com) |

****

## 目录

* [简介](#简介)
* [日志模块](#日志模块)
* 协程模块
* 序列化模块
* hook模块
* 网络模块
* [参考](#参考)

## 简介

DeftRPC是一个rpc框架。拥有日志模块、协程模块、序列化与反序列化等模块。提供了简洁的API，轻便且容易上手。

## 日志模块

### 初始化

在使用日志前，需要调用初始化函数来初始化日志器。其中N是日志器的编号，Appender是vector形式的LogAppend数组。

```cpp
CLSN::init<N>(Appenders);
```

形如：

```cpp
CLSN::init<0>({
    CLSN::createConsoleLogAppender(
        "[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s",
                        CLSN::LogLevel::Debug)});
```

### LogAppender

指定日志的输出位置。提供如下LogAppend于相应的工厂方法。

|   Appender Name    | outPut Location |
|:------------------:|-----------------|
| ConsoleLogAppender | 控制台             |
|  FileLogAppender   | 指定文件            |

### LogFormatter

采用字符串格式化日志的方法。字符与日志项目对照表如下。

| key | value |
|-----|-------|
| %Y  | 年份    |
| %m  | 月份    |
| %d  | 日期    |
| %H  | 小时    |
| %M  | 分钟    |
| %S  | 秒     |
| %n  | 行号    |
| %s  | 内容    |
| %f  | 文件名   |
| %t  | 线程Id  |
| %l  | 日志等级  |
| %o  | 日志器名字 |

例如采用“[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s”，调用如下代码：

```cpp
CLSN::init<0>({
    CLSN::createConsoleLogAppender(
        "[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s",
                        CLSN::LogLevel::Debug)});
        CLSN_LOG_DEBUG << "message";
```

将输出：

    [5037] 2023-05-12 01:36:24:</home/****/project/*****.cpp:20> [Debug] message

### 日志输出API

日志输出宏定义如下所示。

    CLSN_LOG_<LEVEL> << <message>;

例如：

    CLSN_LOG_FATAL << "this is a fatal";
    CLSN_LOG_DEBUG << "this is a debug";

提供了对STL的输出支持。

```cpp
std::vector<int>temp{1,2,3};
CLSN_LOG_DEBUG << temp;
```

## 参考

* sipHash: https://github.com/google/highwayhash
    * 引用了highwayhash部分文件，主要修改了部分头文件路径，在test文件中删除了部分没用的代码
    * 以下文件引用自 highwayhash ：


  src/test/test_sipHash.cpp 
  src/hash/*.cc
  include/google_highwayhash_Public/*.h
