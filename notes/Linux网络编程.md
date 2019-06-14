# Linux 网络编程常用 API &&用 socket 实现简单客户端和服务端的通信（基于TCP）



## 一 、socket（TCP）编程 API 简介

### 1）、int socket(int family, int type, int protocol);

 socket() 打开一个网络通讯端口，如果成功的话，就像 open() 一样返回一个文件描述符，应用程序可以像读写文件一样用 read/write 在网络上收发数据，如果 socket() 调用出错则返回-1。对于 IPv4，family 参数指定为 AF_INET。

对于 TCP 协议，type 参数指定为 SOCK_STREAM，表示面向流的传输协议。如果是 UDP 协议，则type 参数指定 SOCK_DGRAM，表示面向数据报的传输协议。

这里不详细介绍 protocol 参数，指定为0即可。

### 2）、int bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);

服务器程序所监听的网络地址和端口号通常是固定不变的，客户端程序得知服务器程序的地址和端口号后就可以向服务器发起连接，因此服务器需要调用 bind 绑定一个固定的网络地址和端口号，bind()成功返回 0，失败返回 -1。  

bind() 的作用是将参数 sockfd 和 myaddr 绑定在一起，使 sockfd 这个用于网络通讯的文件描述符。
监听 myaddr 所描述的地址和端口号。这里注意 struct sockaddr *是一个通用指针类型，myaddr 参
数实际上可以接受多种协议的 sockaddr 结构体，而它们的长度各不相同，所以需要第三个参数
addrlen 指定结构体的长度。程序中对 myaddr 参数是这样初始化的：

```c
bzero(&servaddr, sizeof(servaddr));
servaddr.sin_family = AF_INET;
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
servaddr.sin_port = htons(SERV_PORT);
```

首先将整个结构体清零，然后设置地址类型为 AF_INET，网络地址为 INADDR_ANY，这个宏表示本地的任意 IP 地址，因为服务器可能有多个网卡，每个网卡也可能绑定多个 IP 地址，这样设置可以在所有的IP地址上监听，直到与某个客户端建立了连接时才确定下来到底用哪个 IP 地址，端口号为SERV_PORT，这里定义为 2345。

### 3）、int listen(int sockfd, int backlog);


典型的服务器程序可以同时服务于多个客户端，当有客户端发起连接时，服务器调用的 accept() 返回并接受这个连接，如果有大量的客户端发起连接而服务器来不及处理，尚未 accept 的客户端就处于连接等待状态，listen() 声明 sockfd 处于监听状态，并且最多允许有 backlog 个客户端处于连接待状态，如果接收到更多的连接请求就忽略。listen() 成功返回0，失败返回-1。

### 4)、int accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);

三次握手完成后，服务器调用 accept() 接受连接。

这些天在网上看到这样一个题目，据听说是腾讯的面试题：

**TCP三次握手的过程，accept 发生在三次握手的哪一个阶段?**

> 答案是：accept 过程发生在三次握手之后，三次握手完成后，客户端和服务器就建立了 tcp 连接并可以进行数据交互了。这时可以调用 accept 函数获得此连接。

参考下图更加清晰。

如果服务器调用 accept() 时还没有客户端的连接请求，就阻塞等待直到有客户端连接上来。cliaddr 是一个传出参数，accept() 返回时传出客户端的地址和端口号。

addrlen 参数是一个传入传出参数（value-result argument），传入的是调用者提供的缓冲区 cliaddr的长度以避免缓冲区溢出问题，传出的是客户端地址结构体的实际长度（有可能没有占满调用者提供的缓冲区）。如果给 cliaddr 参数传NULL，表示不关心客户端的地址。
一般的服务器程序结构是这样的：

```c
while (1) {
cliaddr_len = sizeof(cliaddr);
connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
n = read(connfd, buf, MAXLINE);
......
close(connfd);
}
```

整个是一个 while 循环，每次循环处理一个客户端连接。

