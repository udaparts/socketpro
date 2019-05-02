// iCOSServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "myspaserver.h"

int main(int argc, char* argv[])
{
	std::string pathToGeoIpDb("iCOSGEOIPDatabase.dat");
	std::string pathToGeoLocation("iCOSGEOLocations.dat");

	//create a SocketPro server and load ip and location databases from files
	iCOS::CMySocketProServer MySocketProServer(pathToGeoIpDb, pathToGeoLocation);
	
	bool ok = InitializeGPU(true, 4);

	iCOS::CGeoIpPeer::MakeFakeDataForTest();

	if (!MySocketProServer.Run(20901))
	{
		int errCode = MySocketProServer.GetErrorCode();
		std::cout<<"Error happens with code = "<< errCode <<std::endl;
	}
	std::cout << "Press any key to stop the server ......"<<std::endl;
	::getchar();
	ok = DeinitializeGPU();
	return 0;
}

