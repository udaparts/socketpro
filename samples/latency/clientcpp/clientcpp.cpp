#include <iostream>
#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
#else
#endif
#include <future>
#include "clienthandler.h"
#include <chrono>

using namespace std;
using namespace chrono;
typedef nanoseconds ns;

const unsigned int TEST_CYCLES = 200000;

#ifdef HAVE_COROUTINE
CAwTask MyTest(CLatencyPool::PHandler& lp) {
    unsigned int total = 0;
    system_clock::time_point start = system_clock::now();
    for (unsigned int n = 0; n < TEST_CYCLES; ++n) {
        total += co_await lp->wait_send<unsigned int>(idEchoInt1, n);
    }
    ns d = duration_cast<ns>(system_clock::now() - start);
    cout << "Latency for co_await sync/fast: total = " << total << ", " << d.count() / TEST_CYCLES << " ns\n\n";
}
#endif

//compile options
//Visual C++ 2017 & 2019 16.8.0 before -- /await
//Visual C++ 2019 16.8.0 preview 3.1 or later -- /std:c++latest
//GCC 10.0.1 or later -- -std=c++20 -fcoroutines -ldl -lstdc++ -pthread
//GCC 11 or clang 14 -- -std=c++20 -ldl -lstdc++ -pthread
int main(int argc, char* argv[]) {
    unsigned int n = 0, total = 0;

    CConnectionContext cc;
    cout << "Remote host? \n";
    getline(cin, cc.Host);
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";

    CLatencyPool latencyPool;
    bool ok = latencyPool.StartSocketPool(cc, 1);
    if (!ok) {
        cout << "No connection to remote server\n";
        return -1;
    }
    auto latency = latencyPool.Seek();
    cout << "Test latency for sync/fast ......\n";
    system_clock::time_point start = system_clock::now();
    for (n = 0; n < TEST_CYCLES; ++n) {
        total += latency->send<unsigned int>(idEchoInt1, n).get();
    }
    system_clock::time_point stop = system_clock::now();
    ns d = duration_cast<ns>(stop - start);
    cout << "Latency for sync/fast: total = " << total << ", " << d.count() / TEST_CYCLES << " ns\n\n";
    cout << "Test latency for sync/slow ......\n";

    total = 0;
    start = system_clock::now();
    for (n = 0; n < TEST_CYCLES; ++n) {
        total += latency->send<unsigned int>(idEchoInt2, n).get();
    }
    stop = system_clock::now();
    d = duration_cast<ns>(stop - start);
    cout << "Latency for sync/slow: total = " << total << ", " << d.count() / TEST_CYCLES << " ns\n\n";

    DResultHandler rh = [&total](CAsyncResult & ar) {
        total += ar.Load<unsigned int>();
    };

    total = 0;
    cout << "Test latency for send/fast ......\n";
    start = system_clock::now();
    for (n = 0; n < TEST_CYCLES; ++n) {
        ok = latency->SendRequest(idEchoInt1, rh, nullptr, nullptr, n);
    }
    ok = latency->WaitAll();
    stop = system_clock::now();
    d = duration_cast<ns>(stop - start);
    cout << "Latency for send/fast: total = " << total << ", " << d.count() / TEST_CYCLES << " ns\n\n";

    total = 0;
    cout << "Test latency for send/slow ......\n";
    start = system_clock::now();
    for (n = 0; n < TEST_CYCLES; ++n) {
        ok = latency->SendRequest(idEchoInt2, rh, nullptr, nullptr, n);
    }
    ok = latency->WaitAll();
    stop = system_clock::now();
    d = duration_cast<ns>(stop - start);
    cout << "Latency for send/slow: total = " << total << ", " << d.count() / TEST_CYCLES << " ns\n\n";

#ifdef HAVE_COROUTINE
    MyTest(latency);
#endif

    cout << "Press a key to kill the demo ......\n";
    int c = getchar();
    return 0;
}
