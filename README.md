# **mymuduo**

### 项目介绍

仿照 Muduo 库开发了一款轻量级、高性能的 C++网络库，基于 Reactor 模型，支持多线程并 发，该项目采用 c++11 开发，不依赖 boost 库，适用于构建高性能服务器应用。 采用 epoll 实现了基于 Reactor 模式的事件循环系统，极大提升了 I/O 操作的效率；构建了一个 高效的线程池管理机制，实现任务的动态调度和负载均衡，优化服务器性能；实现了非阻塞的 TCP 连接管理 和数据传输模块，支持高并发的客户端连接处理。

### 用法

- 安装库

```shell
git clone https://github.com/Jam-Ngai/mymuduo.git
cd mymuduo
mkdir build
cmake ..
make
cd ..
sudo sh install.sh
```

- 编译echo server和Client

```shell
cd test
mkdir build
cd build
cmake ..
make
cd ..
```

- 测试服务器

```shell
# 开启服务器
./bin/echoserver
# 创建10,000个客户端进行连接
sh test.sh
```

