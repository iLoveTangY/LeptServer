# LeptServer
一个在Linux环境下简单的基于Epoll的http服务器，练手之作。

# Usage
```
git clone git@github.com:iLoveTangY/LeptServer.git
mdkir build && cd build
cmake .. && make
./LeptServer -h
```

# 支持特性
- [x] 支持`GET`方法
- [x] 支持Keep-alive

待改进：定时器在多线程环境下可能有问题

# 关于名称
名称来源自[这里](https://zhuanlan.zhihu.com/json-tutorial)。是物理中轻子的英文名称。

