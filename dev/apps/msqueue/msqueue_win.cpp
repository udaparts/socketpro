// msqueue.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mqfile.h"
#include <iostream>
#include <assert.h>
#include <time.h>
using namespace std;

const int COUNT = 2000000;
const int Enq_Thread_Count = 4;
const int Deq_Thread_Count = 6;

const char *sample_str = "TEST ME FROM CHARLIE YE BEGIN_STORAGE_RETRY_BLOCK Nowadays, There are a plenty of Javascript charting libraries. Justly, this article aims to show you how you can integrate Javascript charting libraries in ASP .NET and build custom charting user controls depending on your flavor and your needs.";

long Deq_Count = 0;
HANDLE MQThreads[Enq_Thread_Count + Deq_Thread_Count] = {0};
MQ_FILE::CMqFile mqFile(L"TEST", 24, 20901);

int GetRandom()
{
  srand((unsigned)time(NULL));
  return (rand() % 10);
}

DWORD WINAPI EnqThreadProc(LPVOID lpParameter)
{
	MB::CScopeUQueue su;
	MB::U_UINT64 mqIndex;
	DWORD dwThreadId = ::GetCurrentThreadId();
	int n, MyCount = COUNT/Enq_Thread_Count;
	for(n=0; n<MyCount; ++n)
	{
		su<< sample_str << n << dwThreadId;
		mqIndex = mqFile.Enqueue(su->GetBuffer(), su->GetSize());
		su->SetSize(0);
	}
	return 0;
}

DWORD WINAPI DeqThreadProc(LPVOID lpParameter)
{
	int n;
	int sleepCount = 0;
	DWORD dwThreadId;
	std::string str;
	MB::CScopeUQueue su;
	MB::U_UINT64 mqIndex;
	MB::U_UINT64 index;
	do
	{
		index = mqFile.Dequeue(*su, mqIndex);
		while(index != MQ_FILE::CMqFile::INVALID_MSG_INDEX)
		{
			su >> str >> n >> dwThreadId;
			assert(str == sample_str);
			sleepCount = 0;
			assert(su->GetSize() == 0);
			bool fail = (GetRandom() < 5);
			if(!fail)
			  InterlockedIncrement(&Deq_Count);
			mqFile.ConfirmDequeue(index, fail);
			index = mqFile.Dequeue(*su, mqIndex);
		}
		++sleepCount;
		::Sleep(25);
	}while(sleepCount < 3);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int n;
	DWORD dw;
	DWORD dwPrev = ::GetTickCount();
	for(n=0; n<Enq_Thread_Count; ++n)
	{
		MQThreads[n] = ::CreateThread(NULL, 0, EnqThreadProc, NULL, 0, &dw);
	}

	for(n=Enq_Thread_Count; n<(Enq_Thread_Count + Deq_Thread_Count); ++n)
	{
		MQThreads[n] = ::CreateThread(NULL, 0, DeqThreadProc, NULL, 0, &dw);
	}
	
	dw = ::WaitForMultipleObjects(Enq_Thread_Count + Deq_Thread_Count, MQThreads, TRUE, INFINITE);
	assert(Deq_Count == COUNT);
	cout << "Time required = "<< (::GetTickCount() - dwPrev) <<", count = " << Deq_Count << endl;
	cin >> n;
	return 0;
}

