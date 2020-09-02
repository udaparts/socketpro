#include "stdafx.h"
#include "pi.h"
#include <map>
using namespace std;

int main(int argc, char* argv[]) {
    CConnectionContext cc;
    cout << "Input router server ip address ......\n";
    getline(cin, cc.Host);
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";

    typedef CSocketPool<Pi, CClientSocket> CMyPool;
    CMyPool spPi;
    spPi.SetQueueName("pi_queue");
    bool ok = spPi.StartSocketPool(cc, 1);
    auto pi = spPi.Seek();

    double dPi = 0.0;
    int nDivision = 1000;
    int nNum = 10000000;
    double dStep = 1.0 / nNum / nDivision;
    map<double, double> mapReturn;

    DResultHandler rh = [&dPi, &mapReturn](CAsyncResult & ar) {
        double res, start;
        ar >> res >> start;
        dPi += res;
        mapReturn[start] = res;
    };

    for (int n = 0; n < nDivision; ++n) {
        double dStart = (double) n / nDivision;
        ok = pi->SendRequest(idComputePi, rh, dStart, dStep, nNum);
    }
    ok = pi->WaitAll();
    cout << "Your pi = " << dPi << ", returns = " << mapReturn.size() << endl;
    cout << "Press a key to shutdown the demo application ......\n";
    ::getchar();
    return 0;
}
