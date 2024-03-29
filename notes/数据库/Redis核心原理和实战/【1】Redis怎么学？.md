# 第一章：Redis 怎么学？

## 前言

> 相信，很多同学一开始和我一样，都习惯带着一个个具体的问题来学习一门课，比如说，
>
> Redis 数据怎么做持久化？
>
> 集群方案应该怎么做？
>
> 这些问题当然很重要，但是，如果你只是急于解决这些细微的问题，你慢慢发现，你的 Redis 使用能力就很难得到质的提升。
>
> 但按照好这些年，在和国内大厂的合作过程中，我发现，很多技术人都有一个误区，那就是，只关注零散的技术点，没有建立起一套完整的知识框架，缺乏系统观，但是，系统观其实是至关重要的。
>
> 从某种程度上说，在解决问题时，拥有了系统观，就意味着你能有依据、有章 法地定位和解决问题。
>
> --《极客时间》《Redis核心技术与实战》-蒋德钧

## 两大维度，三大主线

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/redis%E4%B8%89%E5%A4%A7%E4%B8%BB%E7%BA%BF.png)

“两大维度”就是指系统维度和应用维度，“三大主线” 也就是指高性能、高可靠和高可 扩展（可以简称为“三高”）。

**高性能主线，**包括线程模型、数据结构、持久化、网络框架； 

**高可靠主线，**包括主从复制、哨兵机制； 

**高可扩展主线**，包括数据分片、负载均衡。

## redis 问题画像图

![Wmtl1H.png](https://z3.ax1x.com/2021/07/15/Wmtl1H.png)

参考：《极客时间》《Redis核心技术与实战》。