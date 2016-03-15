#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <map>
using namespace std;


#include "Poco/Thread.h"
#include "Poco/Mutex.h"
#include "Poco/AutoPtr.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Runnable.h"
#include "Poco/ThreadPool.h"
#include "Poco/Format.h"
#include <queue>
#include "Poco/Tuple.h"
using namespace Poco;


Mutex g_print_lock;


class TaskWorker : public Runnable , public RefCountedObject
{

public:
	TaskWorker(int t_id, string t_data) :_id(t_id), _data(t_data){};

	virtual void run()
	{
		int ncount = 10;
		while (ncount-- > 0)
		{
			Mutex::ScopedLock i_lock(g_print_lock);
			cout << "id: " << _id << "data:  " << _data << endl;
		}
	}

private:
	string _data;
	int    _id;
};





class TaskBoss : public Runnable
{
public:
	TaskBoss(){
		_thread_manager.addCapacity(1000);
	};

	void start()
	{
		_thread_handle.start(*this);	// 无阻塞式的启动线程
	}

protected:

	virtual void run() final{

		int thread_count = 0;
		while (thread_count++ < 1000)
		{		
			string i_data = format("thread id : %d ", thread_count);
			static AutoPtr<TaskWorker>  i_worker = new TaskWorker(thread_count, i_data);
			_thread_manager.start(*i_worker);   // throw heavey task to do 
			Thread::sleep(1);
		}
	};

private:
	ThreadPool _thread_manager;
	Thread     _thread_handle;
};



