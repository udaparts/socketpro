#include "aclientw.h"

#ifdef NODE_JS_ADAPTER_PROJECT

#include <node.h>
#if NODE_VERSION_AT_LEAST(12,13,0)
#define BOOL_ISOLATE
#endif
#include "../src/njadapter/njadapter/njobjects.h"

#endif

namespace SPA {
    namespace ClientSide {

        CUCriticalSection g_csSpPool;

        CSpinLock CClientSocket::m_mutex;
        std::vector<CClientSocket*> CClientSocket::m_vClientSocket;

        Internal::CClientCoreLoader ClientCoreLoader;

        CAsyncServiceHandler::CRR CAsyncServiceHandler::m_rrStack;

        CSpinLock CAsyncServiceHandler::m_csIndex;
        UINT64 CAsyncServiceHandler::m_CallIndex = 0; //should be protected by m_csIndex

        CAsyncServiceHandler::DServerException CAsyncServiceHandler::NULL_SE;
        CAsyncServiceHandler::DDiscarded CAsyncServiceHandler::NULL_ABORTED;

        const wchar_t* const CAsyncServiceHandler::SESSION_CLOSED_BEFORE_ERR_MSG = L"Session already closed before sending the request";
        const wchar_t* const CAsyncServiceHandler::SESSION_CLOSED_AFTER_ERR_MSG = L"Session closed after sending the request";
        const wchar_t* const CAsyncServiceHandler::REQUEST_CANCELED_ERR_MSG = L"Request canceled";

        bool CAsyncServiceHandler::CRRImpl::Invoke(CAsyncServiceHandler *ash, unsigned short reqId, CUQueue & buff) {
            CSpinAutoLock al(*m_cs);
            for (auto it = m_vD.cbegin(), end = m_vD.cend(); it != end; ++it) {
                auto &rr = *it;
                if (rr(ash, reqId, buff)) {
                    return true;
                }
            }
            return false;
        }

        CAsyncServiceHandler::CAsyncServiceHandler(unsigned int nServiceId, CClientSocket * cs)
        : m_vCallback(*m_suCallback), m_vBatching(*m_suBatching), m_nServiceId(nServiceId), m_pClientSocket(nullptr),
        ResultReturned(m_rrImpl), ServerException(m_seImpl), BaseRequestProcessed(m_brpImpl) {
            if (cs) {
                Attach(cs);
            }
            m_vCallback.SetBlockSize(128 * 1024);
            m_vCallback.ReallocBuffer(128 * 1024);
            m_rrImpl.SetCS(&m_csCb);
            m_seImpl.SetCS(&m_csCb);
            m_brpImpl.SetCS(&m_csCb);
#ifdef NODE_JS_ADAPTER_PROJECT
            ::memset(&m_typeReq, 0, sizeof (m_typeReq));
            m_typeReq.data = this;
            int fail = uv_async_init(uv_default_loop(), &m_typeReq, req_cb);
            assert(!fail);
#endif
        }

        CAsyncServiceHandler::~CAsyncServiceHandler() {
            CleanCallbacks();
            m_cs.lock();
            if (m_pClientSocket)
                m_pClientSocket->Detach(this);
            m_cs.unlock();
#ifdef NODE_JS_ADAPTER_PROJECT         
            uv_close((uv_handle_t*) & m_typeReq, nullptr);
#endif
        }

        UINT64 CAsyncServiceHandler::GetCallIndex() noexcept {
            m_csIndex.lock();
            UINT64 index = ++m_CallIndex;
            m_csIndex.unlock();
            return index;
        }

        void CAsyncServiceHandler::ClearResultCallbackPool(unsigned int remaining) {
            m_rrStack.ClearResultCallbackPool(remaining);
        }

        unsigned int CAsyncServiceHandler::CountResultCallbacksInPool() noexcept {
            return (unsigned int) m_rrStack.size();
        }

        void CAsyncServiceHandler::CleanQueue(CUQueue & q) {
            unsigned int size = q.GetSize();
            if (size) {
                PRR_PAIR *pp = (PRR_PAIR*) q.GetBuffer();
                size /= sizeof (PRR_PAIR);
                m_rrStack.push_back(pp, size);
                q.SetSize(0);
            }
        }

        void CAsyncServiceHandler::OnResultReturned(unsigned short reqId, CUQueue & mc) {

        }

        void CAsyncServiceHandler::OnBaseRequestprocessed(unsigned short reqId) {

        }

        void CAsyncServiceHandler::OnAllProcessed() {

        }

        void CAsyncServiceHandler::OnInterrupted(UINT64 options) {

        }

        void CAsyncServiceHandler::OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) {

        }

        void CAsyncServiceHandler::SetSvsID(unsigned int serviceId) noexcept {
            assert(0 == m_nServiceId);
            assert(serviceId);
            m_nServiceId = serviceId;
        }

        bool CAsyncServiceHandler::WaitAll(unsigned int timeOut) {
            USocket_Client_Handle h = GetClientSocketHandle();
            //call the method after calling CommitBatching or AbortBatching
            assert(!ClientCoreLoader.IsBatching(h)); //bad code logic!!!

            //call the method EndJob or AbortJob
            assert(!(ClientCoreLoader.IsQueueStarted(h) && ClientCoreLoader.GetJobSize(h) > 0));
            return ClientCoreLoader.WaitAll(h, timeOut);
        }

        bool CAsyncServiceHandler::Interrupt(UINT64 options) {
            assert(ClientCoreLoader.SendInterruptRequest);
            return ClientCoreLoader.SendInterruptRequest(GetClientSocketHandle(), options);
        }

        bool CAsyncServiceHandler::StartBatching() {
            return ClientCoreLoader.StartBatching(GetClientSocketHandle());
        }

        bool CAsyncServiceHandler::IsDequeuedResult() {
            return ClientCoreLoader.DequeuedResult(GetClientSocketHandle());
        }

        void CAsyncServiceHandler::AbortDequeuedMessage() {
            ClientCoreLoader.AbortDequeuedMessage(GetClientSocketHandle());
        }

        bool CAsyncServiceHandler::IsDequeuedMessageAborted() {
            return ClientCoreLoader.IsDequeuedMessageAborted(GetClientSocketHandle());
        }

        bool CAsyncServiceHandler::IsRouteeRequest() {
            return ClientCoreLoader.IsRouteeRequest(GetClientSocketHandle());
        }

        bool CAsyncServiceHandler::SendRouteeResult(const unsigned char *buffer, unsigned int len, unsigned short reqId) {
            if (reqId == 0) {
                if (m_pClientSocket)
                    reqId = m_pClientSocket->GetCurrentRequestID();
            }
            return ClientCoreLoader.SendRouteeResult(GetClientSocketHandle(), reqId, buffer, len);
        }

        bool CAsyncServiceHandler::SendRouteeResult(unsigned short reqId) {
            return SendRouteeResult((const unsigned char *) nullptr, (unsigned int) 0, reqId);
        }

        void CAsyncServiceHandler::Detach() {
            Attach(nullptr);
        }

