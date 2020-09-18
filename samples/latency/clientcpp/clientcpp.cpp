#include <iostream>
#include <future>
#include "clienthandler.h"
#include <chrono>

using namespace std;
using namespace chrono;
typedef nanoseconds ns;

int main(int argc, char* argv[]) {
    unsigned int res = 0, n = 0;
    const unsigned int TEST_CYCLES = 100000;

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
        res = latency->send<unsigned int, unsigned int>(idEchoInt1, n).get();
    }
    system_clock::time_point stop = system_clock::now();
    ns d = duration_cast<ns>(stop - start);
    cout << "Latency for sync/fast: " << d.count() / TEST_CYCLES << " ns\n\n";

    cout << "Test latency for sync/slow ......\n";
    start = system_clock::now();
    for (n = 0; n < TEST_CYCLES; ++n) {
        res = latency->send<unsigned int, unsigned int>(idEchoInt2, n).get();
    }
    stop = system_clock::now();
    d = duration_cast<ns>(stop - start);
    cout << "Latency for sync/slow: " << d.count() / TEST_CYCLES << " ns\n\n";

    DResultHandler rh = [&res](CAsyncResult & ar) {
        ar >> res;
    };

    cout << "Test latency for send/fast ......\n";
    start = system_clock::now();
    for (n = 0; n < TEST_CYCLES; ++n) {
        ok = latency->SendRequest(idEchoInt1, rh, nullptr, nullptr, n);
    }
    ok = latency->WaitAll();
    stop = system_clock::now();
    d = duration_cast<ns>(stop - start);
    cout << "Latency for send/fast: " << d.count() / TEST_CYCLES << " ns\n\n";

    cout << "Test latency for send/slow ......\n";
    start = system_clock::now();
    for (n = 0; n < TEST_CYCLES; ++n) {
        ok = latency->SendRequest(idEchoInt2, rh, nullptr, nullptr, n);
    }
    ok = latency->WaitAll();
    stop = system_clock::now();
    d = duration_cast<ns>(stop - start);
    cout << "Latency for send/slow: " << d.count() / TEST_CYCLES << " ns\n\n";

    cout << "Press a key to kill the demo ......\n";
    getchar();
    return 0;
}
