# C/C++

### 重新理解【浅拷贝】和【深拷贝】
### 目录

[前言](#pre)

[C++ 的复制机制](#a)

[默认拷贝构造函数](#b)

[浅拷贝](#c)

[深拷贝](#d)



### 1、<span id = "pre">前言</span>

今天上午接到一个公司的面试电话，期间面试官问到了 C++ 的浅拷贝和深拷贝，不知怎么的，尽管脑子里早就有这个概念，但是话到嘴边突然就说不出来，脑子竟然一片空白了，最后还是面试官给我说了他的理解，现在想想，实在是有点丢人，这么一个最基础的东西居然没说上来，暴露了基础理解不够扎实的问题啊，这次要下功夫彻底搞懂它！

### <span id = "a">2、C++ 的复制机制</span>

C++ 中对象的复制就如同”克隆“，用一个已有的对象快速复制出多个完全相同的对象。一般而言，以下三种情况都会使用对象的复制。



> （1）建立一个新对象，并用另一个同类的已有对象对新对象进行初始化，例如：

```c++
class Base
{
private:
    int w;
    int h;
};
int main(int argc,const char *argv[])
{
    Base b1;
    base b2(b1);  // 使用b1初始化b2，此时会进行对象的复制
    return 0;
}
```



> （2） 当函数的参数是类的对象时，调用此函数使用的是值传递，也会发生对象的复制，例如：

```c++
void fun1(Base base)
{
	//TODO
}
int main(int argc,const char *argv[])
{
    Base b1;
    fun1(b1);  // 此时会发生对象的复制
    return 0;
}
```



> （3） 当函数的返回值是类的对象时，当函数调用结束时，需要将函数的对象复制到一个临时的对象并传给该函数的调用处，也会发生对象的复制，例如：

```c++
void fun2()
{
	//TODO
	Base base;
	return base;
}
int main(int argc,const char *argv[])
{
    Base b2;
    b2 = fun2(); 
    // 在 fun2 返回对象时，会执行对象复制，复制出一临时对象，然后将此临时对象“赋值”给 b2
    return 0;
}
```

注意到：对象的复制都是通过一种特殊的构造函数来完成的，这种特殊的构造函数就是拷贝构造函数**（copy constructor，也叫复制构造函数）。**

拷贝构造函数在大多数情况下都很简单，甚至在我们都不知道它存在的情况下也能很好发挥作用，但是在一些特殊情况下，**特别是在对象里有动态成员，或者指针类型变量的时候**，就需要我们特别小心地处理拷贝构造函数了。下面我们就来看看拷贝构造函数的使用。



### <span id = "b"> 3、默认拷贝构造函数</span>

很多时候在我们都不知道拷贝构造函数的情况下，传递对象给函数参数或者函数返回对象都能很好的进行，这是因为**编译器会给我们自动产生一个拷贝构造函数**，这就是**“默认拷贝构造函数”**，这个构造函数很简单，仅仅使用“老对象”的数据成员的值对“新对象”的数据成员一一进行赋值，它一般具有以下形式：

```c++
Base::Base(const Base& t)
{
	w = t.w;
	h = t.h;
}
```

当然了，有人说，我怎么一般没实现这个函数呢，照样可以进行类的复制操作啊，这是因为以上代码不用我们编写，编译器会为我们自动生成。但是如果认为这样就可以解决对象的复制问题，那就错了，让我们来考虑以下一段代码：

```c++
//Author:github.com/rongweihe
//Date  : 2019/04/20
#include <bits/stdc++.h>
using namespace std;
//浅拷贝：对于基本类型的数据和简单的对象，它们之间的拷贝非常简单；就是按位复制内存
class Base
{
public:
    Base()
    {
        c++;
    }
    ~Base()
    {
        cout<<"调用析构函数"<<endl;
        c--;
    }
    static int getC()
    {
        return c;
    }

private:
    int a;
    int b;
    static int c;
};
int Base::c = 0;

int main()
{
    //freopen("in.txt","r",stdin);

    Base obj1;
    cout<<"obj1.getC()="<<obj1.getC()<<endl;
    Base obj2 = obj1;
    cout<<"obj2.getC()="<<obj2.getC()<<endl;
    return 0;
}
```

这段代码对前面的类进行一些修改，加入了一个计数器静态成员，目的是进行计数，统计创建的对象的个数。在每个对象创建时，通过构造函数进行递增，在销毁对象时，通过析构函数进行递减。

运行结果

>    obj1.getC()=1
>    obj2.getC()=1
>    调用析构函数
>    ~c=0
>    调用析构函数
>    ~c=-1

注意到，计数器变为负数了，这是为什么呢？

在主函数中，首先创建对象 obj1，输出此时的对象个数，然后使用 obj1 复制出对象 obj2，再输出此时的对象个数，按照理解，此时应该有两个对象存在，但实际程序运行时，输出的都是 1，反应出只有 1 个对象。

在销毁对象时，会调用销毁两个对象，类的析构函数会调用两次，此时的计数器将变为负数。**出现这些问题最根本就在于在复制对象时，计数器没有递增**，解决的办法就是重新编写拷贝构造函数，在拷贝构造函数中加入对计数器的处理，形成的拷贝构造函数如下：

```c++
class Base
{
public:
    Base()
    {
        c++;
    }
    ~Base()
    {
        cout<<"调用析构函数"<<endl;
        c--;
    }
    // 拷贝构造函数
    Base(const Base2& t)
    {
        a=t.a;
        b=t.b;
        c++;
    }
    static int getC()
    {
        return c;
    }

private:
    int a;
    int b;
    static int c;
};
int Base::c = 0;
```

运行结果

> obj11.getC()=1
> obj22.getC()=2
> 调用析构函数
> ~c=1
> 调用析构函数
> ~c=0



### <span id = "c"> 4、浅拷贝</span>

**浅拷贝：**指的是在对象复制时，只是将对象中的数据成员进行简单的赋值，上面的例子都是属于浅拷贝的情况，**默认拷贝构造函数执行的也是浅拷贝**。大多情况下“浅拷贝”已经能很好地工作了，但是一旦对象存在了动态成员，那么浅拷贝就会出问题了，让我们考虑如下一段代码：

```c++
#include <bits/stdc++.h>
using namespace std;
class Base
{
public:
    Base()//构造函数，p 指向堆中分配的空间
    {
        cout<<"new"<<endl;
        p = new int(100);
    }
    ~Base()//析构函数，释放动态分配的空间
    {
        if(p!=NULL)
        {
            delete p;
            cout<<"delete 的地址= "<<p<<endl;
        }
    }
private:
    int w;
    int h;
    int *p;
};

int main()
{
    //freopen("in.txt","r",stdin);
    Base obj1;
    Base obj2 = obj1;//复制对象
    return 0;
}
```

运行结果

> new
> delete 的地址= 0x62b228
> delete 的地址= 0x62b228

**p 指针被分配一次内存，但是程序结束时该内存却被释放了两次，会造成内存泄漏问题！**

原因就在于在进行对象复制时，对于动态分配的内容没有进行正确的操作。我们来分析一下：

在运行定义 obj1 对象后，由于在构造函数中有一个动态分配的语句，因此执行后的内存情况大致如下：

![深拷贝001.jpg](https://i.loli.net/2019/04/20/5cbad59365932.jpg)

在使用 obj1 复制 obj2 时，由于执行的是浅拷贝，只是将成员的值进行赋值，所以此时 obj1.p 和obj2.p 具有相同的值，也即这两个指针指向了堆里的同一个空间，如下图所示：

![深拷贝002.jpg](https://i.loli.net/2019/04/20/5cbad6401bb85.jpg)

当然，这不是我们所期望的结果，在销毁对象时，两个对象的析构函数将对**同一个内存空间释放两次**，这就是错误出现的原因。我们需要的不是两个 *p* 有相同的值，而是两个 *p* **指向**的空间有相同的值，解决办法就是使用“深拷贝”。



### <span id = "d"> 5、深拷贝</span>

在“深拷贝”的情况下，对于对象中动态成员，就不能仅仅简单地赋值了，而应该重新动态分配空间，如上面的例子就应该按照如下的方式进行处理：

```c++
#include <bits/stdc++.h>
using namespace std;
class Base
{
public:
    Base()//构造函数，p 指向堆中分配的空间
    {
        p = new int(100);
        cout<<"new1 and &p = "<<p<<endl;
    }
    Base(const Base& t)
	{
		w = t.w;
		h = t.h;
		p = new int;	// 为新对象重新动态分配空间
		*p = *(t.p);
		 cout<<"new2 and &p = "<<p<<endl;
	}
    ~Base()//析构函数，释放动态分配的空间
    {
        if(p!=NULL)
        {
            delete p;
            cout<<"delete 的地址= "<<p<<endl;
        }
    }
private:
    int w;
    int h;
    int *p;
};

int main()
{
    //freopen("in.txt","r",stdin);
    Base obj1;
    Base obj2 = obj1;//复制对象
    return 0;
}
```

运行结果

> new1 and &p = 0x3cb228
> new2 and &p = 0x3cb238
> delete 的地址= 0x3cb238
> delete 的地址= 0x3cb228

  此时，在完成对象的复制后，内存的一个大致情况如下：

![深拷贝003.jpg](https://i.loli.net/2019/04/20/5cbad8f1bd8d6.jpg)

注意到，此时 obj1 的 p 和 obj2 的 p 各自指向一段内存空间，但它们指向的空间具有相同的内容，这就是所谓的“深拷贝”。



<br/>

<br/>

参考：

博客：https://blog.csdn.net/bluescorpio/article/details/4322682

《*C++*编程思想 第*1*卷》   *Bruce Eckel*



<br/>

<br/>




# Python
# Java
# Go
