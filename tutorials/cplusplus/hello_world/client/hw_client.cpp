#include "stdafx.h"
#include "HW.h"
#include "../../uqueue_demo/mystruct.h"

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

    //spHw.SetQueueName("helloworld"); //optionally start a queue for auto failure recovery
    bool ok = spHw.StartSocketPool(cc, 1); //1
    if (!ok) {
        std::cout << "Failed in connecting to remote helloworld server" << std::endl;
        return -1;
    }
    auto hw = spHw.Seek(); //2 or auto hw = spHw.Lock();

    //process requests one by one synchronously
    std::wcout << hw->async<std::wstring>(idSayHelloHelloWorld, L"Jone", L"Dole").get() << std::endl; //3
    hw->async(idSleepHelloWorld, (unsigned int) 5000).get();
    CMyStruct ms = hw->async<CMyStruct, CMyStruct>(idEchoHelloWorld, ms0).get(); //4
    assert(ms == ms0);

    //asynchronously process multiple requests with inline batching for best network efficiency
    ok = hw->SendRequest(idSayHelloHelloWorld, L"Jack", L"Smith", [](CAsyncResult & ar) { //5
        std::wstring ret;
        ar >> ret;
        std::wcout << ret << std::endl;
    });
    CMyStruct ms1;
    SetMyStruct(ms1);
    ok = hw->SendRequest(idEchoHelloWorld, ms1, [&ms, &ms1](CAsyncResult & ar) { //6
        ar >> ms;
        assert(ms == ms1);
    });
    hw->async(idSleepHelloWorld, (unsigned int) 5000).get(); //7
    std::wcout << L"Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}