        bool CAsyncServiceHandler::Attach(CClientSocket * cs) {
            CSpinAutoLock al(m_cs);
            if (cs && m_pClientSocket == cs) {
                return true;
            }
            if (m_pClientSocket)
                m_pClientSocket->Detach(this);
            if (cs) {
                if (cs->Attach(this)) {
                    m_pClientSocket = cs;
                    return true;
                }
            } else {
                return true;
            }
            return false;
        }

        bool CAsyncServiceHandler::CommitBatching(bool bBatchingAtServerSide) {
            m_cs.lock();
            m_vCallback.Push(m_vBatching.GetBuffer(), m_vBatching.GetSize());
            m_vBatching.SetSize(0);
            m_cs.unlock();
            return ClientCoreLoader.CommitBatching(GetClientSocketHandle(), bBatchingAtServerSide);
        }

        bool CAsyncServiceHandler::AbortBatching() {
            m_cs.lock();
            CleanQueue(m_vBatching);
            m_cs.unlock();
            return ClientCoreLoader.AbortBatching(GetClientSocketHandle());
        }

        bool CAsyncServiceHandler::IsBatching() {
            return ClientCoreLoader.IsBatching(GetClientSocketHandle());
        }

        bool CAsyncServiceHandler::Remove(CUQueue &q, PRR_PAIR p) noexcept {
            int count = (int) (q.GetSize() / sizeof (PRR_PAIR));
            PRR_PAIR *pp = (PRR_PAIR*) q.GetBuffer();
            for (int n = count - 1; n >= 0; --n) {
                PRR_PAIR it = pp[n];
                if (it == p) {
                    q.Pop((unsigned int) sizeof (PRR_PAIR), (unsigned int) (n * sizeof (PRR_PAIR)));
                    return true;
                }
            }
            return false;
        }

        bool CAsyncServiceHandler::SendRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int size, const DResultHandler& rh, const DDiscarded& discarded, const DServerException & serverException) {
            PRR_PAIR p = nullptr;
            bool batching = false;
            bool sent = false;
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
#else
            if (reqId <= (unsigned short) tagBaseRequestID::idReservedTwo) {
                throw std::invalid_argument("Request id must be larger than 0x2001");
            }
#endif
            USocket_Client_Handle h = GetClientSocketHandle();
            if (rh || discarded || serverException) {
                p = m_rrStack.Reuse();
                if (p) {
                    p->first = reqId;
                    CResultCb *rcb = p->second;
                    rcb->AsyncResultHandler.swap((DResultHandler&) rh); //assuming inline lambda expression for better performance
                    if (rcb->Discarded || discarded) {
                        rcb->Discarded.swap((DDiscarded &) discarded); //assuming inline lambda expression for better performance
                    }
                    if (rcb->ExceptionFromServer || serverException) {
                        rcb->ExceptionFromServer.swap((DServerException &) serverException); //assuming inline lambda expression for better performance
                    }
                } else {
                    p = new std::pair<unsigned short, CResultCb*>(reqId, new CResultCb(rh, discarded, serverException));
                }
                batching = ClientCoreLoader.IsBatching(h);
#ifndef NODE_JS_ADAPTER_PROJECT
                CAutoLock alSend(m_csSend);
#endif
                {
                    m_cs.lock();
                    if (batching) {
                        m_vBatching << p;
                        assert((m_vBatching.GetSize() % sizeof (PRR_PAIR)) == 0);
                    } else {
                        m_vCallback << p;
                    }
                    m_cs.unlock();
                }
                sent = ClientCoreLoader.SendRequest(h, reqId, pBuffer, size);
            } else {
                sent = ClientCoreLoader.SendRequest(h, reqId, pBuffer, size);
            }
            if (!sent) {
                if (p) {
                    m_cs.lock();
                    bool ok = (batching ? Remove(m_vBatching, p) : Remove(m_vCallback, p));
                    m_cs.unlock();
                    if (ok) {
                        m_rrStack.Recycle(p);
                    }
                }
                return false;
            }
            return true;
        }

        void CAsyncServiceHandler::SetNULL() noexcept {
            m_pClientSocket = nullptr;
        }

        void CAsyncServiceHandler::OnPostProcessing(unsigned int hint, UINT64 data) {
        }

        void CAsyncServiceHandler::OnMergeTo(CAsyncServiceHandler & to) {
        }

        void CAsyncServiceHandler::AppendTo(CAsyncServiceHandler & to) {
            to.m_cs.lock();
            m_cs.lock();
            OnMergeTo(to);
            to.m_vCallback.Push(m_vCallback.GetBuffer(), m_vCallback.GetSize());
            m_vCallback.SetSize(0);
            m_cs.unlock();
            to.m_cs.unlock();
        }

        void CAsyncServiceHandler::EraseBack(unsigned int count) {
            unsigned int total = m_vCallback.GetSize() / sizeof (PRR_PAIR);
            if (count > total)
                count = total;
            PRR_PAIR *pp = (PRR_PAIR*) m_vCallback.GetBuffer();
            unsigned int start = total - count;
            for (; start != count; ++start) {
                PRR_PAIR p = pp[start];
                CResultCb *rcb = p->second;
                if (rcb->Discarded) {
                    rcb->Discarded(this, true);
                }
                m_rrStack.Recycle(p);
            }
            m_vCallback.SetSize(m_vCallback.GetSize() - count * sizeof (PRR_PAIR));
        }

        bool CAsyncServiceHandler::SendRequest(unsigned short reqId, const DResultHandler& rh, const DDiscarded& discarded, const DServerException & se) {
            return SendRequest(reqId, (const unsigned char *) nullptr, (unsigned int) 0, rh, discarded, se);
        }

        void CAsyncServiceHandler::OnSE(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) {
            PRR_PAIR p;
            OnExceptionFromServer(requestId, errMessage, errWhere, errCode);
            if (GetAsyncResultHandler(requestId, p)) {
                CResultCb * rcb = p->second;
                if (rcb->ExceptionFromServer) {
                    rcb->ExceptionFromServer(this, requestId, errMessage, errWhere, errCode);
                }
                m_rrStack.Recycle(p);
            }
            m_seImpl.Invoke(this, requestId, errMessage, errWhere, errCode);
        }

        void CAsyncServiceHandler::OnRR(unsigned short reqId, CUQueue & mc) {
            if ((unsigned short) tagBaseRequestID::idInterrupt == reqId) {
                UINT64 options;
                mc >> options;
                OnInterrupted(options);
                return;
            }
            PRR_PAIR p;
            if (GetAsyncResultHandler(reqId, p) && p->second->AsyncResultHandler) {
                CResultCb *rcb = p->second;
                CAsyncResult ar(this, reqId, mc, rcb->AsyncResultHandler);
                rcb->AsyncResultHandler(ar);
            } else if (m_rrImpl.Invoke(this, reqId, mc)) {
            } else {
                OnResultReturned(reqId, mc);
            }
            if (p) {
                m_rrStack.Recycle(p);
            }
        }

        unsigned int CAsyncServiceHandler::GetRequestsQueued() noexcept {
            m_cs.lock();
            unsigned int count = m_vCallback.GetSize() / sizeof (PRR_PAIR);
            m_cs.unlock();
            return count;
        }

        void CAsyncServiceHandler::ShrinkDeque() {
            m_cs.lock();
            unsigned int size = m_vCallback.GetSize();
            if (size < DEFAULT_INITIAL_MEMORY_BUFFER_SIZE)
                size = DEFAULT_INITIAL_MEMORY_BUFFER_SIZE;
            m_vCallback.ReallocBuffer(size);
            size = m_vBatching.GetSize();
            if (size < DEFAULT_INITIAL_MEMORY_BUFFER_SIZE)
                size = DEFAULT_INITIAL_MEMORY_BUFFER_SIZE;
            m_vBatching.ReallocBuffer(size);
            m_cs.unlock();
        }

        unsigned int CAsyncServiceHandler::CleanCallbacks() {
            CScopeUQueue sb0, sb1;
            {
                CSpinAutoLock al(m_cs);
                sb0->Swap(m_vBatching);
                sb1->Swap(m_vCallback);
            }
            CUQueue& vBatching = *sb0;
            CUQueue& vCallback = *sb1;
            unsigned int count = vBatching.GetSize() / sizeof (PRR_PAIR);
            unsigned int total = count;
            PRR_PAIR *pp = (PRR_PAIR*) vBatching.GetBuffer();
            for (unsigned int it = 0; it < count; ++it) {
                CResultCb *rcb = pp[it]->second;
                if (rcb->Discarded) {
                    rcb->Discarded(this, GetSocket()->GetCurrentRequestID() == (unsigned short) tagBaseRequestID::idCancel);
                }
            }
            CleanQueue(vBatching);
            count = vCallback.GetSize() / sizeof (PRR_PAIR);
            pp = (PRR_PAIR*) vCallback.GetBuffer();
            for (unsigned int it = 0; it < count; ++it) {
                CResultCb *rcb = pp[it]->second;
                if (rcb->Discarded) {
                    rcb->Discarded(this, GetSocket()->GetCurrentRequestID() == (unsigned short) tagBaseRequestID::idCancel);
                }
            }
            CleanQueue(vCallback);
            total += count;
            return total;
        }

        USocket_Client_Handle CAsyncServiceHandler::GetClientSocketHandle() const noexcept {
            assert(m_pClientSocket);
            return m_pClientSocket->GetHandle();
        }

        bool CAsyncServiceHandler::GetAsyncResultHandler(unsigned short usReqId, PRR_PAIR & p) noexcept {
            CSpinAutoLock al(m_cs);
            assert((m_vCallback.GetSize() % sizeof (PRR_PAIR)) == 0);
            unsigned int count = m_vCallback.GetSize() / sizeof (PRR_PAIR);
            PRR_PAIR *pp = (PRR_PAIR*) m_vCallback.GetBuffer();
            if (m_pClientSocket->IsRandom()) {
                for (unsigned int it = 0; it < count; ++it) {
                    if (pp[it]->first == usReqId) {
                        p = pp[it];
                        m_vCallback.Pop((unsigned int) sizeof (PRR_PAIR), it * sizeof (PRR_PAIR));
                        return true;
                    }
                }
            } else if (count) {
                p = *pp;
                if (p->first == usReqId) {
                    m_vCallback.Pop((unsigned int) sizeof (PRR_PAIR));
                    return true;
                }
            }
            p = nullptr;
            return false;
        }

