#include "stdafx.h"
#include "pi.h"
#include <thread>
using namespace std;

int main(int argc, char* argv[]) {
    CConnectionContext cc;
    cout << "This is worker. Input router server ip address ......\n";
    getline(cin, cc.Host);

    cc.Port = 20901;
    cc.UserId = L"My_LB_UserId";
    cc.Password = L"My_LB_Password";

    typedef CSocketPool<Pi> CMyPool;
    CMyPool spPi;
    if (!spPi.StartSocketPool(cc, 1, thread::hardware_concurrency())) {
        cout << "No connection to " << cc.Host << endl;
    }
    cout << "Press a key to kill the demo ......\n";
    ::getchar();
    return 0;
}
