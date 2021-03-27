# Linux 高性能网络编程利器：epoll 的前世今生

这张图直观地为我们展示了 select、poll、epoll 几种不同的 I/O 复用技术在面对不同文件描述符大小时的表现差异。

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/epoll/epoll3.png)

从图中可以明显地看到，epoll 的性能是最好的，即使在多达 10000 个文件描述的情况下，其性能的下降和有 10 个文件描述符的情况相比，差别也不是很大。而随着文件描述符的增大，常规的 select 和 poll 方法性能逐渐变得很差。

那么，epoll 究竟使用了什么样的“魔法”，取得了如此令人惊讶的效果呢？接下来，我们就来一起分析一下。

### epoll 的用法

在分析对比 epoll、poll 和 select 几种技术之前，我们先看一下怎么使用 epoll 来完成一个服务器程序。

epoll 可以说是和 poll 非常相似的一种 IO 多路复用技术，不同于传统的 select、poll，epoll 最关键的是通过回调函数来感知就绪事件，通过监控注册的多个描述符，来进行 IO 事件的分发处理。epoll 不仅提供了默认的水平触发模式，还提供了性能更为强劲的边缘触发模式。

使用 epoll 进行网络程序的编写，需要三个基本步骤：

> 1、调用 epoll_create 创建一个 epoll 实例。
>
> 2、调用 epoll_ctl 往 epoll 实例增加或删除监控的事件。
>
> 3、调用 epoll_wait 返回就绪事件的个数。

下面来具体展开详细学习：

### epoll_create

```c++
int epoll_create(int size);
int epoll_create1(int flags);
        返回值: 若成功返回一个大于0的值，表示epoll实例；若返回-1表示出错
```

epoll_create 方法创建了一个 epoll 实例，从 Linux 2.6.8 开始，参数 size 被自动忽略，但是该值仍需要一个大于 0 的整数。

这个 epoll 实例被用来调用 epoll_ctl 和 epoll_wait，如果这个 epoll 实例不再需要，比如服务器正常关机，需要调用 close() 方法释放 epoll 实例，这样系统内核可以回收 epoll 实例所分配使用的内核资源。

在新的实现中，size 这个参数不再被需要，因为内核可以动态分配需要的内核数据结构。我们只需要注意，每次将 size 设置成一个大于 0 的整数就可以了。

epoll_create1() 的用法和 epoll_create() 基本一致，如果 epoll_create1() 的输入 flags 为 0，则和 epoll_create() 一样，内核自动忽略。

### epoll_ctl

```c++
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
        返回值: 若成功返回0；若返回-1表示出错
```

在创建完 epoll 实例之后，可以通过调用 epoll_ctl 往这个 epoll 实例增加或删除监控的事件。函数 epll_ctl 有 4 个入口参数。

第一个参数 epfd 是刚刚调用 epoll_create 创建的 epoll 实例描述字，可以简单理解成是 epoll 句柄。

第二个参数表示增加还是删除一个监控事件，它有三个选项可供选择：

- EPOLL_CTL_ADD： 向 epoll 实例注册文件描述符对应的事件；
- EPOLL_CTL_DEL：向 epoll 实例删除文件描述符对应的事件；
- EPOLL_CTL_MOD： 修改文件描述符对应的事件。

第三个参数是注册的事件的文件描述符，比如一个监听套接字。

第四个参数表示的是注册的事件类型，并且可以在这个结构体里设置用户需要的数据，其中最为常见的是使用联合结构里的 fd 字段，表示事件所对应的文件描述符。

```c++

typedef union epoll_data {
     void        *ptr;
     int          fd;
     uint32_t     u32;
     uint64_t     u64;
 } epoll_data_t;

 struct epoll_event {
     uint32_t     events;      /* Epoll events 我们关心的事件 */
     epoll_data_t data;        /* User data variable 用户就绪的数据 */
 };
```

了解 poll 的同学应该知道这里的基于 mask 的事件类型了，这里 epoll 仍旧使用了同样的机制，我们重点看一下这几种事件类型：

- EPOLLIN：表示对应的文件描述字可以读；
- EPOLLOUT：表示对应的文件描述字可以写；
- EPOLLRDHUP：表示套接字的一端已经关闭，或者半关闭；
- EPOLLHUP：表示对应的文件描述字被挂起；
- EPOLLET：设置为 edge-triggered，默认为 level-triggered。

### epoll_wait

```c++
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
  返回值: 成功返回的是一个大于0的数，表示事件的个数；返回0表示的是超时时间到；若出错返回-1.
```

epoll_wait() 函数类似之前的 poll 和 select 函数，调用者进程被挂起，在等待内核 I/O 事件的分发。

这个函数的第一个参数是 epoll 实例描述字，也就是 epoll 句柄。

第二个参数返回给用户空间需要处理的 I/O 事件，这是一个数组，数组的大小由 epoll_wait 的返回值决定，这个数组的每个元素都是一个需要待处理的 I/O 事件，其中 events 表示具体的事件类型，事件类型取值和 epoll_ctl 可设置的值一样，这个 epoll_event 结构体里的 data 值就是在 epoll_ctl 那里设置的 data，也就是用户空间和内核空间调用时需要的数据。

第三个参数是一个大于 0 的整数，表示 epoll_wait 可以返回的最大事件值。

