// SproClient.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include "mysvshandler.h"
#include "TEchoB.h"
#include "TEchoC.h"
#include "TEchoD.h"
#include "TOne.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>

SPA::CUCriticalSection g_mutex;

const char *str = "Echo p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue + p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue + p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue + p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue +  p->GetAttachedClientSocket()->GetQueueManager().StartQueue";

void TestEchoBasic(const SPA::ClientSide::CConnectionContext &cc);

void TestEchoSys(const SPA::ClientSide::CConnectionContext &cc);
void TestEchoObject(const SPA::ClientSide::CConnectionContext &cc);
void TestAutoConnecting(const SPA::ClientSide::CConnectionContext &cc);
void TestQueue(const SPA::ClientSide::CConnectionContext &cc);
void TestEchoObjectArray(const SPA::ClientSide::CConnectionContext &cc);
void TestRoute0(const SPA::ClientSide::CConnectionContext &cc);
void TestRoute1(const SPA::ClientSide::CConnectionContext &cc);
void TestTOne(const SPA::ClientSide::CConnectionContext &cc);
bool CALLBACK CVCallback(bool preverified, int depth, int errorCode, const char *errMessage, SPA::CertInfo *ci);

int main(int argc, char* argv[]) {
    bool ok;
    int route;
    SPA::ClientSide::CConnectionContext cc;
    std::cout << "Remote host ip address ......" << std::endl;
    std::getline(std::cin, cc.Host);
    cc.Port = 20901;
    cc.Zip = false;
    cc.EncrytionMethod = SPA::TLSv1;

    SPA::ClientSide::CClientSocket::SSL::SetCertificateVerifyCallback(CVCallback);

#ifdef WIN32_64
    //ok = SPA::ClientSide::CClientSocket::SSL::SetVerifyLocation("root@localmachine");
    ok = SPA::ClientSide::CClientSocket::SSL::SetVerifyLocation("root");
    //SPA::ClientSide::CClientSocket::QueueConfigure::SetWorkDirectory("c:\\cyetest");
#else
    //ok = SPA::ClientSide::CClientSocket::SSL::SetVerifyLocation("/home/yye/3rdparty/");
    ok = SPA::ClientSide::CClientSocket::SSL::SetVerifyLocation("ca.cert.pem");
    //ok = SPA::ClientSide::CClientSocket::SSL::SetVerifyLocation("/etc/ssl/certs");
    //SPA::ClientSide::CClientSocket::QueueConfigure::SetWorkDirectory("/home/yye/cyetest");
#endif
    const char *strWorkPath = SPA::ClientSide::CClientSocket::QueueConfigure::GetWorkDirectory();
    SPA::ClientSide::CClientSocket::QueueConfigure::SetMessageQueuePassword("MyPasswordForMessageQueue");

    /*
    SPA::ClientSide::CClientSocket::QueueConfigure::SetMessageQueuePassword("MyPasswordForMessageQueue");
    SPA::ClientSide::CClientSocket::SSL::SetCertificateVerifyCallback(CVCallback);
    SPA::ClientSide::CClientSocket::SSL::SetVerifyLocation("C:\\cye_cedev\\ca.pem");
     */

    /*
    CConnectionContext ccr;
    ccr.Host = "localhost";
    ccr.Port = 20901;
    ccr.UserId = L"MyUserId";
    ccr.Password = L"MyPassword";

    CMapQNameConn mapQNameConn;
    mapQNameConn["tolocal"] = ccr;
    ccr.Host = "192.168.1.110";
    mapQNameConn["tolinux"] = ccr;

    SPA::ClientSide::ReplicationSetting rs;
    CReplication<CEchoBasic, CClientSocket> Replication(rs);
    bool ok = Replication.Start(mapQNameConn, "cplusplus_rt_root");
    SPA::ClientSide::IClientQueue *pClientQueue = Replication.GetSourceQueue();
    auto handler = Replication.GetSourceHandler();
    ok = Replication.DoReplication();
    unsigned int queues = Replication.GetQueues();
     */
    /*
    {
        SPA::ClientSide::CConnectionContext ppCC[1][1];
        SPA::ClientSide::CSocketPool<CEchoBasic> spEchoBasic;
        spEchoBasic.StartSocketPool((SPA::ClientSide::CConnectionContext **)ppCC, 1, 1, true);
    }
     */

#ifdef WIN32_64
    cc.UserId = L"Win_SocketPro";
#else
    cc.UserId = L"Nix_SocketPro";
#endif
    cc.Password = L"MyPassword";
    TestTOne(cc);
    std::cout << "Route number ?" << std::endl;
    std::cin >> route;
    if (route % 2)
        TestRoute1(cc);
    else if (route)
        TestRoute0(cc);
    TestQueue(cc);
    TestAutoConnecting(cc);
    TestEchoBasic(cc);
    TestEchoSys(cc);
    TestEchoObject(cc);
    TestEchoObjectArray(cc);
    return 0;
}

