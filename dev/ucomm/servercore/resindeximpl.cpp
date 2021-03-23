
#include "resindeximpl.h"
#include "../core_shared/shared/streamhead.h"
#ifdef WIN32_64
#include "../uservercore_win/server.h"
#else
#include "../uservercore/server.h"
#endif
#include "../core_shared/shared/includes.h"

extern CServer *g_pServer;

CResIndexImpl::CResIndexImpl(unsigned short reqId, void *session)
: m_uReqId(reqId),
m_pSession(session),
m_bHead(false),
m_pQ(SPA::CScopeUQueue::Lock()) {
    assert(session);
}

CResIndexImpl::CResIndexImpl(unsigned short reqId, void *session, const MQ_FILE::QAttr &qa)
: m_uReqId(reqId),
m_pSession(session),
m_qa(qa),
m_bHead(false),
m_pQ(SPA::CScopeUQueue::Lock()) {
    assert(session);
    assert(m_pQ);
}

CResIndexImpl::~CResIndexImpl() {
    for (auto it = m_deq.begin(), end = m_deq.end(); it != end; ++it) {
        SPA::CScopeUQueue::Unlock(*it);
    }
    m_deq.clear();
    SPA::CScopeUQueue::Unlock(m_pQ);
}

bool CResIndexImpl::IsAllCollected(bool *partial) const {
    if (partial)
        *partial = (m_deq.size() > 0);
    if (!m_pQ)
        return false;
    return (m_pQ->GetSize() || m_uReqId == (unsigned short) SPA::tagBaseRequestID::idStartJob || m_uReqId == (unsigned short) SPA::tagBaseRequestID::idEndJob);
}

unsigned int CResIndexImpl::SendReturnData(unsigned short usReqId, const unsigned char *pBuffer, unsigned int size) {
    CServerSession *session = (CServerSession*) m_pSession;
    if (pBuffer == nullptr)
        size = 0;
    if (session->m_cs < csConnected || g_pServer->m_bStopped)
        return SOCKET_NOT_FOUND;
    if (usReqId != (unsigned short) SPA::tagBaseRequestID::idCancel && session->m_bCanceled)
        return REQUEST_CANCELED;
    SPA::CUQueue *q;
    if (usReqId == (unsigned short) SPA::tagBaseRequestID::idServerException)
        q = m_pQ;
    else if (usReqId == m_uReqId)
        q = m_pQ;
    else {
        q = SPA::CScopeUQueue::Lock(MY_OPERATION_SYSTEM, SPA::IsBigEndian(), size + sizeof (SPA::CStreamHeader));
        m_deq.push_back(q);
    }
    SPA::CStreamHeader sh;
    if (session->m_bZip)
        session->CompressResultTo(session->IsOld(), usReqId, session->m_zl, pBuffer, size, *q);
    else {
        sh.RequestId = usReqId;
        sh.Size = size;
        *q << sh;
        q->Push(pBuffer, size);
    }
    if ((usReqId == m_uReqId || usReqId == (unsigned short) SPA::tagBaseRequestID::idServerException) && m_qa.IsValid()) {
        sh.RequestId = (unsigned short) SPA::tagBaseRequestID::idDequeueConfirmed;
#ifndef NDEBUG
        if (usReqId == (unsigned short) SPA::tagBaseRequestID::idServerException) {
            unsigned short *p = (unsigned short*) pBuffer;
            if (*p != m_uReqId) {
                assert(false);
            }
        }
#endif
        MQ_FILE::CDequeueConfirmInfo dci(m_qa, session->m_bFail, m_uReqId);
        sh.Size = sizeof (dci);
        *q << sh << dci;
    }
    return size;
}

SPA::UINT64 CResIndexImpl::SendPartial() {
    SPA::UINT64 res = 0;
    CServerSession *session = (CServerSession*) m_pSession;
    while (m_deq.size()) {
        auto it = m_deq.begin();
        SPA::CUQueue *q = *it;
        if (q && q->GetSize()) {
            session->Write(q->GetBuffer(), q->GetSize());
            res += q->GetSize();
            q->SetSize(0);
        }
        SPA::CScopeUQueue::Unlock(q);
        m_deq.erase(it);
        if (session->m_qWrite.GetSize() > 100 * IO_BUFFER_SIZE)
            return res;
    }
    return res;
}

