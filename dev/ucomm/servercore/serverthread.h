#ifndef _UMB_SERVER_THREAD_H__
#define _UMB_SERVER_THREAD_H__

#include "../core_shared/shared/uthread.h"
#include "../include/userver.h"

class CServer;
class CServerSession;

class CServerThread : public SPA::CUCommThread {
public:
    CServerThread(unsigned int nMaxThreadIdleTimeBeforeSuicide, SPA::tagThreadApartment ta);

public:
    bool IsAliveSafe();
    bool IsBusy();
    bool PostMessage(CServerSession *pSession, unsigned short uRequestId, unsigned int nMsgId, const void *pBuffer, unsigned int nSize);

protected:
    virtual void OnThreadStarted();
    virtual void OnThreadEnded();

private:
    void Handle();
    unsigned int ProcessSlowRequest(CServerSession *pSession, SPA::CUThreadMessage ThreadMessage);

private:
    bool m_bBusy;
    unsigned int m_nMaxThreadIdleTimeBeforeSuicide;
    boost::posix_time::ptime m_tWorking;
};

typedef CServerThread* PServerThread;

#endif