void TestTOne(const SPA::ClientSide::CConnectionContext &cc) {
#if 0
    int n;
    int count = 1000;
    for (n = 0; n < count; ++n) {
        {
            SPA::ClientSide::CSocketPool<CTOne> sp(true);
            bool b = sp.StartSocketPool(cc, 2, 4);
            auto p = sp.Lock();
            if (!p) {
                std::cout << "No socket connection for TOne service" << std::endl;
                return;
            }
            b = p->GetAttachedClientSocket()->GetClientQueue().StartQueue("tonequeue", 24 * 3600, cc.EncrytionMethod != NoEncryption);
            if (!b) {
                SPA::tagQueueStatus qs = p->GetAttachedClientSocket()->GetClientQueue().GetQueueOpenStatus();
                qs = qsNormal;
            }
            b = p->SendRequest(idQueryCountCTOne, [](SPA::ClientSide::CAsyncResult & ar) {

            });
            assert(b);
        }
    }
#else
    SPA::ClientSide::CSocketPool<CTOne> sp(true);
    bool b = sp.StartSocketPool(cc, 1, 1);
    auto p = sp.Lock();
    if (!p) {
        std::cout << "No socket connection for TOne service" << std::endl;
        return;
    }
    b = p->GetAttachedClientSocket()->GetClientQueue().StartQueue("tonequeue", 24 * 3600, cc.EncrytionMethod != NoEncryption);
    if (!b) {
        SPA::tagQueueStatus qs = p->GetAttachedClientSocket()->GetClientQueue().GetQueueOpenStatus();
        qs = qsNormal;
    }
    for (int n = 0; n < 10; ++n) {
        b = p->SendRequest(idQueryCountCTOne, [](SPA::ClientSide::CAsyncResult & ar) {
        });
    }
    b = p->Interrupt(12345);
    assert(b);
    b = p->WaitAll();
    assert(b);
#endif
}

