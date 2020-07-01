
#include "stdafx.h"
#include "pi.h"
#include <map>

int main(int argc, char* argv[]) {
	int n;
	double dStart;
	CClientSocket::QueueConfigure::SetMessageQueuePassword("MyPwdForMsgQueue");
#ifdef WIN32_64
	CClientSocket::QueueConfigure::SetWorkDirectory("c:\\sp_test");
#else
	CClientSocket::QueueConfigure::SetWorkDirectory("/home/yye/sp_test/");
#endif
	CConnectionContext cc;
	std::cout << "Input router server ip address ......" << std::endl;
	std::getline(std::cin, cc.Host);
	cc.Port = 20901;
	cc.UserId = L"MyUserId";
	cc.Password = L"MyPassword";

	typedef CSocketPool<Pi, CClientSocket> CMyPool;
	CMyPool spPi;

	bool ok = spPi.StartSocketPool(cc, 1);
	auto pi = spPi.Seek(); //or auto pi = spPi.Lock();

	//use persistent queue to ensure auto failure recovery and at-least-once or once-only delivery
	ok = pi->GetAttachedClientSocket()->GetClientQueue().StartQueue("pi_queue", 24 * 3600, false); //time-to-live 1 day and true for encryption
	pi->GetAttachedClientSocket()->GetClientQueue().EnableRoutingQueueIndex(true);

	double dPi = 0.0;
	int nDivision = 1000;
	int nNum = 10000000;
	double dStep = 1.0 / nNum / nDivision;
	std::map<double, double> mapReturn;
	
	DResultHandler rh = [&dPi, &mapReturn](CAsyncResult & ar) {
		double res, start;
		ar >> res >> start;
		dPi += res;
		mapReturn[start] = res;
	};

	for (n = 0; n < nDivision; ++n) {
		dStart = (double) n / nDivision;
		ok = pi->SendRequest(idComputePi, dStart, dStep, nNum, rh);
	}
	ok = pi->WaitAll();
	std::cout << "Your pi = " << dPi << ", returns = " << mapReturn.size() << std::endl;
	std::cout << "Press a key to shutdown the demo application ......" << std::endl;
	::getchar();
	return 0;
}
