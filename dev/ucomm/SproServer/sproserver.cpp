// SproServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "httppeer.h"
#include "mypeerimpl.h"
#include "TEchoBImpl.h"
#include "TEchoCImpl.h"
#include "TEchoDImpl.h"
#include "TOneImpl.h"
#include "../include/scloader.h"

SPA::CUCriticalSection g_mutex;

class CMyServer : public SPA::ServerSide::CSocketProServer {
public:

    CMyServer(int nParam = 0) : SPA::ServerSide::CSocketProServer(nParam) {

    }

    void SetChatGroups() {
        PushManager::AddAChatGroup(1, L"GroupOne");
        PushManager::AddAChatGroup(2, L"R&D Department");
        PushManager::AddAChatGroup(7, L"HR Department");
        PushManager::AddAChatGroup(16, L"Management Department");
    }

    bool RegisterServices() {
        bool ok = true;
        do {
            ok = m_myTOne.AddMe(sidCTOne);
            if (!ok)
                break;

            ok = m_Route0.AddMe(sidRouteSvs0);
            if (!ok)
                break;

            ok = m_Route1.AddMe(sidRouteSvs1);
            if (!ok)
                break;

            ok = m_CEchoBasic.AddMe(sidCEchoBasic, taNone);
            if (!ok)
                break;

            ok = m_CEchoObject.AddMe(sidCEchoObject);
            if (!ok)
                break;

            ok = m_CEchoSys.AddMe(sidCEchoSys);
            if (!ok)
                break;

            ok = m_myService.AddMe(sidTestService);
            if (!ok)
                break;

            ok = m_myTOne.AddSlowRequest(idSleepCTOne);
            if (!ok)
                break;

            ok = m_myService.AddSlowRequest(idSleep);
            if (!ok)
                break;
            ok = m_myService.AddSlowRequest(idOpenDb);
            if (!ok)
                break;
            ok = m_myService.AddSlowRequest(idDequeue);
            if (!ok)
                break;
            ok = m_myHttp.AddMe(SPA::sidHTTP);
            if (!ok)
                break;
            ok = m_myHttp.AddSlowRequest(SPA::ServerSide::idGet);
            if (!ok)
                break;
            ok = m_myHttp.AddSlowRequest(SPA::ServerSide::idPost);
            if (!ok)
                break;
            ok = m_myHttp.AddSlowRequest(SPA::ServerSide::idMultiPart);
            if (!ok)
                break;
            ok = m_myHttp.AddSlowRequest(SPA::ServerSide::idUserRequest);
            if (!ok)
                break;

            ok = m_Push.AddMe(SPA::sidChat);
            if (!ok)
                break;
        } while (false);
        if (!ok) {
            m_myTOne.RemoveMe();
            m_Route0.RemoveMe();
            m_Route1.RemoveMe();
            m_myService.RemoveMe();
            m_Push.RemoveMe();
            m_myHttp.RemoveMe();
        } else {
            ok = Router::SetRouting(sidRouteSvs0, sidRouteSvs1);
        }
        return ok;
    }

protected:

    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        return true;
    }

    void OnAccept(USocket_Server_Handle h, int errCode) {

        /*		
         * SPA::CAutoLock sl(g_mutex);
        std::cout << "accepted with error code = " << errCode << std::endl;
         */
    }

    void OnSSLShakeCompleted(USocket_Server_Handle h, int errCode) {
        /*       
        if (errCode != 0) {
            SPA::CAutoLock al(g_mutex);
            std::cout << "Handshake error = " << GetErrorMessage() << std::endl;
         }
         */
    }

    void OnIdle(SPA::INT64 milliseconds) {
        bool ok;
        if ((milliseconds % 7) == 4) {
            int count = (std::rand() % 6);
            if (!count)
                return;
            ok = CMyPeer::m_mq.StartJob();
            for (int n = 0; n < count; ++n) {
                SPA::UINT64 ret = CMyPeer::m_mq.Enqueue(idDoIdle, milliseconds, n, "MyTestMessage");
            }
            ok = CMyPeer::m_mq.EndJob();
        }
        SPA::CAutoLock al(g_mutex);
        std::cout << "OnIdle = " << milliseconds << std::endl;
    }

    void OnClose(USocket_Server_Handle h, int errCode) {
        if (errCode) {
            char errMsg[1025];
            SPA::ServerSide::ServerCoreLoader.GetServerSocketErrorMessage(h, errMsg, sizeof (errMsg));
            SPA::CAutoLock sl(g_mutex);
            std::cout << "Closed, User Id = ";
            std::wcout << SPA::ServerSide::CSocketProServer::CredentialManager::GetUserID(h);
            std::cout << ", errCode = " << errCode << ", errMsg = ";
            std::cout << errMsg << ", count = " << GetCountOfClients() << ", service id = " << SPA::ServerSide::ServerCoreLoader.GetSvsID(h) << std::endl;
        }
    }

    bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
        /*		
        SPA::CAutoLock sl(g_mutex);
        std::cout << "OnIsPermitted, User Id = ";
        std::wcout << userId;
        std::cout << ", Password = ";
        std::wcout << password << std::endl;
         */
        return true;
    }