#ifdef ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS

        void CClientSocket::CIRequestProcessed::Invoke(CClientSocket *cs, unsigned short reqId, CUQueue & mc) {
            CSpinAutoLock al(*m_cs);
            for (auto it = m_vD.cbegin(), end = m_vD.cend(); it != end; ++it) {
                auto &d = *it;
                d(cs, reqId, mc);
            }
        }
#endif

        CClientSocket::CClientSocket() noexcept
        : m_hSocket((USocket_Client_Handle) nullptr), m_pHandler(nullptr), m_bRandom(false), m_endian(false),
        m_os(MY_OPERATION_SYSTEM), m_nCurrSvsId((unsigned int) tagServiceID::sidStartup), m_routing(false),
        SocketClosed(m_implClosed), HandShakeCompleted(m_implHSC), SocketConnected(m_implConnected),
        ExceptionFromServer(m_implEFS)
#ifdef ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS
        , BaseRequestProcessed(m_implBRP), AllRequestsProcessed(m_implARP), RequestProcessed(m_implRP)
#endif	
        {
            m_mutex.lock();
            m_vClientSocket.push_back(this);
            m_mutex.unlock();

            m_implClosed.SetCS(&m_cs);
            m_implHSC.SetCS(&m_cs);
            m_implConnected.SetCS(&m_cs);
            m_implEFS.SetCS(&m_cs);

#ifdef ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS
            m_implBRP.SetCS(&m_cs);
            m_implARP.SetCS(&m_cs);
            m_implRP.SetCS(&m_cs);
#endif

#ifdef NODE_JS_ADAPTER_PROJECT
            m_asyncType = nullptr;
#endif
        }

        void CClientSocket::Set(USocket_Client_Handle h) {
            m_hSocket = h;
            m_PushImpl.m_cs = this;
            m_QueueImpl.m_hSocket = h;
            ClientCoreLoader.SetOnHandShakeCompleted(h, &CClientSocket::OnHandShakeCompleted);
            ClientCoreLoader.SetOnRequestProcessed(h, &CClientSocket::OnRequestProcessed);
            ClientCoreLoader.SetOnSocketClosed(h, &CClientSocket::OnSocketClosed);
            ClientCoreLoader.SetOnSocketConnected(h, &CClientSocket::OnSocketConnected);
            ClientCoreLoader.SetOnEnter(h, &CClientSocket::OnSubscribe);
            ClientCoreLoader.SetOnExit(h, &CClientSocket::OnUnsubscribe);
            ClientCoreLoader.SetOnSpeakEx(h, &CClientSocket::OnBroadcastEx);
            ClientCoreLoader.SetOnSpeak(h, &CClientSocket::OnBroadcast);
            ClientCoreLoader.SetOnSendUserMessageEx(h, &CClientSocket::OnPostUserMessageEx);
            ClientCoreLoader.SetOnSendUserMessage(h, &CClientSocket::OnPostUserMessage);
            ClientCoreLoader.SetOnServerException(h, &CClientSocket::OnServerException);
            ClientCoreLoader.SetOnBaseRequestProcessed(h, &CClientSocket::OnBaseRequestProcessed);
            ClientCoreLoader.SetOnAllRequestsProcessed(h, &CClientSocket::OnAllRequestsProcessed);
            ClientCoreLoader.SetOnPostProcessing(h, &CClientSocket::OnPostProcessing);

            m_PushImpl.m_lstPublish.SetCS(&m_cs);
            m_PushImpl.m_lstPublishEx.SetCS(&m_cs);
            m_PushImpl.m_lstSub.SetCS(&m_cs);
            m_PushImpl.m_lstUnsub.SetCS(&m_cs);
            m_PushImpl.m_lstUser.SetCS(&m_cs);
            m_PushImpl.m_lstUserEx.SetCS(&m_cs);
        }

        CClientSocket::~CClientSocket() {
            if (m_pHandler) {
                m_pHandler->SetNULL();
            }
            std::vector<CClientSocket*>::iterator it;
            m_mutex.lock();
            std::vector<CClientSocket*>::iterator end = m_vClientSocket.end();
            for (it = m_vClientSocket.begin(); it != end; ++it) {
                if ((*it)->GetHandle() == m_hSocket) {
                    m_vClientSocket.erase(it);
                    break;
                }
            }
            m_mutex.unlock();
        }

        bool CClientSocket::Attach(CAsyncServiceHandler * p) {
            if (!p)
                return false;
            m_pHandler = p;
            return true;
        }

        CAsyncServiceHandler * CClientSocket::GetCurrentHandler() noexcept {
            return m_pHandler;
        }

        void CClientSocket::Detach(CAsyncServiceHandler * p) {
            if (p == m_pHandler) {
                m_pHandler = nullptr;
            }
        }

        const CConnectionContext & CClientSocket::GetConnectionContext() const noexcept {
            return m_cc;
        }

        void SetLastCallInfo(const char *str, int data, const char *func) {
            char buff[4097] = {0};
#ifdef WIN32_64
            _snprintf_s(buff, sizeof (buff), sizeof (buff), "lf: %s, what: %s, data: %d", func, str, data);
#else
            snprintf(buff, sizeof (buff), "lf: %s, what: %s, data: %d", func, str, data);
#endif
            ClientCoreLoader.SetLastCallInfo(buff);
        }

        CClientSocket * CClientSocket::Seek(USocket_Client_Handle h) noexcept {
            CClientSocket *p = nullptr;
            m_mutex.lock();
            size_t count = m_vClientSocket.size();
            PClientSocket *start = m_vClientSocket.data();
            for (size_t it = 0; it < count; ++it) {
                if (start[it]->m_hSocket == h) {
                    p = start[it];
                    break;
                }
            }
            m_mutex.unlock();
            return p;
        }

        bool CClientSocket::WaitAll(unsigned int nTimeout) const {
            //call the method after calling CommitBatching or AbortBatching
            assert(!ClientCoreLoader.IsBatching(m_hSocket)); //bad code logic!!!

            //call the method EndJob or AbortJob
            assert(!m_QueueImpl.IsAvailable() || m_QueueImpl.GetJobSize() > 0);

            return ClientCoreLoader.WaitAll(m_hSocket, nTimeout);
        }

        bool CClientSocket::Cancel(unsigned int requestsQueued) const {
            return ClientCoreLoader.Cancel(m_hSocket, requestsQueued);
        }

        unsigned int CClientSocket::GetBytesInSendingBuffer() const {
            return ClientCoreLoader.GetBytesInSendingBuffer(m_hSocket);
        }

        unsigned int CClientSocket::GetBytesInReceivingBuffer() const {
            return ClientCoreLoader.GetBytesInReceivingBuffer(m_hSocket);
        }

        unsigned int CClientSocket::GetBytesBatched() const {
            return ClientCoreLoader.GetBytesBatched(m_hSocket);
        }

        UINT64 CClientSocket::GetBytesReceived() const {
            return ClientCoreLoader.GetBytesReceived(m_hSocket);
        }

        UINT64 CClientSocket::GetBytesSent() const {
            return ClientCoreLoader.GetBytesSent(m_hSocket);
        }

        void CClientSocket::SetUID(const wchar_t * userId) const {
            ClientCoreLoader.SetUserID(m_hSocket, userId);
        }

        std::wstring CClientSocket::GetUID() const {
            wchar_t strUserId[MAX_USERID_CHARS + 1];
            ClientCoreLoader.GetUID(m_hSocket, strUserId, sizeof (strUserId) / sizeof (wchar_t));
            return strUserId;
        }

        void CClientSocket::SetPassword(const wchar_t * password) const {
            return ClientCoreLoader.SetPassword(m_hSocket, password);
        }

        CClientSocket::CPushImpl & CClientSocket::GetPush() noexcept {
            return m_PushImpl;
        }

        IClientQueue & CClientSocket::GetClientQueue() noexcept {
            return m_QueueImpl;
        }

        bool CClientSocket::CPushImpl::Subscribe(const unsigned int *pChatGroupId, unsigned int count) const {
            return ClientCoreLoader.Enter(m_cs->GetHandle(), pChatGroupId, count);
        }

        void CClientSocket::CPushImpl::Unsubscribe() const {
            ClientCoreLoader.Exit(m_cs->GetHandle());
        }

        bool CClientSocket::CPushImpl::Publish(const VARIANT& vtMessage, const unsigned int *pGroups, unsigned int ulGroupCount) const {
            if (pGroups == nullptr || ulGroupCount == 0)
                return false;
            CScopeUQueue sb;
            sb << vtMessage;
            return ClientCoreLoader.Speak(m_cs->GetHandle(), sb->GetBuffer(), sb->GetSize(), pGroups, ulGroupCount);
        }

        bool CClientSocket::CPushImpl::SendUserMessage(const VARIANT& vtMessage, const wchar_t * strUserId) const {
            CScopeUQueue sb;
            sb << vtMessage;
            return ClientCoreLoader.SendUserMessage(m_cs->GetHandle(), strUserId, sb->GetBuffer(), sb->GetSize());
        }

        bool CClientSocket::CPushImpl::PublishEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count) const {
            return ClientCoreLoader.SpeakEx(m_cs->GetHandle(), message, size, pChatGroupId, count);
        }

        bool CClientSocket::CPushImpl::SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size) const {
            return ClientCoreLoader.SendUserMessageEx(m_cs->GetHandle(), userId, message, size);
        }

        UINT64 CClientSocket::GetSocketNativeHandle() const {
            return ClientCoreLoader.GetSocketNativeHandle(m_hSocket);
        }

        tagOperationSystem CClientSocket::GetPeerOs(bool * endian) const {
            if (endian)
                *endian = m_endian;
            return m_os;
        }

        bool CClientSocket::DoEcho() const {
            return ClientCoreLoader.DoEcho(m_hSocket);
        }

        std::string CClientSocket::GetPeerName(unsigned int *port) const {
            char ipAddr[256] = {0};
            ClientCoreLoader.GetPeerName(m_hSocket, port, ipAddr, sizeof (ipAddr));
            return ipAddr;
        }

        bool CClientSocket::SetSockOpt(tagSocketOption optName, int optValue, tagSocketLevel level) const {
            return ClientCoreLoader.SetSockOpt(m_hSocket, optName, optValue, level);
        }

        bool CClientSocket::SetSockOptAtSvr(tagSocketOption optName, int optValue, tagSocketLevel level) const {
            return ClientCoreLoader.SetSockOptAtSvr(m_hSocket, optName, optValue, level);
        }

        bool CClientSocket::TurnOnZipAtSvr(bool enableZip) const {
            return ClientCoreLoader.TurnOnZipAtSvr(m_hSocket, enableZip);
        }

        bool CClientSocket::SetZipLevelAtSvr(SPA::tagZipLevel zipLevel) const {
            return ClientCoreLoader.SetZipLevelAtSvr(m_hSocket, zipLevel);
        }

        void CClientSocket::Close() const {
            ClientCoreLoader.Close(m_hSocket);
        }

        void CClientSocket::Shutdown(tagShutdownType st) const {
            ClientCoreLoader.Shutdown(m_hSocket, st);
        }

        void CClientSocket::SetZip(bool zip) const {
            ClientCoreLoader.SetZip(m_hSocket, zip);
        }

        bool CClientSocket::operator==(const CClientSocket & cs) const noexcept {
            return (m_hSocket == cs.m_hSocket);
        }

        bool CClientSocket::GetZip() const {
            return ClientCoreLoader.GetZip(m_hSocket);
        }

        void CClientSocket::SetZipLevel(SPA::tagZipLevel zl) const {
            ClientCoreLoader.SetZipLevel(m_hSocket, zl);
        }

        bool CClientSocket::QueueConfigure::IsClientQueueIndexPossiblyCrashed() {
            return ClientCoreLoader.IsClientQueueIndexPossiblyCrashed();
        }

        void CClientSocket::QueueConfigure::SetWorkDirectory(const char *dir) {
            ClientCoreLoader.SetClientWorkDirectory(dir);
        }

        const char* CClientSocket::QueueConfigure::GetWorkDirectory() {
            return ClientCoreLoader.GetClientWorkDirectory();
        }

        void CClientSocket::QueueConfigure::SetMessageQueuePassword(const char *pwd) {
            ClientCoreLoader.SetMessageQueuePassword(pwd);
        }

        bool CClientSocket::SSL::SetVerifyLocation(const char *caFile) {
            return ClientCoreLoader.SetVerifyLocation(caFile);
        }

        void CClientSocket::SSL::SetCertificateVerifyCallback(PCertificateVerifyCallback cvc) {
            ClientCoreLoader.SetCertificateVerifyCallback(cvc);
        }

        const char* CClientSocket::GetVersion() {
            return ClientCoreLoader.GetUClientSocketVersion();
        }

        SPA::tagZipLevel CClientSocket::GetZipLevel() const {
            return ClientCoreLoader.GetZipLevel(m_hSocket);
        }

        bool CClientSocket::CQueueImpl::StartQueue(const char *qName, unsigned int ttl, bool secure, bool dequeueShared) const {
            CAsyncServiceHandler* ash = CClientSocket::Seek(m_hSocket)->GetCurrentHandler();
            bool ok = ClientCoreLoader.StartQueue(m_hSocket, qName, secure, dequeueShared, ttl);
            if (ash) {
                ash->m_qStarted = ok;
            }
            return ok;
        }

        void CClientSocket::CQueueImpl::StopQueue(bool permanent) {
            CAsyncServiceHandler* ash = CClientSocket::Seek(m_hSocket)->GetCurrentHandler();
            ClientCoreLoader.StopQueue(m_hSocket, permanent);
            if (ash) {
                ash->m_qStarted = false;
            }
        }

        unsigned int CClientSocket::CQueueImpl::GetTTL() const {
            return ClientCoreLoader.GetTTL(m_hSocket);
        }

        UINT64 CClientSocket::CQueueImpl::RemoveByTTL() const {
            return ClientCoreLoader.RemoveQueuedRequestsByTTL(m_hSocket);
        }

        tagQueueStatus CClientSocket::CQueueImpl::GetQueueOpenStatus() const {
            return ClientCoreLoader.GetClientQueueStatus(m_hSocket);
        }

        std::time_t CClientSocket::CQueueImpl::GetLastMessageTime() const {
            std::tm tm;
            ::memset(&tm, 0, sizeof (tm));
            tm.tm_mday = 1;
            tm.tm_year = 113;
            return std::mktime(&tm) + ClientCoreLoader.GetLastQueueMessageTime(m_hSocket);
        }

        unsigned int CClientSocket::CQueueImpl::GetMessagesInDequeuing() const {
            return ClientCoreLoader.GetMessagesInDequeuing(m_hSocket);
        }

        void CClientSocket::CQueueImpl::Reset() const {
            ClientCoreLoader.ResetQueue(m_hSocket);
        }

        USocket_Client_Handle CClientSocket::CQueueImpl::GetHandle() const {
            return m_hSocket;
        }

        bool CClientSocket::CQueueImpl::AppendTo(const IClientQueue & clientQueue) const {
            USocket_Client_Handle h = clientQueue.GetHandle();
            return ClientCoreLoader.PushQueueTo(m_hSocket, &h, 1);
        }

        bool CClientSocket::CQueueImpl::AppendTo(const USocket_Client_Handle *handles, unsigned int count) const {
            return ClientCoreLoader.PushQueueTo(m_hSocket, handles, count);
        }

        bool CClientSocket::CQueueImpl::EnsureAppending(const IClientQueue & clientQueue) const {
            USocket_Client_Handle h = clientQueue.GetHandle();
            return EnsureAppending(&h, 1);
        }

        bool CClientSocket::CQueueImpl::EnsureAppending(const USocket_Client_Handle *handles, unsigned int count) const {
            unsigned int n;
            if (IsAvailable())
                return false;
            if (GetQueueOpenStatus() != SPA::tagQueueStatus::qsMergePushing)
                return true;
            if (!handles || !count)
                return true; //don't do anything in case there is no target queue available
            std::vector<USocket_Client_Handle> vHandles;
            for (n = 0; n < count; ++n) {
                if (ClientCoreLoader.GetClientQueueStatus(handles[n]) != tagQueueStatus::qsMergeComplete)
                    vHandles.push_back(handles[n]);
            }
            count = (unsigned int) vHandles.size();
            if (count > 0) {
                const USocket_Client_Handle &header = vHandles.front();
                return AppendTo(&header, count);
            }
            Reset();
            return true;
        }

        bool CClientSocket::CQueueImpl::AbortJob() const {
            CAsyncServiceHandler *ash = CClientSocket::Seek(m_hSocket)->GetCurrentHandler();
            ash->m_cs.lock();
            unsigned int aborted = ash->m_vCallback.GetSize() / sizeof (CAsyncServiceHandler::PRR_PAIR) - m_nQIndex;
            if (ClientCoreLoader.AbortJob(m_hSocket)) {
                ash->EraseBack(aborted);
                ash->m_cs.unlock();
                return true;
            }
            ash->m_cs.unlock();
            return false;
        }

        bool CClientSocket::CQueueImpl::StartJob() const {
            CAsyncServiceHandler *ash = CClientSocket::Seek(m_hSocket)->GetCurrentHandler();
            ash->m_cs.lock();
            (const_cast<CQueueImpl*> (this))->m_nQIndex = ash->m_vCallback.GetSize() / sizeof (CAsyncServiceHandler::PRR_PAIR);
            bool ok = ClientCoreLoader.StartJob(m_hSocket);
            ash->m_cs.unlock();
            return ok;
        }

        bool CClientSocket::CQueueImpl::EndJob() const {
            return ClientCoreLoader.EndJob(m_hSocket);
        }

        UINT64 CClientSocket::CQueueImpl::GetJobSize() const {
            return ClientCoreLoader.GetJobSize(m_hSocket);
        }

        UINT64 CClientSocket::CQueueImpl::GetLastIndex() const {
            return ClientCoreLoader.GetQueueLastIndex(m_hSocket);
        }

        UINT64 CClientSocket::CQueueImpl::CancelQueuedRequests(UINT64 startIndex, UINT64 endIndex) const {
            return ClientCoreLoader.CancelQueuedRequestsByIndex(m_hSocket, startIndex, endIndex);
        }

        bool CClientSocket::CQueueImpl::IsDequeueEnabled() const {
            return ClientCoreLoader.IsDequeueEnabled(m_hSocket);
        }

        bool CClientSocket::CQueueImpl::IsAvailable() const {
            return ClientCoreLoader.IsQueueStarted(m_hSocket);
        }

        bool CClientSocket::CQueueImpl::IsDequeueShared() const {
            return ClientCoreLoader.IsDequeueShared(m_hSocket);
        }

        const char* CClientSocket::CQueueImpl::GetQueueFileName() const {
            return ClientCoreLoader.GetQueueFileName(m_hSocket);
        }

        const char* CClientSocket::CQueueImpl::GetQueueName() const {
            return ClientCoreLoader.GetQueueName(m_hSocket);
        }

        bool CClientSocket::CQueueImpl::IsSecure() const {
            return ClientCoreLoader.IsQueueSecured(m_hSocket);
        }

        SPA::UINT64 CClientSocket::CQueueImpl::GetMessageCount() const {
            return ClientCoreLoader.GetMessageCount(m_hSocket);
        }

        SPA::UINT64 CClientSocket::CQueueImpl::GetQueueSize() const {
            return ClientCoreLoader.GetQueueSize(m_hSocket);
        }

        void CClientSocket::CQueueImpl::EnableRoutingQueueIndex(bool enable) const {
            ClientCoreLoader.EnableRoutingQueueIndex(m_hSocket, enable);
        }

        bool CClientSocket::CQueueImpl::IsRoutingQueueIndexEnabled() const {
            return ClientCoreLoader.IsRoutingQueueIndexEnabled(m_hSocket);
        }

        tagOptimistic CClientSocket::CQueueImpl::GetOptimistic() const {
            return ClientCoreLoader.GetOptimistic(m_hSocket);
        }

        void CClientSocket::CQueueImpl::SetOptimistic(tagOptimistic optimistic) const {
            ClientCoreLoader.SetOptimistic(m_hSocket, optimistic);
        }

        unsigned int CClientSocket::GetConnTimeout() const {
            return ClientCoreLoader.GetConnTimeout(m_hSocket);
        }

        unsigned int CClientSocket::GetRecvTimeout() const {
            return ClientCoreLoader.GetRecvTimeout(m_hSocket);
        }

        void CClientSocket::SetRecvTimeout(unsigned int timeout) const {
            ClientCoreLoader.SetRecvTimeout(m_hSocket, timeout);
        }

        void CClientSocket::SetConnTimeout(unsigned int timeout) const {
            ClientCoreLoader.SetConnTimeout(m_hSocket, timeout);
        }

        void CClientSocket::SetAutoConn(bool autoConnecting) const {
            ClientCoreLoader.SetAutoConn(m_hSocket, autoConnecting);
        }

        bool CClientSocket::GetAutoConn() const {
            return ClientCoreLoader.GetAutoConn(m_hSocket);
        }

        unsigned int CClientSocket::GetCountOfRequestsInQueue() const {
            return ClientCoreLoader.GetCountOfRequestsQueued(m_hSocket);
        }

        unsigned short CClientSocket::GetCurrentRequestID() const {
            return ClientCoreLoader.GetCurrentRequestID(m_hSocket);
        }

        unsigned short CClientSocket::GetServerPingTime() const {
            return ClientCoreLoader.GetServerPingTime(m_hSocket);
        }

        unsigned int CClientSocket::GetCurrentResultSize() const {
            return ClientCoreLoader.GetCurrentResultSize(m_hSocket);
        }

        tagEncryptionMethod CClientSocket::GetEncryptionMethod() const {
            return ClientCoreLoader.GetEncryptionMethod(m_hSocket);
        }

        int CClientSocket::GetErrorCode() const {
            return ClientCoreLoader.GetErrorCode(m_hSocket);
        }

        bool CClientSocket::IgnoreLastRequest(unsigned short reqId) const {
            return ClientCoreLoader.IgnoreLastRequest(m_hSocket, reqId);
        }

        unsigned int CClientSocket::GetRouteeCount() const {
            return ClientCoreLoader.GetRouteeCount(m_hSocket);
        }

        std::string CClientSocket::GetErrorMsg() const {
            char strErrorMsg[1025] = {0};
            ClientCoreLoader.GetErrorMessage(m_hSocket, strErrorMsg, sizeof (strErrorMsg));
            return strErrorMsg;
        }

        unsigned int CClientSocket::GetPoolId() const {
            return ClientCoreLoader.GetSocketPoolId(m_hSocket);
        }

        bool CClientSocket::IsConnected() const {
            return ClientCoreLoader.IsOpened(m_hSocket);
        }

        bool CClientSocket::Sendable() const {
            return (ClientCoreLoader.IsOpened(m_hSocket) || m_QueueImpl.IsAvailable());
        }

        IUcert * CClientSocket::GetUCert() const {
            return ClientCoreLoader.GetUCertEx(m_hSocket);
        }

        tagConnectionState CClientSocket::GetConnectionState() const {
            return ClientCoreLoader.GetConnectionState(m_hSocket);
        }

        void* CClientSocket::GetSslHandle() const {
            return ClientCoreLoader.GetSSL(m_hSocket);
        }

        void CClientSocket::SetEncryptionMethod(tagEncryptionMethod em) const {
            ClientCoreLoader.SetEncryptionMethod(m_hSocket, em);
        }

        void CClientSocket::OnSocketClosed(int nError) {

        }

        void CClientSocket::OnHandShakeCompleted(int nError) {

        }

        void CClientSocket::OnSocketConnected(int nError) {

        }

        void CClientSocket::OnRequestProcessed(unsigned short requestId, CUQueue & q) {

        }

        void CClientSocket::OnSubscribe(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count) {

        }

        void CClientSocket::OnUnsubscribe(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count) {

        }

        void CClientSocket::OnPublishEx(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size) {

        }

        void CClientSocket::OnPublish(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count, const SPA::UVariant & vtMessage) {

        }

        void CClientSocket::OnSendUserMessage(const CMessageSender& sender, const SPA::UVariant & message) {

        }

        void CClientSocket::OnSendUserMessageEx(const CMessageSender& sender, const unsigned char *pMessage, unsigned int size) {

        }

        void CClientSocket::OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) {

        }

        void CClientSocket::OnBaseRequestProcessed(unsigned short requestId) {

        }

        void WINAPI CClientSocket::OnSocketClosed(USocket_Client_Handle handler, int nError) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            if (!p->GetClientQueue().IsAvailable()) {
                CAsyncServiceHandler *ash = p->GetCurrentHandler();
                if (ash)
                    ash->CleanCallbacks();
            }
            p->m_implClosed.Invoke(p, nError);
            p->OnSocketClosed(nError);
        }

        void WINAPI CClientSocket::OnHandShakeCompleted(USocket_Client_Handle handler, int nError) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            p->m_implHSC.Invoke(p, nError);
            p->OnHandShakeCompleted(nError);
        }

        void WINAPI CClientSocket::OnSocketConnected(USocket_Client_Handle handler, int nError) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            p->m_implConnected.Invoke(p, nError);
            p->OnSocketConnected(nError);
        }

        void WINAPI CClientSocket::OnRequestProcessed(USocket_Client_Handle handler, unsigned short requestId, unsigned int len) {
            CClientSocket *p = Seek(handler);
            if (p) {
                CUQueue &q = p->m_qRecv;
                q.SetSize(0);
                if (p->IsRouting()) {
                    p->m_os = ClientCoreLoader.GetPeerOs(handler, &p->m_endian);
                    q.SetOS(p->m_os);
                    q.SetEndian(p->m_endian);
                }
                if (len > q.GetMaxSize())
                    q.ReallocBuffer(len + sizeof (wchar_t));
                if (len) {
                    const unsigned char *result = ClientCoreLoader.GetResultBuffer(handler);
                    q.Push(result, len);
                }
                PAsyncServiceHandler ash = p->m_pHandler;
                if (ash)
                    ash->OnRR(requestId, q);
#ifdef ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS
                p->m_implRP.Invoke(p, requestId, q);
#endif
                p->OnRequestProcessed(requestId, q);
#ifdef NODE_JS_ADAPTER_PROJECT
                NJA::NJSocketPool *pool = (NJA::NJSocketPool *)p->m_asyncType->data;
                if (!pool)
                    return;
                {
                    CSpinAutoLock al(pool->m_cs);
                    if (pool->m_rr.IsEmpty())
                        return;
                }
                NJA::SocketEvent se;
                CUQueue *q2 = CScopeUQueue::Lock();
                *q2 << ash << requestId;
                q2->Push(q.GetBuffer(), q.GetSize());
                se.QData = q2;
                se.Se = NJA::tagSocketEvent::seResultReturned;
                CSpinAutoLock al(pool->m_cs);
                pool->m_deqSocketEvent.push_back(se);
                if (pool->m_deqSocketEvent.size() < 2) {
                    int fail = uv_async_send(p->m_asyncType);
                    assert(!fail);
                }
#endif
            }
        }

        void WINAPI CClientSocket::OnSubscribe(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            p->GetPush().m_lstSub.Invoke(p, sender, pGroup, count);
            p->OnSubscribe(sender, pGroup, count);
#ifdef NODE_JS_ADAPTER_PROJECT
            VARTYPE vt = (VT_ARRAY | VT_UI4);
            unsigned short reqId = (unsigned short) tagChatRequestID::idEnter;
            NJA::NJSocketPool *pool = (NJA::NJSocketPool *)p->m_asyncType->data;
            if (!pool)
                return;
            {
                CSpinAutoLock al(pool->m_cs);
                if (pool->m_push.IsEmpty())
                    return;
            }
            CUQueue *q = CScopeUQueue::Lock();
            *q << p->GetCurrentHandler() << reqId << sender << vt << count;
            q->Push((const unsigned char*) pGroup, count * sizeof (unsigned int));
            NJA::SocketEvent se;
            se.QData = q;
            se.Se = NJA::tagSocketEvent::seChatEnter;
            pool->m_deqSocketEvent.push_back(se);
            CSpinAutoLock al(pool->m_cs);
            if (pool->m_deqSocketEvent.size() < 2) {
                int fail = uv_async_send(p->m_asyncType);
                assert(!fail);
            }
#endif
        }

        void WINAPI CClientSocket::OnUnsubscribe(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            p->GetPush().m_lstUnsub.Invoke(p, sender, pGroup, count);
            p->OnUnsubscribe(sender, pGroup, count);
#ifdef NODE_JS_ADAPTER_PROJECT
            VARTYPE vt = (VT_ARRAY | VT_UI4);
            unsigned short reqId = (unsigned short) tagChatRequestID::idExit;
            NJA::NJSocketPool *pool = (NJA::NJSocketPool *)p->m_asyncType->data;
            if (!pool)
                return;
            {
                CSpinAutoLock al(pool->m_cs);
                if (pool->m_push.IsEmpty())
                    return;
            }
            CUQueue *q = CScopeUQueue::Lock();
            *q << p->GetCurrentHandler() << reqId << sender << vt << count;
            q->Push((const unsigned char*) pGroup, count * sizeof (unsigned int));
            NJA::SocketEvent se;
            se.QData = q;
            se.Se = NJA::tagSocketEvent::seChatExit;
            CSpinAutoLock al(pool->m_cs);
            pool->m_deqSocketEvent.push_back(se);
            if (pool->m_deqSocketEvent.size() < 2) {
                int fail = uv_async_send(p->m_asyncType);
                assert(!fail);
            }
#endif
        }

        void WINAPI WINAPI CClientSocket::OnBroadcast(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            CScopeUQueue sb;
            sb->Push(pMessage, size);
            SPA::UVariant vtMessage;
            sb >> vtMessage;
            p->GetPush().m_lstPublish.Invoke(p, sender, pGroup, count, vtMessage);
            p->OnPublish(sender, pGroup, count, vtMessage);
#ifdef NODE_JS_ADAPTER_PROJECT
            VARTYPE vt = (VT_ARRAY | VT_UI4);
            unsigned short reqId = (unsigned short) tagChatRequestID::idSpeak;
            NJA::NJSocketPool *pool = (NJA::NJSocketPool *)p->m_asyncType->data;
            if (!pool)
                return;
            {
                CSpinAutoLock al(pool->m_cs);
                if (pool->m_push.IsEmpty())
                    return;
            }
            CUQueue *q = CScopeUQueue::Lock();
            *q << p->GetCurrentHandler() << reqId << sender << vt << count;
            q->Push((const unsigned char*) pGroup, count * sizeof (unsigned int));
            q->Push(pMessage, size);
            NJA::SocketEvent se;
            se.QData = q;
            se.Se = NJA::tagSocketEvent::sePublish;
            CSpinAutoLock al(pool->m_cs);
            pool->m_deqSocketEvent.push_back(se);
            if (pool->m_deqSocketEvent.size() < 2) {
                int fail = uv_async_send(p->m_asyncType);
                assert(!fail);
            }
#endif
        }

        void WINAPI CClientSocket::OnBroadcastEx(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            p->GetPush().m_lstPublishEx.Invoke(p, sender, pGroup, count, pMessage, size);
            p->OnPublishEx(sender, pGroup, count, pMessage, size);
#ifdef NODE_JS_ADAPTER_PROJECT
            VARTYPE vt = (VT_ARRAY | VT_UI4);
            unsigned short reqId = (unsigned short) tagChatRequestID::idSpeakEx;
            NJA::NJSocketPool *pool = (NJA::NJSocketPool *)p->m_asyncType->data;
            if (!pool)
                return;
            {
                CSpinAutoLock al(pool->m_cs);
                if (pool->m_push.IsEmpty())
                    return;
            }
            CUQueue *q = CScopeUQueue::Lock();
            *q << p->GetCurrentHandler() << reqId << sender << vt << count;
            q->Push((const unsigned char*) pGroup, count * sizeof (unsigned int));
            q->Push(pMessage, size);
            NJA::SocketEvent se;
            se.QData = q;
            se.Se = NJA::tagSocketEvent::sePublishEx;
            CSpinAutoLock al(pool->m_cs);
            pool->m_deqSocketEvent.push_back(se);
            if (pool->m_deqSocketEvent.size() < 2) {
                int fail = uv_async_send(p->m_asyncType);
                assert(!fail);
            }
#endif
        }

        void WINAPI CClientSocket::OnPostUserMessage(USocket_Client_Handle handler, CMessageSender sender, const unsigned char *pMessage, unsigned int size) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            CScopeUQueue sb;
            sb->Push(pMessage, size);
            SPA::UVariant vtMessage;
            sb >> vtMessage;
            p->GetPush().m_lstUser.Invoke(p, sender, vtMessage);
            p->OnSendUserMessage(sender, vtMessage);
