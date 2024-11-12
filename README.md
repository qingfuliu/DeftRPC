DeftRPC:A Corutine based RPC framework
===================================
DeftRPC是一个基于C++编写的Rpc框架。

![](https://img.shields.io/badge/License-Apache-green)
![](https://img.shields.io/badge/Release-V1.2-orange)
![](https://img.shields.io/badge/CMake-V3.26.3-blue)
![](https://img.shields.io/badge/Cpp-17-blue)
![](https://img.shields.io/badge/CLang-14.0-60d6a7)
![](https://img.shields.io/badge/gtest-ef6c9a)
![](https://github.com/qingfuliu/DeftRPC/workflows/CMake/badge.svg)
![](https://github.com/qingfuliu/DeftRPC/workflows/Make/badge.svg)

****

| 作者          | github                   |
|-------------|--------------------------|
| qfingfu liu | [github](www.sdfasd.com) |

****

## 目录

* [简介](#简介)
* [日志模块](#日志模块)
* 协程模块（非对称、共享栈）
* 序列化模块（模板元编程）
* hook模块（dlsym）
* 网络模块（reactor网络模型）
* [RPC模块(路由、Sever、Client)](#RPC模块)
* [参考](#参考)

## 简介

DeftRPC是一个rpc框架。拥有日志模块、协程模块、序列化与反序列化等模块。提供了简洁的API，轻便且容易上手。

## 日志模块

### 初始化

在使用日志前，需要调用初始化函数来初始化日志器。其中N是日志器的编号，Appender是vector形式的LogAppend数组。

```cpp
clsn::init<N>(Appenders);
```

形如：

```cpp
clsn::init<0>({
    clsn::createConsoleLogAppender(
        "[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s",
                        clsn::LogLevel::Debug)});
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
clsn::init<0>({
    clsn::createConsoleLogAppender(
        "[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s",
                        clsn::LogLevel::Debug)});
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

## RPC模块

### RPC路由

提供若干种底层实现：

| 数据结构                |
|---------------------|
| hash table（siphash） |
| RB Tree             |
| 前缀树                 |

使用模板如下：

```cpp
int test_router(int a) {
    return a + 1;
}

clsn::RpcRouter r("test"); //创建路由
r.InsertFunc("test", test_router);//插入函数

std::string res;
clsn::StringSerialize encode(res);
encode(std::tuple<int>(100));//将参数序列化

auto p = r.CallFuncSync("test", res); //调用函数

clsn::StringDeSerialize decode(p);
int aa;
decode(aa);     //结果反序列化

CLSN_LOG_DEBUG << aa; //输出结果
```

### RPC Sever

即成于TcpSever，使用模板如下：

```c++
#include "log/Log.h"
#include "rpc/Router.h"
#include "rpc/RPCSever.h"
#include "hook/Hook.h"
int test_router(int a) {
    return a + 1;
}

int test_rpc(int a) {
    return a + 1;
}

int test_exception(int a, int b) {
    throw std::runtime_error("test exception");
}

int main() {
    Enable_Hook();
    
    auto r = new clsn::RpcRouter("test");       //新建路由并注册函数
    r->InsertFunc("test_router", test_router);
    r->InsertFunc("test_rpc", test_rpc);
    r->InsertFunc("test_exception", test_exception);

    auto rpcSever = clsn::CreateRpcSever("0.0.0.0:5201", 1); //创建rpcSever，并设置ip、端口
    rpcSever->SetRouter(r);           //设置路由，RpcSever将接管该router的生命周期
    rpcSever->Start(100000);           //启动路由
}
```

### RPC Client

继承于TcpClient，提供future特性，部分接口如下：

| 接口名称                                                                                                                                                              | 作用                     |
|-------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------|
| template<class Res, class ...Args> std::future<Res><br/> DoFuncAsync(std::string &&name, Args &&...args)                                                          | 类似于std::async的future特性 |
| template<typename Clock, typename Dur, class Res, class ...Args>Res DoFuncAsyncUntil(std::chrono::time_point<Clock, Dur> point,std::string &&name,Args &&...args) | 类似于wait_unitl          |
| template<typename Res, typename Rep, class Period, class ...Args>Res DoFuncAsyncFor(std::chrono::duration<Rep, Period> point,std::string &&name,Args &&...args)   | 类似于wait_for            |

使用模板如下：

```c++
#include "hook/Hook.h"
#include "rpc/RpcClient.h"

int main() {
    Enable_Hook();
    Disable_Enable_Hook();
    clsn::RpcClient client("0.0.0.0:5201");
    CLSN_LOG_DEBUG << "remote:" << client.GetRemote().toString();
    if (!client.Connect()) {
        CLSN_LOG_ERROR << "connect failed,error is "
                       << strerror(errno);
        return -1;
    }

    auto p = client.DoFuncAsync<int>("test_router", 1);

    CLSN_LOG_DEBUG << p.get();
}
```

## 参考

* sipHash: https://github.com/google/highwayhash
    * 引用了highwayhash部分文件，主要修改了部分头文件路径，在test文件中删除了部分没用的代码
    * 以下文件引用自 highwayhash ：

  src/test/test_sipHash.cpp   
  src/hash/*.cc   
  include/google_highwayhash_Public/*.h

