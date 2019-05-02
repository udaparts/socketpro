
#include "HW.h"
#include <iostream>

void debug_test() {
	typedef CSocketPool<HelloWorld, CClientSocket> CMyPool;

	CConnectionContext cc;
	cc.Host = "localhost";
	cc.Port = 20901;
	cc.UserId = L"MyUserId";
	cc.Password = L"MyPassword";

	CMyPool spHw;
	CMyStruct ms0;
	SetMyStruct(ms0);

	bool ok = spHw.StartSocketPool(cc, 1, 1);
	if (!ok) {
		std::cout << "Failed in connecting to remote helloworld server" << std::endl;
		return;
	}
	auto hw = spHw.Seek(); //or auto hw = spHw.Lock();

						   //optionally start a persistent queue at client side for auto failure recovery and once-only delivery
	ok = hw->GetAttachedClientSocket()->GetClientQueue().StartQueue("helloworld", 24 * 3600, false); //time-to-live 1 day and true for encryption

																									 //process requests one by one synchronously
	std::wcout << hw->SayHello(L"Jone", L"Dole") << std::endl;
	hw->Sleep(5000);
	CMyStruct ms = hw->Echo(ms0);
	assert(ms == ms0);

	//process multiple requests in batch asynchronously
	ok = hw->StartBatching();
	ok = hw->SendRequest(idSayHelloHelloWorld, L"Jack", L"Smith", [](CAsyncResult & ar) {
		std::wstring ret;
		ar >> ret;
		std::wcout << ret << std::endl;
	});
	CMyStruct ms1;
	SetMyStruct(ms1);
	ok = hw->SendRequest(idEchoHelloWorld, ms1, [&ms, &ms1](CAsyncResult & ar) {
		ar >> ms;
		assert(ms == ms1);
	});
	ok = hw->SendRequest(idSleepHelloWorld, (int)5000, NULL_RH);
	ok = hw->CommitBatching(true); //true -- ask server return multiple results in one shot
	ok = hw->WaitAll();
	
}