第四个参数是 epoll_wait 阻塞调用的超时值，如果这个值设置为 -1，表示不超时；如果设置为 0 则立即返回，即使没有任何 I/O 事件发生。

### epoll 例子代码解析

实践出真知，我们在 Linux 服务器端程序写一段基于 epoll 的代码：

```c++
#define MAXEVENTS 128
#define SERV_PORT 12345
#include <sys/epoll.h>
char rot13_char(char c) { 
  if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M')) return c + 13; 
  else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z')) return c - 13;
  else return c;
}
void nonblocking_server(int port){}

int main(int argc, char **argv) {
    int listen_fd, socket_fd;
    int n, i;
    int efd;

    struct epoll_event event;
    struct epoll_event *events;

    listen_fd = nonblocking_server(SERV_PORT);
    efd = epoll_create1(0);//调用 epoll_create0 创建了一个 epoll 实例。
    if (efd == -1) {
        error(1, errno, "epoll create failed");
    }
  
		/*
		调用 epoll_ctl 将监听套接字对应的 I/O 事件进行了注册，这样在有新的连接建立之后
		就可以感知到。注意这里使用的是 edge-triggered（边缘触发）。
		*/
    event.data.fd = listen_fd;
    event.events = EPOLL | EPOLLET;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, listen_fd, &event) == -1) {
        error(1, errno, "epoll_ctl add listen fd failed");
    }
    // Buffer where events are returned 为返回的 event 数组分配了内存。
    events = calloc(MAXEVENTS, sizeof(event));

    //主循环调用 epoll_wait 函数分发 I/O 事件
    //当 epoll_wait 成功返回时，通过遍历返回的 event 数组，就直接可以知道发生的 I/O 事件。
    while (1) {
        n = epoll_wait(efd, events, MAXEVENTS, -1);
        printf("epoll_wait wakeup\n");

        for(i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            } else if (listen_fd == events[i].data.fd) {
              	/*
              	监听套接字上有事件发生的情况下，调用 accept 获取已建立连接
              	并将该连接设置为非阻塞，再调用 epoll_ctl 把已连接套接字对应的可读事件
              	注册到 epoll 实例中。
              	这里我们使用了 event_data 里面的 fd 字段，将连接套接字存储其中。
              	注意监听套接字和连接套接字不一样。
              	*/
                struct sockaddr_storage ss;
                socklen_t slen = sizeof(ss);
                int fd = accept(listen_fd, (struct sockaddr *)&ss, &slen);
                if (fd < 0) {
                    error(1, error, "accept failed");
                } else {
                    nonblocking_server(fd);
                    event.data.fd = fd;
                    event.events = EPOLLIN | EPOLLET; //edge-triggered
                    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1) {
                        error(1, errno, "epoll_ctl add connection fd failed");
                    }
                }
                continue;
            } else {
              	//处理了已连接套接字上的可读事件，读取字节流，编码后再回应给客户端。
                socket_fd = events[i].data.fd;
                printf("get event on socket fd == %d\n", socket_fd);
                while (1) {
                    char buf[512];
                    if ((n = read(socket_fd, buf, sizeof(buf))) < 0 ) {
                        if (errno != EAGAIN) {
                            error(1, error, "read error");
                            close(socket_fd);
                        }
                        break;
                    } else if(n == 0) {
                        close(socket_fd);
                        break;
                    } else {
                        for (i = 0; i<n; ++i) {
                            buf[i] = rot13_char(buf[i]);
                        }
                        if (write(socket_fd, buf, n) < 0) {
                            error(1, errno, "write error");
                        }
                    }
                }
            }
        }
    }
    free(events);
    close(listen_fd);
    return 0;
}
```

### 实验

启动该服务器：

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/epoll/epoll-01-s2.png)

再启动几个 telnet 客户端，可以看到有连接建立情况下，epoll_wait 迅速从挂起状态结束；并且套接字上有数据可读时，epoll_wait 也迅速结束挂起状态，这时候通过 read 可以读取套接字接收缓冲区上的数据。

客户端回显：

![](https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/epoll/epoll-01-c2.png)

第一个程序我们设置为 edge-triggered，即边缘触发。开启这个服务器程序，用 telnet 连接上，输入一些字符，我们看到，服务器端只从 epoll_wait 中苏醒过一次，就是第一次有数据可读的时候。如上图所示。

第二个程序我们设置为 level-triggered，即条件触发。然后按照同样的步骤来一次，观察服务器端，这一次我们可以看到，服务器端不断地从 epoll_wait 中苏醒，告诉我们有数据需要读取。

<div  align="center">   <img src="https://cdn.jsdelivr.net/gh/rongweihe/ImageHost01/epoll/epoll-03-s1.png" width = "500" height = "500" align=center/> </div>



看到了，这就是两者的区别，条件触发的意思是说，只要满足了条件的事件，比如有数据可读，就会一直不断地把这个事件传递给用户；而边缘触发的意思是说只有第一次满足条件的时候才会触发。之后就不会传递相同的事件了。

一般我们认为，边缘触发的效率比条件触发的效率要高，这一点也是 epoll 的杀手锏之一。

2002 年，epoll 最终在 Linux 2.5.44 中首次出现，在 2.6 中趋于稳定，为 Linux 的高性能网络 I/O 画上了一段句号。

下一篇，我们有时间来剖析一下 epoll 的源码，看有多少想看，点赞超过 10 继续硬核更新！