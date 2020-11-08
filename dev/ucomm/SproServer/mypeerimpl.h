#pragma once

#include "../include/aserverw.h"
#include "../utest.h"
//#include <boost/thread.hpp>
extern SPA::CUCriticalSection g_mutex;

class CMyPeer : public SPA::ServerSide::CClientPeer {
    unsigned int m_dbIndex;
    unsigned int m_nCalls;

public:
    static SPA::ServerSide::CServerQueue m_mq;

protected:

    void OnSwitchFrom(unsigned int nOldServiceId) {
        unsigned int groups[] = {1, 2, 7};
        GetPush().Subscribe(groups, 3);

        SPA::UVariant vt = L"This is a test message";

        GetPush().Publish(vt, groups, 3);

        SetZip(true);
        SetZipLevel(SPA::tagZipLevel::zlBestSpeed);

        m_dbIndex = 0;
        m_nCalls = 0;

        /*SPA::CAutoLock sl(g_mutex);
        std::wcout << L"OnSwitchFrom, sender = " << GetUID() << L", old service id = " << nOldServiceId << std::endl;*/
    }

    void OnSubscribe(const unsigned int *pGroup, unsigned int count) {
        //SPA::CAutoLock sl(g_mutex);
        //		std::wcout << "OnSubscribe, sender = " << GetUID() << ", group count = " << count << std::endl;
    }

    void OnUnsubscribe(const unsigned int *pGroup, unsigned int count) {
        //SPA::CAutoLock sl(g_mutex);
        //		std::wcout << "OnUnsubscribe, sender = " << GetUID() << ", group count = " << count << std::endl;
    }

    void OnPublishEx(const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size) {
        /*		const char *msg = (const char*) pMessage;
                std::string strMsg(msg, msg + size);
                SPA::CAutoLock sl(g_mutex);
                std::wcout << L"OnBroadcast, sender = " << GetUID() << L", group count = " << count << L", msg = ";
                std::cout << strMsg << std::endl;*/
        SPA::ServerSide::CSocketProServer::PushManager::Publish(pMessage, size, pGroup, count);
    }

    void OnPublish(const SPA::UVariant& vtMessage, const unsigned int *pGroup, unsigned int count) {
        /*		SPA::CAutoLock sl(g_mutex);
                std::wcout << L"OnBroadcast, sender = " << GetUID() << L", group count = " << count << L", msg = ";
                std::cout << std::endl;*/
        SPA::ServerSide::CSocketProServer::PushManager::Publish(vtMessage, pGroup, count);
    }

    void OnSendUserMessage(const wchar_t* receiver, const SPA::UVariant& vtMessage) {
        /*		SPA::CAutoLock sl(g_mutex);
                std::wcout << L"OnPostUserMessage, sender = " << GetUID() << L", receiver = " << receiver << L", msg = ";
                std::cout << std::endl;*/
        SPA::ServerSide::CSocketProServer::PushManager::SendUserMessage(vtMessage, receiver);
    }

    void OnSendUserMessageEx(const wchar_t* receiver, const unsigned char *pMessage, unsigned int size) {
        /*		const char *msg = (const char*) pMessage;
                std::string strMsg(msg, msg + size);
                SPA::CAutoLock sl(g_mutex);
                std::wcout << L"OnPostUserMessageEx, sender = " << GetUID() << L", receiver = " << receiver << L", msg = ";
                std::cout << strMsg << std::endl;*/
        SPA::ServerSide::CSocketProServer::PushManager::SendUserMessage(receiver, pMessage, size);
    }

    void OnChatRequestCame(SPA::tagChatRequestID chatRequestId) {
        /*		SPA::CAutoLock sl(g_mutex);
                std::wcout << L"OnChatRequestCame, sender = " << GetUID() << L", chat request id = " << chatRequestId << std::endl;*/
    }

    void OnRequestArrive(unsigned short requestId, unsigned int len) {

    }

