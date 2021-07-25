# 第二章：Redis 为什么那么快？数据结构篇

![第二章-Redis为什么那么快？-数据结构篇](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/Redis%E6%A0%B8%E5%BF%83%E5%8E%9F%E7%90%86%E5%92%8C%E5%AE%9E%E6%88%98-Redis%E4%B8%BA%E4%BB%80%E4%B9%88%E9%82%A3%E4%B9%88%E5%BF%AB.png)

实际上，当 Redis 接收到一个键值对操作后， 能以微秒级别的速度找到数据，并快速完成操作。 

数据库这么多，为啥 Redis 能有这么突出的表现呢？

**一方面，这是因为它是内存数据库， 所有操作都在内存上完成，内存的访问速度本身就很快。**

**另一方面，这要归功于它的数据 结构。这是因为，键值对是按一定的数据结构来组织的，操作键值对最终就是对数据结构 进行增删改查操作，所以高效的数据结构是 Redis 快速处理数据的基础。**

## 1、底层数据结构

简单来说，Redis 的底层数据结构一共有 6 种，分别是简单动态字符串、双向链表、压缩列表、哈希表、跳表和整数数组。它们和数据类型的对应关系如下图所示：

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/redis001.png)

## 2、全局哈希表

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/redis002.png)

## 3、哈希表的冲突

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/redis003.png)

## 4、渐进式哈希

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/redis004.png)

## 5、压缩列表结构

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/redis005.png)



## 6、跳表结构

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/redis006.png)

## 7、各种数据结构复杂度

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/redis007.png)



## 总结

Redis 的底层数据结构，这既包括了 Redis 中用来保存每个键和值的 全局哈希表结构，也包括了支持集合类型实现的双向链表、压缩列表、整数数组、哈希表 和跳表这五大底层结构。

Redis 之所以能快速操作键值对，一方面是因为 O(1) 复杂度的哈希表被广泛使用，包括 String、Hash 和 Set，它们的操作复杂度基本由哈希表决定，另一方面，Sorted Set 也采 用了 O(logN) 复杂度的跳表。

不过，集合类型的范围操作，因为要遍历底层数据结构，复杂度通常是 O(N)。

另外复杂度较高的 List 类型，它的两种底层实现结构：双向链表和压缩列 表的操作复杂度都是 O(N)。

例如，既然 它的 POP/PUSH 效率很高，那么就将它主要用于 FIFO 队列场景，而不是作为一个可以随 机读写的集合。 

Redis 数据类型丰富，每个类型的操作繁多，我们通常无法一下子记住所有操作的复杂度。所以，最好的办法就是**掌握原理，以不变应万变。**

这里，一旦掌握了数据结构基本原理，可以从原理上推断不同操作的复杂度，即使这个操作你不一定熟悉。 这样一来，你不用死记硬背，也能快速合理地做出选择了。

参考：《极客时间-Redis核心技术和实战》