//#define ENABLE_SOCKETPRO_LB_TRACING
#define _WIN32_DCOM 
#include <objbase.h>
#include "PiImpl.h"

int _tmain(int argc, _TCHAR* argv[])
{
	::CoInitializeEx(UNULL_PTR, COINIT_MULTITHREADED);
	{
		int n;
		CMySocketProServer	MySocketProServer;
		if(!MySocketProServer.Run(20910))
		{
			DWORD dwError = ::GetLastError();
			cout<<"Error happens with code = "<<dwError<<endl;
		}
		cout << "Input a number to close the server application"<<endl;
		cin >> n;
	}
	::CoUninitialize();
	return 0;
}
