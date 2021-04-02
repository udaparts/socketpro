#ifndef _UMB_SERVER_THREAD_H__
#define _UMB_SERVER_THREAD_H__

#include "../core_shared/shared/uthread.h"
#include "../include/userver.h"

class CServer;
class CServerSession;

struct U_MODULE_HIDDEN CUThreadMessage {
    unsigned int m_nMsgId;
    SPA::CUQueue *m_pMessageBuffer;
    unsigned short m_uRequestId;

    CUThreadMessage() : m_nMsgId(0), m_pMessageBuffer(nullptr), m_uRequestId(0) {
    }

    CUThreadMessage(unsigned int nMsgId, SPA::CUQueue *pUQueue, unsigned short reqId)
    : m_nMsgId(nMsgId), m_pMessageBuffer(pUQueue), m_uRequestId(reqId) {
    }

#if 0
    CUThreadMessage(const CUThreadMessage &msg) = delete;
    CUThreadMessage &operator=(const CUThreadMessage &msg) = delete;
#else

    CUThreadMessage(const CUThreadMessage &msg)
    : m_nMsgId(msg.m_nMsgId), m_pMessageBuffer(msg.m_pMessageBuffer), m_uRequestId(msg.m_uRequestId) {
    }

    CUThreadMessage &operator=(const CUThreadMessage &msg) {
        if (this == &msg) {
            return *this;
        }
        m_nMsgId = msg.m_nMsgId;
        m_pMessageBuffer = msg.m_pMessageBuffer;
        m_uRequestId = msg.m_uRequestId;
        return *this;
    }
#endif

    CUThreadMessage(CUThreadMessage &&msg)
    : m_nMsgId(msg.m_nMsgId), m_pMessageBuffer(msg.m_pMessageBuffer), m_uRequestId(msg.m_uRequestId) {
        msg.m_pMessageBuffer = nullptr;
    }

    CUThreadMessage &operator=(CUThreadMessage &&msg) {
        if (this == &msg) {
            return *this;
        }
        m_nMsgId = msg.m_nMsgId;
        m_pMessageBuffer = msg.m_pMessageBuffer;
        m_uRequestId = msg.m_uRequestId;
        msg.m_pMessageBuffer = nullptr;
        return *this;
    }
};

class U_MODULE_HIDDEN CServerThread : public SPA::CUCommThread {
public:
    CServerThread(unsigned int nMaxThreadIdleTimeBeforeSuicide, SPA::tagThreadApartment ta);
    ~CServerThread();

public:
    bool IsAliveSafe();
    bool IsBusy();
    bool PostMessage(CServerSession *pSession, unsigned short uRequestId, unsigned int nMsgId, const void *pBuffer, unsigned int nSize);

protected:
    virtual void OnThreadStarted();
    virtual void OnThreadEnded();

private:
    void Handle();
    unsigned int ProcessSlowRequest(CServerSession *pSession, unsigned short reqId);

private:
    bool m_bBusy;
    unsigned int m_nMaxThreadIdleTimeBeforeSuicide;
    std::atomic<SPA::UINT64> m_tWorking;
    SPA::CSpinLock m_sl;
    std::queue<CUThreadMessage> m_qThreadMessage;
};

typedef CServerThread* PServerThread;

#endif