    void OnSlowRequestProcessed(unsigned short requestId) {

    }

    void OnBaseRequestArrive(unsigned short requestId) {
        switch (requestId) {
			case (unsigned short)SPA::tagBaseRequestID::idStartJob:
                break;
            case (unsigned short)SPA::tagBaseRequestID::idEndJob:
                break;
            default:
                break;
        }
    }

protected:

    void OnFastRequestArrive(unsigned short requestId, unsigned int len) {
        BEGIN_SWITCH(requestId)
        M_I2_R1(idBadRequest, BadRequest, unsigned int, std::wstring, std::wstring)
        M_I1_R1(idEcho, Echo, std::string, std::string)
        M_I8_R1(idDoRequest0, DoRequest0, char, wchar_t, std::string, std::wstring, unsigned short, double, bool, SPA::UDateTime, SPA::CScopeUQueue)
        M_I1_R1(idOpenDb, OpenDb, std::string, bool)
        END_SWITCH
    }

    int OnSlowRequestArrive(unsigned short requestId, unsigned int len) {
        BEGIN_SWITCH(requestId)
        M_I1_R0(idSleep, Sleep, unsigned int)
        M_I1_R1(idOpenDb, OpenDb, std::string, bool)
        M_I1_R0(idDequeue, Dodequeue, unsigned int)
        END_SWITCH
        return 0;
    }

private:

    void Dodequeue(unsigned int messageCount) {
        SPA::UINT64 res = Dequeue(m_mq.GetHandle(), messageCount, true);
        unsigned int bytes = (unsigned int) (res >> 32);
        unsigned int dequedMessages = (unsigned int) (res & 0xFFFFFFFF);
        //SPA::CAutoLock sl(g_mutex);
        //std::cout << "Bytes = " << bytes << ", dequeued messages = " << dequedMessages << std::endl;
    }

    void DoRequest0(char aChar, wchar_t aWChar, const std::string &str, const std::wstring &wstr, unsigned short us, double d, bool b, const SPA::UDateTime dt, SPA::CScopeUQueue &q) {
        assert(aWChar == L'?');
        assert(wstr.find(L"???????90?????") != std::wstring::npos);
        q << aChar << aWChar << str << wstr << us << d << b << dt;
        SPA::UINT64 res = m_mq.Enqueue(2346, aChar, aWChar, str, wstr, us, d, b, dt);
    }

    void BadRequest(int n, const std::wstring &input, /*out*/std::wstring &res) {
        unsigned int port = 0;
        res = input;
        SPA::UINT64 ret = m_mq.Enqueue(idDoRequest1, input, res);
        /*SPA::CAutoLock sl(g_mutex);
        size_t pos = input.find(L"???????????????2011?2?15?");
        std::wcout << L"Position = " << pos << L"\r\n";
        std::string addr = GetPeerName(&port);
        addr = GetPeerName(nullptr);*/
    }

    void Sleep(unsigned int ms) {
        //boost::this_thread::sleep(boost::posix_time::milliseconds(25000));
        //boost::this_thread::sleep(boost::posix_time::milliseconds(ms / 2));
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
    }

    void OpenDb(const std::string &connString, /*out*/bool &ok) {
        ok = true;
        ++m_dbIndex;
        /*
                SPA::CAutoLock sl(g_mutex);
                if ((m_dbIndex % 100) == 0)
                    std::cout << "db index = " << m_dbIndex << std::endl;
         */
    }

    void Echo(std::string &input, /*out*/std::string &output) {
        output.swap(input);
        ++m_nCalls;

        //bool has = SPA::ServerSide::CSocketProServer::CredentialManager::HasUserId(L"socketPro");
        //has = SPA::ServerSide::CSocketProServer::CredentialManager::HasUserId(L"Win_SOcketPro");
        //has = SPA::ServerSide::CSocketProServer::CredentialManager::HasUserId(L"nix_SocketPro");
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
    }
};