#ifdef NODE_JS_ADAPTER_PROJECT
            unsigned short reqId = (unsigned short) tagChatRequestID::idSendUserMessage;
            NJA::NJSocketPool *pool = (NJA::NJSocketPool *)p->m_asyncType->data;
            if (!pool)
                return;
            {
                CSpinAutoLock al(pool->m_cs);
                if (pool->m_push.IsEmpty())
                    return;
            }
            CUQueue *q = CScopeUQueue::Lock();
            *q << p->GetCurrentHandler() << reqId << sender;
            q->Push(pMessage, size);
            NJA::SocketEvent se;
            se.QData = q;
            se.Se = NJA::tagSocketEvent::sePostUserMessage;
            CSpinAutoLock al(pool->m_cs);
            pool->m_deqSocketEvent.push_back(se);
            if (pool->m_deqSocketEvent.size() < 2) {
                int fail = uv_async_send(p->m_asyncType);
                assert(!fail);
            }
#endif
        }

        void WINAPI CClientSocket::OnPostUserMessageEx(USocket_Client_Handle handler, CMessageSender sender, const unsigned char *pMessage, unsigned int size) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            p->GetPush().m_lstUserEx.Invoke(p, sender, pMessage, size);
            p->OnSendUserMessageEx(sender, pMessage, size);
