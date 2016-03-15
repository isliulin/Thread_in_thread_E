// Console_cpluse_extend.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
using namespace std;

#include "lambda_thread.h"
#include "unit_test.h"
#include "test_task.h"


int _tmain(int argc, _TCHAR* argv[])
{
	//SocketImpl i_socket;
	//i_socket.start();
	TaskBoss i_boss;
	i_boss.start();


	system("pause");
	return 0;
}

