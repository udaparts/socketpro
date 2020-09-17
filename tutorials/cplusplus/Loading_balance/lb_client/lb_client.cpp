#include "stdafx.h"
#include "pi.h"
using namespace std;

int main(int argc, char* argv[]) {
    CConnectionContext cc;
    cout << "Input router server ip address ......\n";
    getline(cin, cc.Host);
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";

    typedef CSocketPool<Pi> CMyPool;
    CMyPool spPi;
    spPi.SetQueueName("pi_queue");
    if (!spPi.StartSocketPool(cc, 1)) {
        cout << "No connection to " << cc.Host <<
                "\nPress a key to shutdown the demo ......\n";
        ::getchar();
        return 1;
    }
    auto pi = spPi.SeekByQueue();

    vector<future<CScopeUQueue> > vfR;
    int nDivision = 1000, nNum = 10000000;
    double dPi = 0.0, dStep = 1.0 / nNum / nDivision;
    for (int n = 0; n < nDivision; ++n) {
        double dStart = (double) n / nDivision;
        vfR.push_back(pi->send0(idComputePi, dStart, dStep, nNum));
    }

    double res;
    for (auto& f : vfR) {
        CScopeUQueue sb = f.get();
        sb >> res;
        dPi += res;
        cout << "dStart: " << sb->Load<double>() << "\n";
    }
    cout.precision(14);
    cout << "pi: " << dPi << ", returns: " << vfR.size() <<
            "\nPress a key to shutdown the demo ......\n";
    ::getchar();
    return 0;
}