private:
    SPA::ServerSide::CSocketProService<CTOnePeer> m_myTOne;
    SPA::ServerSide::CSocketProService<CMyPeer> m_myService;
    SPA::ServerSide::CNotificationService m_Push;
    SPA::ServerSide::CSocketProService<CHttpPeer> m_myHttp;
    SPA::ServerSide::CSocketProService<CEchoBasicPeer> m_CEchoBasic;
    CSocketProService<CEchoObjectPeer> m_CEchoObject;
    CSocketProService<CEchoSysPeer> m_CEchoSys;
    SPA::ServerSide::CNotificationService m_Route0;
    SPA::ServerSide::CNotificationService m_Route1;
};

SPA::ServerSide::CServerQueue CMyPeer::m_mq(0);

void send(int *to, int * from, int count) {
    int n = (count + 7) / 8;
    switch (count % 8) {
        case 0:
            do {
                *to++ = *from++;
                case 7: *to++ = *from++;
                case 6: *to++ = *from++;
                case 5: *to++ = *from++;
                case 4: *to++ = *from++;
                case 3: *to++ = *from++;
                case 2: *to++ = *from++;
                case 1: *to++ = *from++;
            } while (--n > 0);
    }
}

int main(int argc, char* argv[]) {
    int param;
    VARIANT *p = nullptr;
    SAFEARRAYBOUND sab[1] = {2, 0};
    SAFEARRAY *sa = SafeArrayCreate(VT_VARIANT, 1, sab);
    CComVariant vt;
    vt.vt = (VT_VARIANT | VT_ARRAY);
    vt.parray = sa;
    SafeArrayAccessData(sa, (void**) &p);
    p[0].vt = VT_I4;
    p[0].lVal = 123456;
    p[1].vt = VT_BSTR;
    p[1].bstrVal = SysAllocString(L"MyTest");
    SafeArrayUnaccessData(sa);
    CComVariant vtNew(vt);
    SafeArrayAccessData(sa, (void**) &p);
    p[0].vt = VT_INT;
    SafeArrayUnaccessData(sa);
    CComVariant vtCopy;
    vtCopy = vtNew;
    bool equal = SPA::IsEqual(vtNew, vt);
    assert(equal);
    equal = SPA::IsEqual(vtNew, vtCopy);
    assert(equal);
    std::cout << "Input a parameter for the number of threads ......" << std::endl;
    std::cin >> param;
    if (param < 0)
        param = 0;
    do {
        CMyServer myServer(param);
        bool crash = CSocketProServer::QueueManager::IsServerQueueIndexPossiblyCrashed();
        //#ifdef WIN32_64
        //        CSocketProServer::QueueManager::SetWorkDirectory("c:\\cyetest\\");
        //#else
        //        CSocketProServer::QueueManager::SetWorkDirectory("/home/yye/cyetest/");
        //#endif
        const char *strWorkPath = CSocketProServer::QueueManager::GetWorkDirectory();
#ifdef WIN32_64
        //myServer.UseSSL("intermediate.pfx", "", "mypassword");
        myServer.UseSSL("root", "UDAParts Intermediate CA", "");
        HINSTANCE h = CSocketProServer::DllManager::AddALibrary("HelloWorld.dll");
#else
        myServer.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
        HINSTANCE h = CSocketProServer::DllManager::AddALibrary("libHelloWorld.so");
#endif
        if (h) {
            CSocketProServer::DllManager::RemoveALibrary(h);
        }
        if (!myServer.RegisterServices())
            break;
        CMyServer::Config::SetMaxConnectionsPerClient(100000);
        myServer.SetChatGroups();
        if (!myServer.Run(20901, 16, true)) {
            break;
        }
        unsigned int groups[2] = {1, 2};
        UVariant vtMsg(L"mytest");

        bool ok = CSocketProServer::PushManager::Publish(vtMsg, groups, 2);
        ok = CSocketProServer::PushManager::SendUserMessage(vtMsg, L"cye");

        crash = CSocketProServer::QueueManager::IsServerQueueIndexPossiblyCrashed();
        //CMyServer::QueueManager::SetMessageQueuePassword("MyPassword");
        SPA::ServerSide::CServerQueue q = SPA::ServerSide::CSocketProServer::QueueManager::StartQueue("temp_queue", 30 * 24 * 3600, false);
        ok = q.StartJob();
        SPA::UINT64 ret = q.Enqueue(idDoIdle, (SPA::INT64)0, (int) 54321, "Message from temp queue 0");
        ret = q.Enqueue(idDoIdle, (SPA::INT64)12345, (int) 234, "Temp queue message 1");
        ok = q.EndJob();
        ret = q.Enqueue(idDoIdle, (SPA::INT64)1234578, (int) 1234, "Temp me queue message 2");
        ret = q.Enqueue(idDoIdle, (SPA::INT64)23412345, (int) 2342, "Temp queue message again 3");
        /*
        h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("uodbsvr.dll");
        ok = SPA::ServerSide::CSocketProServer::DllManager::RemoveALibrary(h);
         */
#ifdef WIN32_64		
        CMyPeer::m_mq = SPA::ServerSide::CSocketProServer::QueueManager::StartQueue("MyQueue_Win", 30 * 24 * 3600);
        auto again = SPA::ServerSide::CSocketProServer::QueueManager::StartQueue("MyQueue_Win", 30 * 24 * 3600);
#else
        CMyPeer::m_mq = SPA::ServerSide::CSocketProServer::QueueManager::StartQueue("MyQueue_Nix", 30 * 24 * 3600);
        auto again = SPA::ServerSide::CSocketProServer::QueueManager::StartQueue("MyQueue_Nix", 30 * 24 * 3600);
#endif
        assert(again.GetHandle() == CMyPeer::m_mq.GetHandle());
        SPA::UINT64 count = CMyPeer::m_mq.GetMessageCount();
        q.AppendTo(CMyPeer::m_mq);
        unsigned int dequeues = CMyPeer::m_mq.GetMessagesInDequeuing();
        SPA::UINT64 queueSize = CMyPeer::m_mq.GetQueueSize();
        {
            SPA::CAutoLock al(g_mutex);
            std::cout << "Queue file size = " << queueSize << ", queue message count = " << CMyPeer::m_mq.GetMessageCount() << std::endl;
            std::cout << "Press any key to stop the server ......" << std::endl;
        }
        ::getchar();
        ::getchar();
        myServer.StopSocketProServer();
        bool opened = myServer.IsRunning();
        opened = false;
    } while (false);
    return 0;
}