#ifdef NODE_JS_ADAPTER_PROJECT
            unsigned short reqId = (unsigned short) tagChatRequestID::idSendUserMessageEx;
            NJA::NJSocketPool *pool = (NJA::NJSocketPool *)p->m_asyncType->data;
            if (!pool)
                return;
            NJA::SocketEvent se;
            {
                CSpinAutoLock al(pool->m_cs);
                if (pool->m_push.IsEmpty())
                    return;
            }
            CUQueue *q = CScopeUQueue::Lock();
            *q << p->GetCurrentHandler() << reqId << sender;
            q->Push(pMessage, size);
            se.QData = q;
            se.Se = NJA::tagSocketEvent::sePostUserMessageEx;
            CSpinAutoLock al(pool->m_cs);
            pool->m_deqSocketEvent.push_back(se);
            if (pool->m_deqSocketEvent.size() < 2) {
                int fail = uv_async_send(p->m_asyncType);
                assert(!fail);
            }
#endif
        }

        void WINAPI CClientSocket::OnPostProcessing(USocket_Client_Handle handler, unsigned int hint, SPA::UINT64 data) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            PAsyncServiceHandler ash = p->m_pHandler;
            if (ash) {
                ash->OnPostProcessing(hint, data);
            }
        }

        void CClientSocket::OnAllRequestsProcessed(unsigned short lastRequestId) {

        }

        void WINAPI CClientSocket::OnAllRequestsProcessed(USocket_Client_Handle handler, unsigned short lastRequestId) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            PAsyncServiceHandler ash = p->m_pHandler;
            if (ash)
                ash->OnAllProcessed();
