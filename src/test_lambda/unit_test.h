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


class AbstractListener 
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
	queue<string> _queque;
	AbstractListener *_ptr_listener;

};