void TestQueue(const SPA::ClientSide::CConnectionContext &cc) {
    int n;
    std::string shortMessage = "SocketPro is a world-leading package of secured communication software components written with request batching, asynchrony and parallel computation in mind. It offers superior performance and scalabi";
    std::string sEchoTest = "SocketPro -- a package of revolutionary software components written from batching, asynchrony and parallel computation with many unique and critical features. These features assist you to quickly develop high speed and scalable distributed applications on Windows and smart devices as well as web browsers."
            "Tutorial One -- The first tutorial is a hello project to support five simple requests. One of them is a slow request processed with a worker thread at the server side. The tutorial tells you how to code step-by-step at both server and client sides based on the classes of SocketProAdapter and its sub namespaces. The tutorial "
            "leads you to do two experiments that require you to use the well known tool Telnet for connecting from a client to a remote SocketPro server. The tutorial tells you SocketPro threads management at server side. It tells you what variables should be synchronized with a critical section or monitor. For client development, the "
            "tutorial is focused on how to use SocketProAdapter at client side, how to turn on/off online compressing, how to batch requests, how to do asynchrony computation, how to do synchrony computation, and how to switch between the two computation models. This tutorial leads you to do an particular experiment, freezing and de-freezing Window GUIs at run time.";

    typedef SPA::ClientSide::CSocketPool<CMyServiceHandler, CMySocket> CMySocketPool;

    CMySocketPool sp(true, 15000);

    sp.DoSslServerAuthentication = [](CMySocketPool *sp, CMySocket * cs) -> bool {
        int errCode;
        SPA::IUcert *cert = cs->GetUCert();
        if (cert)
            std::cout << cert->SessionInfo << std::endl;
        const char *res = cert->Verify(&errCode);
        bool ok = (errCode == 0);
        std::cout << "NotBefore: " << cert->NotBefore << std::endl;
        std::cout << "NotAfter: " << cert->NotAfter << std::endl;
        return ok;
    };

    sp.SocketPoolEvent = [](CMySocketPool *sp, tagSocketPoolEvent spe, CMyServiceHandler * handler) {
        switch (spe) {
            case SPA::ClientSide::speSocketClosed:
                std::cout << "Socket closed with error message: " << handler->GetAttachedClientSocket()->GetErrorMsg();
                std::cout << ", error code: " << handler->GetAttachedClientSocket()->GetErrorCode() << std::endl;
                break;
            default:
                break;
        }
    };

    std::string qName;
    std::cout << "A unique queue name, please?" << std::endl;
    std::cin >> qName;

    bool b = sp.StartSocketPool(cc, 1, 1);
    auto p = sp.Lock();
    if (!p) {
        std::cout << "No socket connection for queue" << std::endl;
        return;
    }
    IClientQueue &cq = p->GetAttachedClientSocket()->GetClientQueue();
    b = cq.StartQueue(qName.c_str(), 30 * 24 * 3600, cc.EncrytionMethod != NoEncryption);
    if (b) {
        auto found = sp.SeekByQueue(qName);
        assert(found);
    }
    do {
        unsigned int j;
        SPA::UINT64 memStart = CScopeUQueue::GetContention(),
                cacheStart = CAsyncServiceHandler::GetCacheContention(),
                handlerStart = p->GetContention();
        std::cout << "Input a number for test option and negative number for quitting the test ......" << std::endl;
        std::cin >> n;
        if (n < 0)
            break;
        bool shortOne = (n >= 4) ? true : false;
        bool zip = ((n & 0x1) > 0);
        bool batching = ((n & 0x2) > 0);
        p->GetAttachedClientSocket()->SetZip(zip);
        boost::posix_time::ptime t0 = boost::posix_time::microsec_clock::local_time();
        for (j = 0; j < 5000; ++j) {
            b = cq.StartJob();
            b = p->OpenDbAsync("This is a test connection string");
            if (batching && shortOne)
                b = p->StartBatching();
            for (unsigned int m = 0; m < 200; ++m) {
                if (m && !(m % 20)) {
                    p->SendRequest(idSleep, (unsigned int) 0, [](SPA::ClientSide::CAsyncResult & ar) {
#if 0
                        int *p = nullptr;
                        srand((unsigned int) time(nullptr));
                                unsigned int random = (unsigned int) rand();
                                int m = (random % 10);
                        switch (m) {
                            case 9:
                                std::wcout << "*p: " << *p << std::endl;
                                break;
                            default:
                                if (!m) {
                                    std::wcout << "n/m: " << 10 / m << std::endl;
                                }
                                break;
                        }
#endif
                    });
                }
                //auto f0 = p->async<std::string, std::string>(idEcho, shortMessage); //crash with non-window platforms
                if (shortOne) {
                    b = p->SendRequest(idEcho, shortMessage, [&shortMessage](SPA::ClientSide::CAsyncResult & ar) {
                        std::string s;
                        ar >> s;
                        if (s != shortMessage) {
                            std::cout << s << std::endl;
                        }
                    });
                } else {
                    b = p->SendRequest(idEcho, sEchoTest, [&sEchoTest](SPA::ClientSide::CAsyncResult & ar) {
                        std::string s;
                        ar >> s;
                        if (s != sEchoTest) {
                            std::cout << s << std::endl;
                        }
                    });
                }
            }
            if (batching && shortOne)
                b = p->CommitBatching(true);
            p->DodequeueAsync(n);
            b = cq.EndJob();
            if (cq.GetQueueSize() > 4 * 1024 * 1024 && p->GetAttachedClientSocket()->IsConnected()) {
                p->GetAttachedClientSocket()->Cancel();
                p->WaitAll();
            }
        }
        p->WaitAll();
        boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::time_duration diff = t1 - t0;
        std::cout << "Time required = " << diff.total_milliseconds() << std::endl;
        std::cout << "Memory spin contention: " << CScopeUQueue::GetContention() - memStart <<
                ", Cache: " << CAsyncServiceHandler::GetCacheContention() - cacheStart <<
                ", Handler: " << p->GetContention() - handlerStart << std::endl;
    } while (true);
    std::getchar();
    unsigned int total = SPA::ClientSide::CAsyncServiceHandler::CountResultCallbacksInPool();
    std::cout << "ResultCallbacksInPool = " << total << std::endl;
    std::cout << "Press a key to continue ......" << std::endl;
    std::getchar();
    SPA::ClientSide::CAsyncServiceHandler::ClearResultCallbackPool(total / 2);
    total = SPA::ClientSide::CAsyncServiceHandler::CountResultCallbacksInPool();
    std::cout << "ResultCallbacksInPool after removing half callbacks = " << total << std::endl;
    std::cout << "Press a key to continue ......" << std::endl;
    std::getchar();
    SPA::ClientSide::CAsyncServiceHandler::ClearResultCallbackPool(0);
    std::cout << "Shrink deque ......" << std::endl;
    p->ShrinkDeque();
    std::cout << "Press a key to continue ......" << std::endl;
    std::getchar();
}