#ifdef ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS
            p->m_implARP.Invoke(p, lastRequestId);
#endif
            p->OnAllRequestsProcessed(lastRequestId);
#ifdef NODE_JS_ADAPTER_PROJECT
            NJA::NJSocketPool *pool = (NJA::NJSocketPool *)p->m_asyncType->data;
            if (!pool)
                return;
            NJA::SocketEvent se;
            CUQueue *q = CScopeUQueue::Lock();
            *q << ash << lastRequestId;
            se.QData = q;
            se.Se = NJA::tagSocketEvent::seAllProcessed;
            CSpinAutoLock al(pool->m_cs);
            pool->m_deqSocketEvent.push_back(se);
            if (pool->m_deqSocketEvent.size() < 2) {
                int fail = uv_async_send(p->m_asyncType);
                assert(!fail);
            }
#endif
        }

        void WINAPI CClientSocket::OnBaseRequestProcessed(USocket_Client_Handle handler, unsigned short requestId) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            if (requestId == (unsigned short) tagBaseRequestID::idSwitchTo) {
                p->m_bRandom = ClientCoreLoader.IsRandom(handler);
                p->m_os = ClientCoreLoader.GetPeerOs(handler, &p->m_endian);
                p->m_nCurrSvsId = ClientCoreLoader.GetCurrentServiceId(handler);
                p->m_routing = ClientCoreLoader.IsRouting(handler);
                p->m_qRecv.SetEndian(p->m_endian);
                p->m_qRecv.SetOS(p->m_os);
            }
            PAsyncServiceHandler ash = p->m_pHandler;
            if (ash) {
                ash->m_brpImpl.Invoke(ash, requestId);
                ash->OnBaseRequestprocessed(requestId);
                if (requestId == (unsigned short) tagBaseRequestID::idCancel)
                    ash->CleanCallbacks();
            }
