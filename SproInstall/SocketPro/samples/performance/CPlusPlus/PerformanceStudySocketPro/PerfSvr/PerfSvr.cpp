// PerfSvr.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PerfImpl.h"
int main(int argc, char* argv[])
{
	int n;
	CMySocketProServer	MySocketProServer;
	if(!MySocketProServer.Run(21911))
	{
		DWORD dwError = ::GetLastError();
		cout<<"Error happens with code = "<<dwError<<endl;
	}
	cout << "Input a number to close the server application"<<endl;
	cin >> n;
	return 0;
}