void TestAutoConnecting(const SPA::ClientSide::CConnectionContext & cc) {
    int n;
    SPA::ClientSide::CSocketPool<CMyServiceHandler, CMySocket> sp(true);
    bool b = sp.StartSocketPool(cc, 5, 6);
    //auto p = sp.GetAsyncHandlers()[0];
    auto p = sp.Lock();
    if (!p) {
        std::cout << "No socket connection for auto connecting" << std::endl;
        return;
    }

    b = p->GetAttachedClientSocket()->GetClientQueue().StartQueue("echosys", 30 * 24 * 3600, cc.EncrytionMethod != NoEncryption);
    if (b) {
        auto found = sp.SeekByQueue("echosys");
        assert(found);
    }
    std::string s = p->Echo("This is a test string");

    std::cout << "input a number and press [ENTER] to shutdown the AutoConnecting pool" << std::endl;
    std::cin >> n;

    std::cout << "Connected connections " << sp.GetConnectedSockets() << std::endl;

    sp.ShutdownPool();

    std::cout << "input a number and press [ENTER] to continue the AutoConnecting pool" << std::endl;
    std::cin >> n;

    std::cout << "Restart a pool of sockets " << std::endl;
    b = sp.StartSocketPool(cc, 5, 6);
    //p = sp.GetAsyncHandlers()[0];
    p = sp.Lock();
    if (p)
        s = p->Echo("This is a test string");
    std::cout << "Connected connections " << sp.GetConnectedSockets() << std::endl;
    std::cout << "Press any key to complete the AutoConnecting test ......" << std::endl;
    ::getchar();
}