#ifdef ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS
            p->m_implBRP.Invoke(p, requestId);
#endif
            p->OnBaseRequestProcessed(requestId);
#ifdef NODE_JS_ADAPTER_PROJECT
            NJA::NJSocketPool *pool = (NJA::NJSocketPool *)p->m_asyncType->data;
            if (!pool)
                return;
            {
                CSpinAutoLock al(pool->m_cs);
                if (pool->m_brp.IsEmpty())
                    return;
            }
            NJA::SocketEvent se;
            CUQueue *q = CScopeUQueue::Lock();
            *q << ash << requestId;
            se.QData = q;
            se.Se = NJA::tagSocketEvent::seBaseRequestProcessed;
            CSpinAutoLock al(pool->m_cs);
            pool->m_deqSocketEvent.push_back(se);
            if (pool->m_deqSocketEvent.size() < 2) {
                int fail = uv_async_send(p->m_asyncType);
                assert(!fail);
            }
#endif
        }

        void WINAPI CClientSocket::OnServerException(USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            if (p->ExceptionFromServer) {
                p->m_implEFS.Invoke(p, requestId, errMessage, errWhere, errCode);
            }
            PAsyncServiceHandler ash = p->m_pHandler;
            if (ash)
                ash->OnSE(requestId, errMessage, errWhere, errCode);
            p->OnExceptionFromServer(requestId, errMessage, errWhere, errCode);
#ifdef NODE_JS_ADAPTER_PROJECT
            NJA::NJSocketPool *pool = (NJA::NJSocketPool *)p->m_asyncType->data;
            if (!pool)
                return;
            {
                CSpinAutoLock al(pool->m_cs);
                if (pool->m_se.IsEmpty())
                    return;
            }
            NJA::SocketEvent se;
            CUQueue *q = CScopeUQueue::Lock();
            *q << ash << requestId << errMessage << errWhere << errCode;
            se.QData = q;
            se.Se = NJA::tagSocketEvent::seServerException;
            CSpinAutoLock al(pool->m_cs);
            pool->m_deqSocketEvent.push_back(se);
            if (pool->m_deqSocketEvent.size() < 2) {
                int fail = uv_async_send(p->m_asyncType);
                assert(!fail);
            }
#endif
        }
    }//ClientSide
}//SPA