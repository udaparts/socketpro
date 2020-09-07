#include "stdafx.h"
#include "../hello_world/client/HW.h"
#include "../uqueue_demo/mystruct.h"

void SetMyStruct(CMyStruct &ms);

int main(int argc, char* argv[]) {
    typedef CReplication<HelloWorld> CMyReplication;

#ifdef WIN32_64
    CClientSocket::QueueConfigure::SetWorkDirectory("c:\\sp_test");
#else
    CClientSocket::QueueConfigure::SetWorkDirectory("/home/yye/sp_test/");
#endif

    CClientSocket::QueueConfigure::SetMessageQueuePassword("MyQueuePassword");
    ReplicationSetting rs;

    CMyReplication Replication(rs);

    CConnectionContext cc;
    cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";

    CMapQNameConn mapQNameConn;
    mapQNameConn["tolocal"] = cc;
    cc.Host = "192.168.1.110";
    mapQNameConn["tolinux"] = cc;

    bool ok = Replication.Start(mapQNameConn, "cplusplus_rt_root");

    ok = Replication.Send(idSayHello, L"Jack", L"Smith");
    CMyStruct ms;
    SetMyStruct(ms);
    ok = Replication.Send(idEcho, ms);

    std::cout << "Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}