void TestEchoBasic(const SPA::ClientSide::CConnectionContext & cc) {
    SPA::ClientSide::CSocketPool<CEchoBasic> spEchoBasic(false);

    unsigned int n = 0;
    const unsigned int count = 5;
    bool b = spEchoBasic.StartSocketPool(cc, 5, 6);
    auto p = spEchoBasic.Lock();
    if (!p) {
        std::cout << "No socket connection for basic echo" << std::endl;
        return;
    }

    std::string s;
    boost::posix_time::ptime t0 = boost::posix_time::microsec_clock::local_time();
    for (n = 0; n < count; ++n) {
        s = p->EchoAString("TestASCII");
        assert(s == "TestASCII");
    }
    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = t1 - t0;
    std::cout << "Time required = " << diff.total_milliseconds() << ", input a number for other tests" << std::endl;

    std::cin >> n;

    //assert(s == "TestASCII");
    std::wstring ws;

    t0 = boost::posix_time::microsec_clock::local_time();
    for (n = 0; n < count; ++n) {
        b = p->EchoBool(true);
        assert(b);
    }
    t1 = boost::posix_time::microsec_clock::local_time();
    diff = t1 - t0;
    std::cout << "Time required = " << diff.total_milliseconds() << ", input a number for other tests" << std::endl;

    std::cin >> n;
    n = 1;
    //𤭢, 𤭢,  -- 4 bytes
    for (n = 0; n < count; ++n) {
        ws = p->EchoString(L"Test");
        assert(ws == L"Test");
    }

    char c;
    n = 0;
    for (n = 0; n < count; ++n) {
        c = p->EchoInt8('A');
        assert(c == 'A');
    }

    wchar_t wc = p->EchoWChar(L'特');
    assert(wc == L'特');

    unsigned char byte = p->EchoUInt8(234);
    assert(byte == 234);

    short sData = p->EchoInt16(1234);
    assert(sData == 1234);

    unsigned short usData = p->EchoUInt16(61234);
    assert(usData == 61234);

    int intData = p->EchoInt32(12345678);
    assert(intData == 12345678);

    unsigned int uintData = p->EchoUInt32(4000034500);
    assert(uintData == 4000034500);

    float f = p->EchoFloat(123.456f);
    assert(f == 123.456f);

    double d = p->EchoDouble(7123.456);
    assert(abs(d - 7123.456) < 0.0000000001);

    SPA::UINT64 uint64 = p->EchoUInt64(0xFFFFFFFFFF);
    assert(uint64 == 0xFFFFFFFFFF);

    SPA::INT64 int64 = p->EchoInt64(-1234567890123);
    assert(int64 == -1234567890123);

    s = p->EchoAString(nullptr);
    assert(s.empty());

    ws = p->EchoString(nullptr);
    assert(ws.empty());

    s = p->EchoAString("");
    assert(s.empty());

    ws = p->EchoString(L"");
    assert(ws.empty());

    GUID guid;
#ifdef WIN32_64
    ::CoCreateGuid(&guid);
#else
    guid = boost::uuids::random_generator()();
#endif
    GUID rid = p->EchoGuid(guid);
    assert(rid == guid);

    CY cy;
    cy.int64 = 123456789; //12345.6789
    CY rcy = p->EchoCy(cy);
    assert(::memcmp(&cy, &rcy, sizeof (CY)) == 0);

    SPA::UDateTime dt;
    dt.time = std::time(nullptr);
    dt.time *= 1000; //to ms
    SPA::UDateTime dtRes = p->EchoDateTime(dt);
    assert(dtRes == dt);

    std::cout << "Input a number and press ENTER to close the TestEchoBasic ......" << std::endl;
    std::cin >> intData;
}

