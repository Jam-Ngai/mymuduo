# **mymuduo**

### 项目介绍

仿照Muduo库开发了一款轻量级、高性能的C++网络库，基于Reactor模型，支持多线程并发，该项目采用c++11开发，不依赖boost库，适用于构建高性能服务器应用。 采用epoll实现了基于Reactor模式的事件循环系统，极大提升了I/O操作的效率；构建了一个高效的线程池管理机制，实现任务的动态调度和负载均衡，优化服务器性能；实现了非阻塞的TCP连接管理和数据传输模块，支持高并发的客户端连接处理。

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

