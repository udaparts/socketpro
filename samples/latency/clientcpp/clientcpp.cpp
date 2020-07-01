#include <iostream>
#include <future>
#include "clienthandler.h"
#include <chrono>
using std::chrono::system_clock;
typedef std::chrono::nanoseconds ns;

int main(int argc, char* argv[]) {
    unsigned int res = 0, n = 0;
    const unsigned int TEST_CYCLES = 100000;

    SPA::ClientSide::CConnectionContext cc;
    std::cout << "Remote host? " << std::endl;
    std::getline(std::cin, cc.Host);
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";

    CLatencyPool latencyPool;
    bool ok = latencyPool.StartSocketPool(cc, 1);
    if (!ok) {
        std::cout << "Failed in connecting to remote helloworld server" << std::endl;
        return -1;
    }
    auto latency = latencyPool.Seek();
    std::cout << "Going to test latency for sync/fast request" << std::endl;
    system_clock::time_point start = system_clock::now();
    for (n = 0; n < TEST_CYCLES; ++n) {
        res = latency->async<unsigned int, unsigned int>(idEchoInt1, n).get();
    }
    system_clock::time_point stop = system_clock::now();
    ns d = std::chrono::duration_cast<ns>(stop - start);
    std::cout << "Latency for sync/fast request in microseconds: " << d.count() / TEST_CYCLES << " ns" << std::endl;
    std::cout << std::endl;

    std::cout << "Going to test latency for sync/slow request" << std::endl;
    start = system_clock::now();
    for (n = 0; n < TEST_CYCLES; ++n) {
        res = latency->async<unsigned int, unsigned int>(idEchoInt2, n).get();
    }
    stop = system_clock::now();
    d = std::chrono::duration_cast<ns>(stop - start);
    std::cout << "Latency for sync/slow request in microseconds: " << d.count() / TEST_CYCLES << " ns" << std::endl;
    std::cout << std::endl;

    SPA::ClientSide::DResultHandler rh = [&res](SPA::ClientSide::CAsyncResult & ar) {
        ar >> res;
    };

    std::cout << "Going to test latency for async/fast request" << std::endl;
    start = system_clock::now();
    for (n = 0; n < TEST_CYCLES; ++n) {
        ok = latency->SendRequest(idEchoInt1, n, rh);
    }
    ok = latency->WaitAll();
    stop = system_clock::now();
    d = std::chrono::duration_cast<ns>(stop - start);
    std::cout << "Latency for async/fast request in microseconds: " << d.count() / TEST_CYCLES << " ns" << std::endl;
    std::cout << std::endl;

    std::cout << "Going to test latency for async/slow request" << std::endl;
    start = system_clock::now();
    for (n = 0; n < TEST_CYCLES; ++n) {
        ok = latency->SendRequest(idEchoInt2, n, rh);
    }
    ok = latency->WaitAll();
    stop = system_clock::now();
    d = std::chrono::duration_cast<ns>(stop - start);
    std::cout << "Latency for async/slow request in microseconds: " << d.count() / TEST_CYCLES << " ns" << std::endl;

    std::cout << "Press a key to shutdown the application ......" << std::endl;
    std::getchar();
    return 0;
}