void TestEchoObjectArray(const SPA::ClientSide::CConnectionContext & cc) {
    unsigned int len;
    SPA::ClientSide::CSocketPool<CEchoObject> spEchoObject(false);
    bool b = spEchoObject.StartSocketPool(cc, 6, 8);
    auto p = spEchoObject.Lock();
    if (!p) {
        std::cout << "No socket connection for object array echo" << std::endl;
        return;
    }
    SPA::UVariant vt;
    SPA::UVariant res;
    {
        std::string ascii("TestASCII");
        char *str;
        len = (unsigned int) ascii.size();
        SAFEARRAYBOUND sab[1] = {len, 0};
        vt.vt = (VT_ARRAY | VT_I1);
        vt.parray = ::SafeArrayCreate(VT_I1, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &str);
        ::memcpy(str, ascii.c_str(), len * sizeof (char));
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoAString(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        unsigned char *bytes;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_UI1);
        vt.parray = ::SafeArrayCreate(VT_UI1, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &bytes);
        bytes[0] = 212;
        bytes[1] = 1;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoUInt8Array(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        VARIANT_BOOL *bools;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_BOOL);
        vt.parray = ::SafeArrayCreate(VT_BOOL, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &bools);
        bools[0] = VARIANT_TRUE;
        bools[1] = VARIANT_FALSE;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoBoolArray(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        short *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_I2);
        vt.parray = ::SafeArrayCreate(VT_I2, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0] = 23456;
        data[1] = -1234;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoInt16Array(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        unsigned short *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_UI2);
        vt.parray = ::SafeArrayCreate(VT_UI2, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0] = 61456;
        data[1] = 2;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoUInt16Array(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        int *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_INT);
        vt.parray = ::SafeArrayCreate(VT_INT, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0] = 61456;
        data[1] = -2;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoInt32Array(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        unsigned int *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_UINT);
        vt.parray = ::SafeArrayCreate(VT_UINT, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0] = 61456;
        data[1] = 0xFFFFFFF0;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoUInt32Array(vt);
        assert(SPA::IsEqual(vt, res));
    }


    {
        long *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_I4);
        vt.parray = ::SafeArrayCreate(VT_I4, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0] = 61456;
        data[1] = -2;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoInt32Array(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        unsigned long *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_UI4);
        vt.parray = ::SafeArrayCreate(VT_UI4, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0] = 61456;
        data[1] = 0xFFFFFFF0;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoUInt32Array(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        SPA::INT64 *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_I8);
        vt.parray = ::SafeArrayCreate(VT_I8, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0] = 61456;
        data[1] = -2;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoInt64Array(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        SPA::UINT64 *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_UI8);
        vt.parray = ::SafeArrayCreate(VT_UI8, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0] = 61456;
        data[1] = 0xFFFFFFFFFFF;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoUInt64Array(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        float *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_R4);
        vt.parray = ::SafeArrayCreate(VT_R4, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0] = 61456.23f;
        data[1] = -17.24f;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoFloatArray(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        double *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_R8);
        vt.parray = ::SafeArrayCreate(VT_R8, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0] = 61456.23;
        data[1] = -17.24;
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoDoubleArray(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        tagCY *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_CY);
        vt.parray = ::SafeArrayCreate(VT_CY, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0].int64 = 614562303; //61456.2303
        data[1].int64 = -172401; //-17.2401
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoCYArray(vt);
        assert(SPA::IsEqual(vt, res));
    }

    {
        BSTR *data;
        SAFEARRAYBOUND sab[1] = {2, 0};
        vt.vt = (VT_ARRAY | VT_BSTR);
        vt.parray = ::SafeArrayCreate(VT_BSTR, 1, sab);
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        data[0] = ::SysAllocString(L"TestBSTRArray0");
        data[1] = ::SysAllocString(L"");
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoStringArray(vt);
        //assert(SPA::IsEqual(vt, res));
    }

    {
        SYSTEMTIME st;
        vt.vt = (VT_ARRAY | VT_DATE);
        SAFEARRAYBOUND sab[1] = {1, 0};
        vt.parray = ::SafeArrayCreate(VT_DATE, 1, sab);
#ifdef WIN32_64
        DATE *data;
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        ::GetLocalTime(&st);
        ::SystemTimeToVariantTime(&st, data);
#else
        SPA::UINT64 *data = nullptr;
        ::SafeArrayAccessData(vt.parray, (void**) &data);
        ::gettimeofday(&st, nullptr);
        data[0] = UDateTime(st).time;
#endif
        ::SafeArrayUnaccessData(vt.parray);
        res = p->EchoDateTimeArray(vt);
        assert(SPA::IsEqual(vt, res));
    }
}

