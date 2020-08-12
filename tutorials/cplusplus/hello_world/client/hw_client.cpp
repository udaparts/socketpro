#include "stdafx.h"
#include "HW.h"
#include "../../uqueue_demo/mystruct.h"

int main(int argc, char* argv[]) {
    typedef CSocketPool<HelloWorld, CClientSocket> CMyPool;
    CMyPool spHw;
    CConnectionContext cc("localhost", 20901, L"MyUserId", L"MyPassword");

    //spHw.SetQueueName("helloworld"); //optionally start a queue for auto failure recovery
    bool ok = spHw.StartSocketPool(cc, 1); //1
    if (!ok) {
        std::cout << "Failed in connecting to remote helloworld server" << std::endl;
        return -1;
    }
    auto hw = spHw.Seek(); //2 or auto hw = spHw.Lock();
    CMyStruct ms0, ms;
    SetMyStruct(ms0);
    try{
        //process requests one by one synchronously
        std::wcout << hw->async<std::wstring>(idSayHelloHelloWorld, L"Blabla", L"Dole").get() << std::endl; //3
        hw->async0(idSleepHelloWorld, -5000).get(); //4
        ms = hw->async<CMyStruct>(idEchoHelloWorld, ms0).get(); //5
        assert(ms == ms0);
    }

    catch(SPA::CUException & ex) {
        std::cout << ex.what() << std::endl;
        std::cout << ex.GetStack() << std::endl;
    }

    catch(std::exception & ex) {
        std::cout << ex.what() << std::endl;
    }
    //asynchronously process multiple requests with in-line batching for best network efficiency
    ok = hw->SendRequest(idSayHelloHelloWorld, L"Jack", L"Smith", [](CAsyncResult & ar) { //6
        std::wstring ret;
        ar >> ret;
                std::wcout << ret << std::endl;
    });
    ok = hw->SendRequest(idEchoHelloWorld, ms0, [&ms, &ms0](CAsyncResult & ar) { //7
        ar >> ms;
        assert(ms == ms0);
    });
    hw->async0(idSleepHelloWorld, (unsigned int) 5000).get(); //8
    ok = hw->SendRequest(idSayHelloHelloWorld, L"Donald", L"Trump", [](CAsyncResult & ar) { //9
        std::wstring ret;
        ar >> ret;
                std::wcout << ret << std::endl;
    });
    auto res = hw->async<std::wstring>(idSayHelloHelloWorld, L"Hilary", L"Trump"); //10
    std::wcout << res.get() << std::endl; //11
    std::wcout << L"Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}