由于 cliaddr_len 是传入传出参数，每次调用 accept() 之前应该重新赋初值。accept() 的参数 listenfd是先前的监听文件描述符，而 accept() 的返回值是另外一个文件描述符 connfd，之后与客户端之间就通过这个 connfd 通讯，最后关闭 connfd 断开连接，而不关闭 listenfd，再次回到循环开头listenfd 仍然用作 accept 的参数。accept() 成功返回一个文件描述符，出错返回 -1。由于客户端不需要固定的端口号，因此不必调用 bind()，客户端的端口号由内核自动分配。

注意，客户端不是不允许调用 bind()，只是没有必要调用 bind() 固定一个端口号，服务器也不是必须调用 bind()，但如果服务器不调用 bind()，内核会自动给服务器分配监听端口，每次启动服务器时端口号都不一样，客户端要连接服务器就会遇到麻烦。

### 5）、int connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen);

客户端需要调用 connect() 连接服务器，connect 和 bind 的参数形式一致，区别在于 bind 的参数是自己的地址，而 connect 的参数是对方的地址。connect() 成功返回0，出错返回-1。

### 6)、write函数 size_t write(int fd,const void *buf,size_t nbytes);


```c
Write 函数将 buf 中的 nbytes 字节内容写入到文件描述符中，成功返回写的字节数，失败返回-1.并设置 errno 变量。在网络程序中，当我们向套接字文件描述舒服写数据时有两种可能：

（1）、write 的返回值大于0，表示写了部分数据或者是全部的数据，这样用一个 while 循环不断的写入数据，但是循环过程中的 buf 参数和 nbytes 参数是我们自己来更新的，也就是说，网络编程中写函数是不负责将全部数据写完之后再返回的，说不定中途就返回了！

（2）、返回值小于 0，此时出错了，需要根据错误类型进行相应的处理。

（3）、如果错误是 EINTR 表示在写的时候出现了中断错误，如果是 EPIPE 表示网络连接出现了问题。
```

### 7)、read函数 size_t read(int fd,void *buf,size_t nbyte)

```c
Read 函数是负责从 fd 中读取内容。
（1）、当读取成功时，read 返回实际读取到的字节数，如果返回值是 0，表示已经读取到文件的结束了，小于 0 表示是读取错误。
（2）、如果错误是 EINTR 表示在写的时候出现了中断错误，如果是 EPIPE 表示网络连接出现了问题。
```

### 8)、recv 函数和 send 函数

```c
recv 函数和 read 函数提供了 read 和 write 函数一样的功能，不同的是他们提供了四个参数。

int recv(int fd,void *buf,int len,int flags)
int send(int fd,void *buf,int len,int flags)

前面的三个参数和 read、write 函数是一样的。第四个参数可以是 0 或者是以下的组合：

（1）、MSG_DONTROUTE：不查找表是 send 函数使用的标志，这个标志告诉 IP，目的主机在本地网络上，没有必要查找表，这个标志一般用在网络诊断和路由程序里面。

（2）、MSG_OOB：接受或者发生带外数据，表示可以接收和发送带外数据。

（3）、MSG_PEEK：查看数据，并不从系统缓冲区移走数据，是 recv 函数使用的标志，表示只是从系统缓冲区中读取内容，而不清楚系统缓冲区的内容。这样在下次读取的时候，依然是一样的内容，一般在有过个进程读写数据的时候使用这个标志。

（4）、MSG_WAITALL：等待所有数据，是recv函数的使用标志，表示等到所有的信息到达时才返回，使用这个标志的时候，recv 返回一直阻塞，直到指定的条件满足时，或者是发生了错误。
```

###  9) 、close 的定义如下：

```c
#include <unistd.h>
int close(int fd);

关闭读写。
成功则返回 0，错误返回 -1，错误码 errno：EBADF 表示 fd 不是一个有效描述符；EINTR 表示close 函数被信号中断；EIO 表示一个 IO 错误。
```