void TestEchoObject(const SPA::ClientSide::CConnectionContext & cc) {
    SPA::ClientSide::CSocketPool<CEchoObject> spEchoObject(false);
    bool b = spEchoObject.StartSocketPool(cc, 5, 6);
    auto p = spEchoObject.Lock();
    if (!p) {
        std::cout << "No socket connection for object echo" << std::endl;
        return;
    }

    SPA::UVariant vt;
    SPA::UVariant res = p->EchoEmpty(vt);
    assert(SPA::IsEqual(vt, res));

    vt = true;
    res = p->EchoBool(vt);
    assert(SPA::IsEqual(vt, res));

    vt = 'M';
    res = p->EchoInt8(vt);
    assert(SPA::IsEqual(vt, res));

    vt = (unsigned char) 244;
    res = p->EchoUInt8(vt);
    assert(SPA::IsEqual(vt, res));

    vt = (unsigned short) 54444;
    res = p->EchoUInt16(vt);
    assert(SPA::IsEqual(vt, res));

    vt = (short) 24444;
    res = p->EchoInt16(vt);
    assert(SPA::IsEqual(vt, res));

    vt = (int) 2000067890;
    res = p->EchoInt32(vt);
    assert(SPA::IsEqual(vt, res));

    vt = (unsigned int) 4000067890;
    res = p->EchoUInt32(vt);
    assert(SPA::IsEqual(vt, res));

    vt = (SPA::INT64)200006789000000;
    res = p->EchoInt64(vt);
    assert(SPA::IsEqual(vt, res));

    vt = (SPA::UINT64)400006789000000;
    res = p->EchoUInt64(vt);
    assert(SPA::IsEqual(vt, res));

    vt = 28.25f;
    res = p->EchoFloat(vt);
    assert(SPA::IsEqual(vt, res));

    vt = 32.453;
    res = p->EchoDouble(vt);
    assert(SPA::IsEqual(vt, res));

    tagCY cy;
    cy.int64 = 1234567890;
    vt = cy;
    res = p->EchoCY(cy);
    assert(SPA::IsEqual(vt, res) || vt.dblVal == 123456.789);

    vt = L"Test";
    res = p->EchoString(vt);
    assert(SPA::IsEqual(vt, res));

#ifdef WIN32_64
    vt.Clear();
    vt.vt = VT_DATE;
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    ::SystemTimeToVariantTime(&st, &vt.date);
#else
    {
        SPA::UDateTime udt(std::time(nullptr));
        vt = udt;
    }
#endif
    res = p->EchoDateTime(vt);
    assert(SPA::IsEqual(vt, res));
}

void TestEchoSys(const SPA::ClientSide::CConnectionContext & cc) {
    unsigned int n;
    bool b;
    SPA::ClientSide::CSocketPool<CEchoSys> spEchoSys(false);
    std::vector<std::shared_ptr<CEchoSys> > vHandle;
    boost::posix_time::ptime t0 = boost::posix_time::microsec_clock::local_time();
    for (n = 0; n < 50; ++n) {
        b = spEchoSys.StartSocketPool(cc, 1, 1);
        while (vHandle.size() != 1 && spEchoSys.GetConnectedSockets()) {
            auto p = spEchoSys.Lock();
            if (p) {
                std::wstring str;
                const wchar_t *input = L"MyWSTRTest";
                SPA::UVariant vt(L"MyTestVariant");
                SPA::UVariant res = p->EchoComplex0(1.23, input, vt, true, str);
                assert(str == input);
                assert(res == vt);

                MyStruct ms;
                ms.ABool = true;
                ms.AInt = 987654321;
                ms.AString = "TestASCII";
                ms.WString = L"TestWSTR";

                MyStruct rtn = p->EchoMyStruct(ms);
                assert(rtn == ms);

                SPA::CScopeUQueue su;

                su << ms;
                SPA::CUQueue q = p->EchoUQueue(*su);
                if (q.GetSize()) {
                    q >> rtn;
                    assert(rtn == ms);
                } else {
                    break;
                }
                vHandle.push_back(p);
            }
        }
        vHandle.clear();
        spEchoSys.DisconnectAll();
        //spEchoSys.ShutdownPool();
    }
    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = t1 - t0;
    std::cout << "Echosys time required = " << diff.total_milliseconds() << ", input a number for other tests" << std::endl;
    spEchoSys.ShutdownPool();
    std::cin >> n;
}

