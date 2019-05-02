
#include "PiImpl.h"

int _tmain(int argc, _TCHAR* argv[])
{
	int n;
	CMySocketProServer	MySocketProServer;
	if(!MySocketProServer.Run(20901))
	{
		DWORD dwError = ::GetLastError();
		cout<<"Error happens with code = "<<dwError<<endl;
	}
	cout << "Input a number to close the server application"<<endl;
	cin >> n;
	return 0;
}
