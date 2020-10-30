#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
#else
static_assert(false, "No co_await support");
#endif
#include <iostream>
#include <deque>
#include "../../uqueue_demo/mystruct.h"
#include "../../socketpro/tutorials/cplusplus/hello_world/client/HW.h"

using namespace std;
using namespace SPA;
using namespace SPA::ClientSide;

CMyStruct ms0;

typedef CSocketPool<HelloWorld> CMyPool;

deque<RWaiter<wstring>> CreateAwaitables(CMyPool::PHandler& hw) {
    auto aw0 = hw->wait_send<std::wstring>(idSayHello, L"John", L"Dole");
    auto aw1 = hw->wait_send<std::wstring>(idSayHello, L"Hillary", L"Clinton");
    auto aw2 = hw->wait_send<std::wstring>(idSayHello, L"Donald", L"Trump");
    auto aw3 = hw->wait_send<std::wstring>(idSayHello, L"Joe", L"Biden");
    auto aw4 = hw->wait_send<std::wstring>(idSayHello, L"Mike", L"Pence");
    return {aw0, aw1, aw2, aw3, aw4};
}

CAwTask MyTest(CMyPool::PHandler& hw) {
    try {
        //requests/results streamed with inline batching
        auto qWaiter = CreateAwaitables(hw);
        auto ws = hw->wait_sendRequest(idSleep, (unsigned int)5000);
        auto wms = hw->wait_send<CMyStruct>(idEcho, ms0);

        //co_await for all results
        while(qWaiter.size()) {
            wcout << co_await qWaiter.front() << "\n";
            qWaiter.pop_front();
        }
        wcout << "Waiting sleep ......\n";
        auto sb = co_await ws;
        //sleep request returns nothing
        assert(sb->GetSize() == 0);
        auto ms = co_await wms;
        wcout << "(ms == ms0): " << ((ms == ms0) ? 1 : 0)
            << "\nAll requests processed\n";
    }
    catch (CServerError& ex) {
        wcout << ex.ToString() << "\n";
    }
    catch (CSocketError& ex) {
        wcout << ex.ToString() << "\n";
    }
    catch (std::exception& ex) {
        wcout << "Unexpected error: " << ex.what() << "\n";
    }
}

//compile options
//Visual C++ 2017 & 2019 16.8.0 before -- /await
//Visual C++ 2019 16.8.0 preview 3.1 or later -- /std:c++latest
//GCC 10.0.1 or later -- -std=c++20 -fcoroutines -ldl -pthread
int main(int argc, char* argv[]) {
    CMyPool spHw;
    CConnectionContext cc("localhost", 20901, L"MyUserId", L"MyPassword");
    //spHw.SetQueueName("qhw");
    if (!spHw.StartSocketPool(cc, 1)) {
        std::wcout << "No connection to remote helloworld server\n";
    }
    else {
        auto hw = spHw.Seek();
        SetMyStruct(ms0);
        MyTest(hw);
    }

    std::wcout << L"Press a key to kill the demo ......\n";
    ::getchar();
    return 0;
}