## **二、客户端和服务端的通信图**

  ![TCP建立一次连接的过程图.png](https://i.loli.net/2019/04/09/5cac4c06bad0e.png)



## **三、实现现简单客户端和服务端的通信（基于TCP）**

这是实现的是客户端写数据，服务端读取，然后把读取的数据写到客户端显示。

服务端代码

```c
/**********************************************************
* file:  server.c
* Author:github.com/rongweihe
* Data:  2019/04/09
***********************************************************/
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<netdb.h>
#include<errno.h>
const int PORT    = 2345;
const int MAXSIZE = 1024;

int main(int argc, char *argv[]){
    int sockfd, newsockfd;

    //1. 定义服务端套接字数据结构,客户端套接口数据结构
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    //2. 创建套接字
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "create socket failed\n");
        exit(EXIT_FAILURE);
    }
    printf("create socket success and sockfd is %d\n",sockfd);

    //清空表示地址的结构体变量,设置addr的成员变量信息,设置 ip 为本机 IP
    bzero(&server_addr,  sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //3. 绑定套接字
    if (bind(sockfd, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr)) < 0){
        fprintf(stderr, "bind failed \n");
        exit(EXIT_FAILURE);
    }
    printf("bind success...\n");

    //4. 监听套接字
    if (listen(sockfd, 10) < 0){
        perror("listen fail\n");
        exit(EXIT_FAILURE);
    }
    printf("listen success...\n");

    int sin_size = sizeof(struct sockaddr_in);
    printf("sin_size is %d\n", sin_size);
    //5. 三次握手完成后，服务器调用 accept() 接受连接
    if ((newsockfd = accept(sockfd, (struct sockaddr *)(&client_addr), &sin_size)) < 0){
        perror("accept error");
        exit(EXIT_FAILURE);
    }
    printf("accepted a new connetction...\n");
    printf("new socket id is %d\n", newsockfd);
    printf("Accept clent ip is %s\n", inet_ntoa(client_addr.sin_addr));
    printf("Connect successful please input message:");

    //发送，接收数据缓冲区
    char sendbuf[1024];
    char recvbuf[1024];
    while(1){
        int len = recv(newsockfd, recvbuf, sizeof(recvbuf), 0);
        if (strcmp(recvbuf, "exit\n") == 0) break;
        fputs(recvbuf, stdout);
        send(newsockfd, recvbuf, len, 0);
        memset(sendbuf, 0,sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(newsockfd);
    close(sockfd);
    puts("exit success");
    exit(EXIT_SUCCESS);
    return 0;
}
```



客户端代码

```c
/**********************************************************
* file:  client.c
* Author:github.com/rongweihe
* Data:  2019/04/09
***********************************************************/
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<netdb.h>
#include<errno.h>
const int PORT    = 2345;
const int MAXSIZE = 1024;

int main(int argc,const char *argv[]){
    int sockfd;
    struct sockaddr_in server_addr;

    //1. 创建套接字
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "Socket Error is %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //2. 客户端发出请求
    if(connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1){
        fprintf(stderr, "Connect failed\n");
        exit(EXIT_FAILURE);
    }

    char sendbuf[MAXSIZE];
    char recvbuf[MAXSIZE];
    while (1){
        //从标准输入中读取数据
        fgets(sendbuf, sizeof(sendbuf), stdin);
        //发送数据
        send(sockfd, sendbuf, strlen(sendbuf), 0);
        if (strcmp(sendbuf, "exit\n") == 0) break;
        //接收数据并显示到终端
        recv(sockfd, recvbuf, sizeof(recvbuf), 0);
        fputs(recvbuf, stdout);
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(sockfd);
    exit(EXIT_SUCCESS);
    return 0;
}
```



运行效果图

**Server**

![server.png](https://i.loli.net/2019/04/09/5cac4afce7c28.png)

**Client**

![client.png](https://i.loli.net/2019/04/09/5cac4b15075a9.png)


