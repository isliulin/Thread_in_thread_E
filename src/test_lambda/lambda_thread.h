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

