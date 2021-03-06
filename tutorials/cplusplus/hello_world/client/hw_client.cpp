#include "stdafx.h"
#include "HW.h"
#include "../../uqueue_demo/mystruct.h"

int main(int argc, char* argv[]) {
    typedef CSocketPool<HelloWorld> CMyPool;
    CMyPool spHw;
    CConnectionContext cc("localhost", 20901, L"MyUserId", L"MyPassword");

    //spHw.SetQueueName("helloworld"); //optionally start a queue for auto failure recovery
    if (!spHw.StartSocketPool(cc, 1)) {
        std::wcout << "Failed in connecting to remote helloworld server\n";
        return -1;
    }
    auto hw = spHw.Seek();
    CMyStruct ms0, ms;
    SetMyStruct(ms0);
    try {
        //process requests one by one synchronously
        std::future<std::wstring> f = hw->send<std::wstring>(idSayHello, L"John", L"Dole");
        std::wcout << f.get() << std::endl;
        std::future<CScopeUQueue> fs = hw->sendRequest(idSleep, (unsigned int) 4000);
        fs.get(); //fs.get() >> ms; //SPA::CUException
        ms = hw->send<CMyStruct>(idEcho, ms0).get();
        assert(ms == ms0);

        //asynchronously process multiple requests with in-line batching for best network efficiency
        auto f0 = hw->send<std::wstring>(idSayHello, L"Jack", L"Smith");
        auto f1 = hw->send<CMyStruct>(idEcho, ms0);
        auto f2 = hw->sendRequest(idSleep, (int) 15000);
        auto f3 = hw->send<std::wstring>(idSayHello, L"Donald", L"Trump");
        auto f4 = hw->send<std::wstring>(idSayHello, L"Hilary", L"Clinton");
        //hw->GetSocket()->Cancel();
        std::cout << "Waiting results ......\n";
        std::wcout << f0.get() << std::endl;
        std::wcout << "Echo equal: " << (ms == f1.get()) << std::endl;
        std::wcout << "Sleep returns " << f2.get()->GetSize() << " byte because server side returns nothing\n";
        std::wcout << f3.get() << std::endl;
        std::wcout << f4.get() << std::endl;
    } catch (CServerError & ex) {
        std::wcout << ex.ToString() << std::endl;
    } catch (CSocketError & ex) {
        std::wcout << ex.ToString() << std::endl;
    } catch (std::exception & ex) {
        std::wcout << "Unexpected error: " << ex.what() << "\n";
    }

    std::wcout << L"Press a key to shutdown the demo ......\n";
    ::getchar();
    return 0;
}
