# 前言
> 在工作过程中, 有同事希望用线程将socket的同步的接受数据传输改造成异步的数据接收处理, 同事的思路是在数据接收线程中抛出一个耗时的工作线程去执行耗时的任务,以避免数据接收线程被阻塞导致数据无法及时接受处理. 
面对这样的问题, 刚开始我也百思不得其姐, 但是越是困难, 就越想将其克服. 

# 问题
> 在实际的开发过程中, 在母线程中抛出子线程, 如果子线程的句柄在线程中被销毁, 将导致两种严重的结果（不同的语言的处理结果似乎不一样,windows底层线程API为了防止线程被释放,强制要求所有的线程均需要静态方法实现）

## 严重结果
1. 程序直接崩溃或者抛出异常
2. 子线程根本不执行


# 问题一分析
导致问题一的原因分析需要一定的计算机内存管理知识：
在子线程启动后,子线程的句柄是不允许释放了（内存失去了管理者）, 一般对于释放线程的方式是要求将线程正在做的工作停止下来,再释放的. 考虑到绝大多数线程都是通过一个while循环中来实现耗时任务.
一种比较优雅释放子线程的方式是 使用一个变量从while循环中break来后, 再释放子线程句柄 

``` C++
void doWork()
{
    while(true) {
        doSomthing();   // 耗时任务
    }
}

void doWork    // 优雅的循环线程控制方案
{
    while(true) {
            if(is_stop)
    break;
 doSomthing();   // 耗时任务
    }
}

```

# 问题二分析
子线程的启动时需要时间的,（经过笔者简单测试了一下, 线程启动的时间< 1ms）, 如果子线程的句柄在子线程尚未启动的时候就被销毁, 自然子线程不会执行对应的内容. 

```C++
void doSomething(string t_data)
{
    .......// do something;
}

void run()
{
    while(true) {
        string i_data  = socket.onReicieve();   // 阻塞式接受数据
      
       Thread i_thread ;    声明线程
        i_thread.start( doSomething(i_data));    //指派线程任务,并且启动线程
    }     // 当前循环结束是, i_thread 子线程的句柄将被默认释放掉, 导致线程无法正常中
}
```

------------------------------

# 解决思路
> 知己知彼百战百胜. 既然知道了导致问题的原因, 根据相关情况制定相应的策略

## 方案一：**匿名函数**
让子线程句柄的生命周期与子线程的内容分离, 并且确保子线程在子线程句柄被释放之前能够启动起

优点：匿名线程可直接使用类里面所有的成员变量和成员函数,实现简单快捷方便, 子线程句柄和子线程事务脱离
缺点：需要等待子线程启动后才能释放子线程句柄,需要耗费一定的时间, 不适合应用的实时性要求较高的场所.对线程API有要求,需要提供线程的匿名函数使用方案

## 方案二： **全局线程池托管子线程句柄**
### 数据同步方案 
- 方式1：**数据传输通过中间变量**
采用**生产者-消费者**的模式,用一个全局的队列将母线程中的数据传递到子线程中, 实现数据同步
这是同步的socket改造成异步socket基本思路

优点：实时性较高,可实现类内变量和方法全部接入权限,实现异步功能
缺点：实现较为复杂

---------------------
- 方式2： **new一个对象,对象同时存储工作线程的具体事务和事务数据** 这是实现**任务类**的基本手段

优点： 数据和事务保持在一个类中,方便管理
缺点：实现略微复杂

# 具体代码
1 **匿名函数解决方案**

```C++
#pragma once
#include <string>
#include <iostream>
using namespace std;
#include "Poco/Thread.h"
#include "Poco/Mutex.h"
#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Runnable.h"
using namespace Poco;

Mutex cout_lock;   // multi-thread safe lock 

void Test(int t_id)
{
	int counts = 100;
	while (counts-- > 0)
	{
		{
			Thread::sleep(10);
			Mutex::ScopedLock i_lock(cout_lock);
			cout << "thread " << t_id << " is running " << endl;
		}
	}
}

// solution 1 work, but when setup thread would cost too much time 
// not fit for the instant situation , like socket communication 
void Solution_lambda()
{
	for (int i = 0; i < 10; i++)
	{
		Thread i_thread;
		i_thread.startFunc([&]{
			int i_id = i;
			Test(i);
		});
		Thread::sleep(1);    // protect thread to be not destroyed
	}
	system("pause");
}

int _tmain(int argc, _TCHAR* argv[])
{
    Solution_lambda();
	return 0;
}

```

-----------------------
2 socket同步改异步的设计方案

```c++
class AbstractListener  // 数据接收端口
{
public:
	virtual void onRecieve(string t_data)=0;
	virtual void onSend() = 0;
	virtual void onClose() = 0; 
};

class SocketAsynic: public Runnable
{
public:
	SocketAsynic(AbstractListener * tptr_listener){

		if (tptr_listener == nullptr)
		{
			abort();
		}
		_ptr_listener = tptr_listener;
		_thread_manager.addCapacity(1000);
	};
	void start()   // main thread 
	{
		for (int i = 0; i < 100; i++)
		{
			string i_data = format("thread id : %d ", i);
			Mutex::ScopedLock i_lock(_queque_lock);
			_queque.push((i_data));		// push data into 
			_thread_manager.start(*this);   // throw heavey task to do 
		}
	}
	void Send(string t_data){};
protected:
	virtual void run() final{
		string i_data;
		{
			Mutex::ScopedLock i_lock(_queque_lock);
			if (_queque.empty())
			{
				return;			// not data to deal
			}
			i_data = _queque.front();
			_queque.pop();
		}
		_ptr_listener->onRecieve(i_data);
	};
private:
	ThreadPool _thread_manager;
	Mutex	   _print_lock;
	Mutex	   _queque_lock;
	queue<string> _queque;      // 使用队列传递数据
	AbstractListener *_ptr_listener;
};

```