SPA::UINT64 CResIndexImpl::SendAll() {
    SPA::UINT64 res = 0;
    CServerSession *session = (CServerSession*) m_pSession;
    while (m_deq.size()) {
        auto it = m_deq.begin();
        SPA::CUQueue *q = *it;
        if (q && q->GetSize()) {
            session->Write(q->GetBuffer(), q->GetSize());
            res += q->GetSize();
            q->SetSize(0);
        }
        SPA::CScopeUQueue::Unlock(q);
        m_deq.erase(it);
        if (session->m_qWrite.GetSize() > 100 * IO_BUFFER_SIZE)
            return res;
    }
    if (m_pQ) {
        if (m_uReqId == (unsigned short) SPA::tagBaseRequestID::idStartJob) {
#ifndef NDEBUG
            ++session->m_nJobRequest;
            if (session->m_bDequeueTrans) {
                std::cout << "Bad m_bDequeueTrans/(unsigned short)SPA::tagBaseRequestID::idStartJob: " << session->m_bDequeueTrans << ", " << __FUNCTION__ << " @line: " << __LINE__ << std::endl;
            }
#endif
            session->m_bDequeueTrans = true;
            assert(m_qa.IsValid());
            MQ_FILE::CDequeueConfirmInfo dci(m_qa, session->m_bFail, m_uReqId);
			SPA::CStreamHeader sh((unsigned short)SPA::tagBaseRequestID::idDequeueConfirmed, sizeof(dci));
            *m_pQ << sh << dci;
            POnBaseRequestCame p = session->m_ccb.SvsContext.m_OnBaseRequestCame;
            if (p != nullptr) {
                USocket_Server_Handle index = session->MakeHandlerInternal();
                CRAutoLock ral(session->m_mutex, session->m_bChatting);
                p(index, m_uReqId);
            }
        } else if (m_uReqId == (unsigned short) SPA::tagBaseRequestID::idEndJob) {
#ifndef NDEBUG
            --session->m_nJobRequest;
            if (!session->m_bDequeueTrans) {
                std::cout << "Bad m_bDequeueTrans/(unsigned short)SPA::tagBaseRequestID::idEndJob: " << session->m_bDequeueTrans << ", " << __FUNCTION__ << " @line: " << __LINE__ << std::endl;
            }
#endif
            session->m_bDequeueTrans = false;
            assert(m_qa.IsValid());
			MQ_FILE::CDequeueConfirmInfo dci(m_qa, session->m_bFail, m_uReqId);
            SPA::CStreamHeader sh((unsigned short)SPA::tagBaseRequestID::idDequeueConfirmed, sizeof(dci));
            *m_pQ << sh << dci;
            POnBaseRequestCame p = session->m_ccb.SvsContext.m_OnBaseRequestCame;
            if (p != nullptr) {
                USocket_Server_Handle index = session->MakeHandlerInternal();
                CRAutoLock ral(session->m_mutex, session->m_bChatting);
                p(index, m_uReqId);
            }
        }
        if (m_pQ->GetSize()) {
            session->Write(m_pQ->GetBuffer(), m_pQ->GetSize());
            res += m_pQ->GetSize();
            m_pQ->SetSize(0);
        }
        SPA::CScopeUQueue::Unlock(m_pQ);
        m_pQ = nullptr;
    }
    return res;
}

#define TO_MANY_QUEUED ((unsigned int)64)

bool IsTooMany(const CMapIndex &mi) {
    if (mi.size() < TO_MANY_QUEUED)
        return false;
    size_t head = 0;
    int balance = 0;
    bool sb = false;
    for (auto it = mi.cbegin(), end = mi.cend(); it != end; ++it) {
        if (!balance) {
            ++head;
            if (head > TO_MANY_QUEUED)
                return true;
        }
        unsigned short id = it->second->GetReqId();
        if (id == (unsigned short) SPA::tagBaseRequestID::idStartJob) {
            ++balance;
        } else if (id == (unsigned short) SPA::tagBaseRequestID::idEndJob) {
            sb = true;
            --balance;
            if (mi.size() > TO_MANY_QUEUED)
                return true;
        }
        if (sb && mi.size() > 2 * TO_MANY_QUEUED)
            return true;
    }
    return false;
}
