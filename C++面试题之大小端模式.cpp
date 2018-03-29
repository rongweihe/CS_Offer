/*
Author:@herongwei
Date:2018/03/29 23:00
*/
#include <iostream>
#include <bits/stdc++.h>
using namespace std;

/*面试题2
【题目】请写一个C函数，若处理器是Big_endian的，则返回0；若是Little_endian的，则返回1
【解析】采用little-endian模式的CPU对操作数的存放方式是从低字节到高字节、
而Big-endian模式对操作数的存放方式是从高字节到低字节。
例如16bit宽的数0x1234在Little-endian模式CPU内存中的存放方式（假设从地址0x4000开始存放）为：

内存地址 存放内容

0x4000  0x34

0x4001  0x12

而在Big-endian模式CPU内存中的存放方式则为：

内存地址 存放内容

0x4000  0x12

0x4001  0x34

32bit宽的数0x12345678在Little-endian模式CPU内存中的存放方式（假设从地址0x4000开始存放）为：

内存地址  存放内容

0x4000    0x78

0x4001    0x56

0x4002    0x34

0x4003    0x12

而在Big-endian模式CPU内存中的存放方式则为：

内存地址  存放内容

0x4000    0x12

0x4001    0x34

0x4002    0x56

0x4003    0x78

联合体union的存放顺序是所有成员都从低地址开始存放。
什么是大端和小端
[参考](https://blog.csdn.net/ce123_zhouwei/article/details/6971544)
        Big-Endian和Little-Endian的定义如下：
1) Little-Endian就是低位字节排放在内存的低地址端，高位字节排放在内存的高地址端。
2) Big-Endian就是高位字节排放在内存的低地址端，低位字节排放在内存的高地址端。
举一个例子，比如数字0x12 34 56 78在内存中的表示形式为：
1)大端模式：

低地址 -----------------> 高地址
0x12  |  0x34  |  0x56  |  0x78
2)小端模式：

低地址 ------------------> 高地址
0x78  |  0x56  |  0x34  |  0x12
可见，大端模式和字符串的存储模式类似。
*/
int CheckCpu()
{
    union w
    {
        int a;
        char b;
    } c;
    c.a=1;
    return (c.b==1);
}
int main()
{
    if(CheckCpu() == 0) cout<<"Big_endian"<<endl;
    else cout<<"Little_endian"<<endl;
    return 0;
}
