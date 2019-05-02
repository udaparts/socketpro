// PerfClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Perf.h"

#define REPEAT_NUM		10000
#define BATCH_COUNT		100
#define BATCH_REPEAT	100

int main(int argc, char* argv[])
{
	int n, j;
	::CoInitializeEx(UNULL_PTR, COINIT_MULTITHREADED);
	{
		CSocketPool<CPerf> PerfPool;
		if(PerfPool.StartSocketPool(CComBSTR("localhost"), 21911, CComBSTR("SocketPro"), CComBSTR("PassOne"), 1, 1, NoEncryption, false))
		{
			CUPerformanceQuery PerfQuery;
			CPerf *pPerf = PerfPool.Lock();
			do
			{
				__int64 lNow = PerfQuery.Now();
				for(n=0; n<REPEAT_NUM; ++n)
				{
					const CComBSTR &ret = pPerf->MyEcho(L"EchoTest");
				}
				__int64 lDiff = PerfQuery.Diff(lNow);
				cout<<"Time required without batching = "<<(long)lDiff<<endl;

				lNow = PerfQuery.Now();
				for(n=0; n<BATCH_REPEAT; ++n)
				{
					pPerf->GetAttachedClientSocket()->BeginBatching();
					for(j=0; j<BATCH_COUNT; ++j)
					{
						pPerf->MyEchoAsyn(L"EchoTest");
					}
					pPerf->GetAttachedClientSocket()->Commit(true);
					pPerf->GetAttachedClientSocket()->WaitAll();
				}
				lDiff = PerfQuery.Diff(lNow);
				cout<<"Time required with batching = "<<(long)lDiff<<endl<<endl;
				cout<<"Input a number: 0 -- stop test; and 1 continue testing"<<endl;
				cin >> n;
			}while(n > 0);
		}
	}
	::CoUninitialize();
	return 0;
}
