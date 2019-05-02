// ceclient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../include/ccloader.h"
#include <iostream>

void CALLBACK SPE(unsigned int poolId, SPA::ClientSide::tagSocketPoolEvent spe, USocket_Client_Handle h);

SPA::ClientSide::Internal::CClientCoreLoader loader;
USocket_Client_Handle g_h;
int _tmain(int argc, _TCHAR* argv[])
{
	int n;
	loader.SetMessageQueuePassword("MyQueuePassword");
	unsigned int id = loader.CreateSocketPool(SPE, 1, 1, true, SPA::taNone);
	loader.Connect(g_h, "192.168.1.122", 20901, false, false);
	std::cin >> n;
	std::cout << "Shutting down ......" << std::endl;
	loader.DestroySocketPool(id);
	return 0;
}

void CALLBACK OnHandShakeCompleted (USocket_Client_Handle handler, int nError) {
	std::cout << "HandShakeCompleted with error = " << nError << std::endl;
}

void CALLBACK SPE(unsigned int poolId, SPA::ClientSide::tagSocketPoolEvent spe, USocket_Client_Handle h) {
	if (spe != SPA::ClientSide::speTimer)
		std::cout << "Socket pool event = " << spe << std::endl;
	switch(spe)
	{
	case SPA::ClientSide::speUSocketCreated:
		/*
		loader.SetRecvTimeout(h, 30 * 1000);
        loader.SetConnTimeout(h, 30 * 1000);
		*/
		g_h = h;
		loader.SetAutoConn(h, true);
		loader.SetEncryptionMethod(h, SPA::TLSv1);
		loader.SetOnHandShakeCompleted(h, OnHandShakeCompleted);
		loader.SetUserID(h, L"SocketPro");
		loader.SetPassword(h, L"PassOne");
		if (loader.StartQueue(h, "testq", true, false, 24 * 3600))
			std::cout << "Queue created successfully " << std::endl;
		else
		{
			std::cout << "Failed to create queue with status " << (int)loader.GetClientQueueStatus(h) << std::endl;
		}

		break;
	case SPA::ClientSide::speConnected:
		std::cout << "Session info = " << loader.GetUCertEx(h)->SessionInfo << std::endl;
		loader.SwitchTo(h, SPA::sidReserved + 1);
		break;
	default:
		break;
	}
}