void TestRoute0(const SPA::ClientSide::CConnectionContext & cc) {
    int stop;
    std::string qName;
    SPA::ClientSide::CSocketPool<CRouteHandler0> sp;
    bool b = sp.StartSocketPool(cc, 1, 1);
    auto p = sp.Lock();
    if (!p) {
        std::cout << "No socket connection for RouteHandler 0" << std::endl;
        return;
    }

    std::cout << "Route 0 queue name? " << std::endl;
    std::cin >> qName;

    //could lead a dead lock at server side if server main threads is larger than 1
    b = p->GetAttachedClientSocket()->GetClientQueue().StartQueue(qName.c_str(), 30 * 24 * 3600, cc.EncrytionMethod != NoEncryption);
    if (b) {
        auto found = sp.SeekByQueue(qName);
        assert(found);
    }

    SPA::ClientSide::CSocketPool<CRouteHandler1> sp1;
    b = sp1.StartSocketPool(cc, 1, 1);

    std::cout << "Input a number for test (negative value for stopping) with queue size = "
            << p->GetAttachedClientSocket()->GetClientQueue().GetMessageCount() << std::endl;

    std::cout << "Routee count = " << p->GetAttachedClientSocket()->GetRouteeCount() << std::endl;

    std::cin >> stop;
    while (stop >= 0) {
        std::string res = p->DoMyEcho0(str);
        std::cout << "Result -- 0: " << res << std::endl;

        std::cout << "Input a number for test (negative value for stopping)" << std::endl;
        std::cin >> stop;
    }
    sp1.ShutdownPool();
}

void TestRoute1(const SPA::ClientSide::CConnectionContext & cc) {
    int stop;
    std::string qName;
    SPA::ClientSide::CSocketPool<CRouteHandler1> sp;
    bool b = sp.StartSocketPool(cc, 1, 1);
    auto p = sp.Lock();
    if (!p) {
        std::cout << "No socket connection for RouteHandler 1" << std::endl;
        return;
    }

    std::cout << "Route 1 queue name? " << std::endl;
    std::cin >> qName;

    //could lead a dead lock at server side if server main threads is larger than 1
    b = p->GetAttachedClientSocket()->GetClientQueue().StartQueue(qName.c_str(), 30 * 24 * 3600, cc.EncrytionMethod != NoEncryption);
    if (b) {
        auto found = sp.SeekByQueue(qName);
        assert(found);
    }

    SPA::ClientSide::CSocketPool<CRouteHandler0> sp0;
    b = sp0.StartSocketPool(cc, 1, 1);

    std::cout << "Input a number for test (negative value for stopping) with queue size = "
            << p->GetAttachedClientSocket()->GetClientQueue().GetMessageCount() << std::endl;

    std::cout << "Routee count = " << p->GetAttachedClientSocket()->GetRouteeCount() << std::endl;

    std::cin >> stop;
    while (stop >= 0) {

        p->GetAttachedClientSocket()->SetZip(stop >= 2);

        b = p->GetAttachedClientSocket()->GetClientQueue().StartJob();

        b = p->SendRequest(idREcho1, str, [](SPA::ClientSide::CAsyncResult & ar) {
            std::string res;
            ar >> res;
                    std::cout << "Result -- 1 - 0: " << res << std::endl;
        });

        b = p->SendRequest(idREcho1, str, [](SPA::ClientSide::CAsyncResult & ar) {
            std::string res;
            ar >> res;
                    std::cout << "Result -- 1 - 1: " << res << std::endl;
        });

        b = p->GetAttachedClientSocket()->GetClientQueue().EndJob();
        std::cout << "Input a number for test (negative value for stopping)" << std::endl;
        std::cin >> stop;
    }
    sp.ShutdownPool();
    sp0.ShutdownPool();
}

bool CALLBACK CVCallback(bool preverified, int depth, int errorCode, const char *errMessage, SPA::CertInfo * ci) {
    return true;
}