
#include "stdafx.h"
#include "HW.h"

int main(int argc, char* argv[]) {
    typedef CSocketPool<HelloWorld, CClientSocket> CMyPool;

    CConnectionContext cc;
    cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";

    CMyPool spHw;
    CMyStruct ms0;
    SetMyStruct(ms0);
    //optionally start a persistent queue at client side for auto failure recovery and once-only delivery
    //spHw.SetQueueName("helloworld");
    bool ok = spHw.StartSocketPool(cc, 1, 1);
    if (!ok) {
        std::cout << "Failed in connecting to remote helloworld server" << std::endl;
        return -1;
    }
    auto hw = spHw.Seek(); //or auto hw = spHw.Lock();

    //process requests one by one synchronously
    std::wcout << hw->async<std::wstring>(idSayHelloHelloWorld, L"Jone", L"Dole").get() << std::endl;
    hw->async(idSleepHelloWorld, (unsigned int) 5000).get();
    CMyStruct ms = hw->async<CMyStruct, CMyStruct>(idEchoHelloWorld, ms0).get();
    assert(ms == ms0);

    //asynchronously process multiple requests with inline batching for best network efficiency
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
    ok = hw->SendRequest(idSleepHelloWorld, (int) 5000, NULL_RH);
    ok = hw->WaitAll();
    std::wcout << L"Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}
