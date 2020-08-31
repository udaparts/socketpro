
#pragma once

#include "../include/aclientw.h"
#include "../utest.h"

extern SPA::CUCriticalSection g_mutex;

class CMySocket : public SPA::ClientSide::CClientSocket {
protected:

    virtual void OnSocketClosed(int error) {
        //SPA::CAutoLock sl(g_mutex);
        //std::cout << "Socket closed with errCode = " << error << std::endl;
    }

    virtual void OnHandShakeCompleted(int error) {
        //SPA::CAutoLock sl(g_mutex);
        //std::cout << "SSL handshake completed with errCode = " << error << std::endl;
    }

    virtual void OnSocketConnected(int error) {
        //SPA::CAutoLock sl(g_mutex);
        //std::cout << "Socket connected with errCode = " << error << std::endl;
    }

    virtual void OnRequestProcessed(unsigned short requestId, unsigned int len) {
        //SPA::CAutoLock sl(g_mutex);
        //std::cout << "Request " << requestId << " came with data len = " << len << std::endl;
    }

    virtual void OnSubscribe(const SPA::ClientSide::CMessageSender& sender, const unsigned int *group, unsigned int count) {
        /*SPA::CAutoLock sl(g_mutex);
        std::cout << "OnSubscribe called with group count = " << count << ", self = " << sender.SelfMessage << ", sender = ";
        std::wcout << sender.UserId;
        std::cout << ", ip addr = " << sender.IpAddress << ", port = " << sender.Port << std::endl;*/
    }

    virtual void OnUnsubscribe(const SPA::ClientSide::CMessageSender& sender, const unsigned int *group, unsigned int count) {
        /*SPA::CAutoLock sl(g_mutex);
        std::cout << "OnUnsubscribe called with group count = " << count << ", self = " << sender.SelfMessage << ", sender = ";
        std::wcout << sender.UserId;
        std::cout << ", ip addr = " << sender.IpAddress << ", port = " << sender.Port << std::endl;*/
    }

    virtual void OnPublishEx(const SPA::ClientSide::CMessageSender& sender, const unsigned int *group, unsigned int count, const unsigned char *message, unsigned int size) {
        /*SPA::CAutoLock sl(g_mutex);
        std::cout << "OnBroadcastEx called with count = " << count << ", self = " << sender.SelfMessage << ", sender = ";
        std::wcout << sender.UserId;
        std::cout << ", ip addr = " << sender.IpAddress << ", port = " << sender.Port;
        std::string str((const char*) message, (const char*) (message + size));
        std::cout << ", message = " << str << std::endl;*/
    }

    virtual void OnPublish(const SPA::ClientSide::CMessageSender& sender, const unsigned int *group, unsigned int count, const SPA::UVariant &vtMsg) {
        /*SPA::CAutoLock sl(g_mutex);
        std::cout << "OnBroadcast called with count = " << count << ", self = " << sender.SelfMessage << ", sender = ";
        std::wcout << sender.UserId;
        std::cout << ", ip addr = " << sender.IpAddress << ", port = " << sender.Port << std::endl;*/
    }

    virtual void OnSendUserMessageEx(const SPA::ClientSide::CMessageSender& sender, const unsigned char *message, unsigned int size) {
        /*SPA::CAutoLock sl(g_mutex);
        std::cout << "OnPostUserMessageEx called with self = " << sender.SelfMessage << ", sender = ";
        std::wcout << sender.UserId;
        std::cout << ", ip addr = " << sender.IpAddress << ", port = " << sender.Port;
        std::string str((const char*) message, (const char*) (message + size));
        std::cout << ", message = " << str << std::endl;*/
    }

    virtual void OnSendUserMessage(const SPA::ClientSide::CMessageSender& sender, const SPA::UVariant &vtMsg) {
        /*SPA::CAutoLock sl(g_mutex);
        std::cout << "OnPostUserMessage called with self = " << sender.SelfMessage << ", sender = ";
        std::wcout << sender.UserId;
        std::cout << ", ip addr = " << sender.IpAddress << ", port = " << sender.Port << std::endl;*/
    }

    virtual void OnBaseRequestProcessed(unsigned short requestId) {
        switch (requestId) {
            case SPA::idStartJob:
                std::cout << "Start server dequeue trans" << std::endl;
                break;
            case SPA::idEndJob:
                std::cout << "Commit server dequeue trans" << std::endl;
                break;
            default:
                break;
        }
    }
};

class CMyServiceHandler : public SPA::ClientSide::CAsyncServiceHandler {
public:

    CMyServiceHandler(SPA::ClientSide::CClientSocket *cs)
    : SPA::ClientSide::CAsyncServiceHandler(sidTestService, cs) {
    }

public:

    void Sleep(unsigned int time) {
        async0(idSleep, time).get();
    }

    void DodequeueAsync(unsigned int messageCount) {
        bool ok = SendRequest(idDequeue, SPA::ClientSide::NULL_RH, messageCount);
    }

    std::wstring BadRequest(unsigned int n, const wchar_t* input) {
		return async<std::wstring>(idBadRequest, n, input).get();
    }

    bool OpenDb(const char *connString) {
        return async<bool>(idOpenDb, connString).get();
    }

    bool OpenDbAsync(const char *connString) {
        return SendRequest(idOpenDb, SPA::ClientSide::NULL_RH, connString);
    }

    std::string Echo(const char *input) {
		return async<std::string>(idEcho, input).get();
    }

    SPA::CUQueue DoRequest0(char aChar, wchar_t aWChar, const char *str, const wchar_t *wstr, unsigned short us, double d, bool b, SPA::UDateTime dt) {
        return async<SPA::CUQueue>(idDoRequest0, aChar, aWChar, str, wstr, us, d, b, dt).get();
    }

private:
    bool m_ok;
    SPA::CUQueue m_q;
    std::string m_out;
    std::wstring m_wout;

protected:

    void OnResultReturned(unsigned short reqId, SPA::CUQueue &mc) {
        switch (reqId) {
            case idEcho:
                mc >> m_out;
                break;
            case idOpenDb:
                mc >> m_ok;
                break;
            case idBadRequest:
                mc >> m_wout;
                break;
            case idDoRequest0:
                m_q.Push(mc.GetBuffer(), mc.GetSize());
                mc.SetSize(0);
                break;
            case idDoRequest1:
            {
                std::wstring input;
                std::wstring res;
                mc >> input >> res;
                //std::wcout << L" +++ " << res << L" ++++" << std::endl;
                assert(input == res);
            }
                break;
            case idDoRequest2:
                break;
            case idDoRequest3:
                break;
            case idDoRequest4:
                break;
            case idDequeue:
                break;
            case idDoIdle:
            {
                int n;
                SPA::UINT64 ms;
                std::string s;
                mc >> ms >> n >> s;
                std::cout << "ms = " << ms << ", n = " << n << ", name = " << s << std::endl;
            }
                break;
            default:
                break;
        }
    }
};
