# 为什么单线程的 Redis 能那么快？

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/redis0300.png)

## 总结

### Redis 线程的灵魂三问

“Redis 真的只有单线程吗？

”为什么 用单线程？

”单线程为什么这么快？”

现在，我们知道了。

Redis 单线程是指它对网络 IO 和数据读写的操作采用了一个线程，而采用单线程的一个核心原因是避免多线程开发的并发控制问题。

单线程的 Redis 也能获得高性能，跟多路复用的 IO 模型密切相关，因为这避免了 accept() 和 send()/recv() 潜在的 网络 IO 操作阻塞点。

