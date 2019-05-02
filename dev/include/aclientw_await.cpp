
#include "aclientw.h"

namespace SPA
{
    namespace ClientSide{

        CUCriticalSection CClientSocket::m_mutex;
        std::vector<CClientSocket*> CClientSocket::m_vClientSocket;

        Internal::CClientCoreLoader ClientCoreLoader;

        CAsyncServiceHandler::CAsyncServiceHandler(unsigned int nServiceId, CClientSocket * cs)
        :
#if defined(_FUTURE_) || defined(_GLIBCXX_FUTURE)
        m_nAsyncBatching(0),
#endif
        m_nServiceId(nServiceId), m_pClientSocket(nullptr) {
            if (cs)
                Attach(cs);
        }

        CAsyncServiceHandler::~CAsyncServiceHandler() {
            CAutoLock al(m_cs);
            if (m_pClientSocket)
                m_pClientSocket->Detach(this);
        }

        void CAsyncServiceHandler::OnResultReturned(unsigned short reqId, CUQueue & mc) {

        }

        void CAsyncServiceHandler::OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) {

        }

        unsigned int CAsyncServiceHandler::GetSvsID() const {
            return m_nServiceId;
        }

        bool CAsyncServiceHandler::WaitAll(unsigned int timeOut) {
            USocket_Client_Handle h = GetClientSocketHandle();
            //call the method after calling CommitBatching or AbortBatching
            assert(!ClientCoreLoader.IsBatching(h)); //bad code logic!!!

            //call the method EndJob or AbortJob
            assert(!(ClientCoreLoader.IsQueueStarted(h) && ClientCoreLoader.GetJobSize(h) > 0));
            return ClientCoreLoader.WaitAll(h, timeOut);
        }

        bool CAsyncServiceHandler::StartBatching() {
            if (ClientCoreLoader.StartBatching(GetClientSocketHandle())) {
#if defined(_FUTURE_) || defined(_GLIBCXX_FUTURE)
                m_cs.lock();
                m_nAsyncBatching = 0;
                m_cs.unlock();
#endif
                return true;
            }
            return false;
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

        bool CAsyncServiceHandler::SendRouteeResult(const CUQueue &mc, unsigned short reqId) {
            return SendRouteeResult(mc.GetBuffer(), mc.GetSize(), reqId);
        }

        bool CAsyncServiceHandler::SendRouteeResult(const CScopeUQueue &sb, unsigned short reqId) {
            return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), reqId);
        }

        CClientSocket * CAsyncServiceHandler::GetAttachedClientSocket() {
            CAutoLock al(m_cs);
            return m_pClientSocket;
        }

        void CAsyncServiceHandler::Detach() {
            Attach(nullptr);
        }

        bool CAsyncServiceHandler::Attach(CClientSocket * cs) {
            CAutoLock al(m_cs);
            if (cs && m_pClientSocket == cs)
                return true;
            if (m_pClientSocket)
                m_pClientSocket->Detach(this);
            m_vCallback.clear();
            if (cs) {
                if (cs->Attach(this)) {
                    m_pClientSocket = cs;
                    return true;
                }
            } else
                return true;
            return false;
        }

        bool CAsyncServiceHandler::CommitBatching(bool bBatchingAtServerSide) {
            m_cs.lock();
            m_vCallback.insert(m_vCallback.end(), m_aBatching.begin(), m_aBatching.end());
            m_aBatching.clear();
#if defined(_FUTURE_) || defined(_GLIBCXX_FUTURE)
            m_nAsyncBatching = 0;
#endif
            m_cs.unlock();
            return ClientCoreLoader.CommitBatching(GetClientSocketHandle(), bBatchingAtServerSide);
        }

        bool CAsyncServiceHandler::AbortBatching() {
            m_cs.lock();
#if defined(_FUTURE_) || defined(_GLIBCXX_FUTURE)
            while (m_nAsyncBatching && m_deqUAsyncCancel.size() > 0) {
                m_deqUAsyncCancel.back()->set_uexception(std::make_exception_ptr(std::future_error("Request aborted")));
                m_deqUAsyncCancel.pop_back();
                --m_nAsyncBatching;
            }
            m_nAsyncBatching = 0;
#endif
            m_aBatching.clear();
            m_cs.unlock();
            return ClientCoreLoader.AbortBatching(GetClientSocketHandle());
        }

        bool CAsyncServiceHandler::IsBatching() {
            return ClientCoreLoader.IsBatching(GetClientSocketHandle());
        }

        bool CAsyncServiceHandler::SendRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int size, const ResultHandler & rh) {
            USocket_Client_Handle h = GetClientSocketHandle();
            if (rh) {
                bool batching = ClientCoreLoader.IsBatching(h);
                CAutoLock al(m_cs);
                if (batching)
                    m_aBatching.push_back(std::pair<unsigned short, ResultHandler>(reqId, rh));
                else
                    m_vCallback.push_back(std::pair<unsigned short, ResultHandler>(reqId, rh));
            }
            return ClientCoreLoader.SendRequest(h, reqId, pBuffer, size);
        }

        void CAsyncServiceHandler::SetNULL() {
            m_pClientSocket = nullptr;
        }

        void CAsyncServiceHandler::EraseBack(size_t count) {
            size_t total = m_vCallback.size();
            auto start = m_vCallback.begin() + (total - count);
            m_vCallback.erase(start, m_vCallback.end());
        }

        bool CAsyncServiceHandler::SendRequest(unsigned short reqId, const ResultHandler & rh) {
            return SendRequest(reqId, (const unsigned char *) nullptr, (unsigned int) 0, rh);
        }

        void CAsyncServiceHandler::OnSE(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) {
            std::pair<unsigned short, ResultHandler> p;
            bool ok = GetAsyncResultHandler(requestId, p);
            OnExceptionFromServer(requestId, errMessage, errWhere, errCode);
            if (ServerException)
                ServerException(this, requestId, errMessage, errWhere, errCode);
        }

        void CAsyncServiceHandler::OnRR(unsigned short reqId, CUQueue & mc) {
            std::pair<unsigned short, ResultHandler> p;
            if (GetAsyncResultHandler(reqId, p)) {
                CAsyncResult ar(this, reqId, mc, p.second);
                p.second(ar);
            } else {
                if (ResultReturned)
                    ResultReturned(this, reqId, mc);
                else
                    OnResultReturned(reqId, mc);
            }
        }

        unsigned int CAsyncServiceHandler::CleanCallbacks() {
            m_cs.lock();
#if defined(_FUTURE_) || defined(_GLIBCXX_FUTURE)
            while (m_deqUAsyncCancel.size() > 0) {
                m_deqUAsyncCancel.front()->set_uexception(std::make_exception_ptr(std::future_error("Request removed")));
                m_deqUAsyncCancel.pop_front();
            }
            m_nAsyncBatching = 0;
#endif
            unsigned int ulCount = (unsigned int) m_vCallback.size();
            m_vCallback.clear();
            m_aBatching.clear();
            m_cs.unlock();
            return ulCount;
        }

        USocket_Client_Handle CAsyncServiceHandler::GetClientSocketHandle() const {
            if (m_pClientSocket)
                return m_pClientSocket->GetHandle();
            return nullptr;
        }

        bool CAsyncServiceHandler::GetAsyncResultHandler(unsigned short usReqId, std::pair<unsigned short, ResultHandler> &p) {
            CAutoLock al(m_cs);
            if (m_pClientSocket->IsRandom()) {
                for (CRHVector::iterator it = m_vCallback.begin(), end = m_vCallback.end(); it != end; ++it) {
                    if (it->first == usReqId) {
                        p = (*it);
                        m_vCallback.erase(it);
                        return true;
                    }
                }
            } else {
                CRHVector::iterator it = m_vCallback.begin();
                if (it != m_vCallback.end() && it->first == usReqId) {
                    p = (*it);
                    m_vCallback.erase(it);
                    return true;
                }
            }
            return false;
        }

        CClientSocket::CClientSocket()
        : m_hSocket((USocket_Client_Handle) nullptr), m_bRandom(false), m_endian(false), m_os(MY_OPERATION_SYSTEM), m_nCurrSvsId(sidStartup), m_routing(false) {
            m_mutex.lock();
            m_vClientSocket.push_back(this);
            m_mutex.unlock();
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
            ClientCoreLoader.SetOnAllRequestsProcessed(h, &CClientSocket::OnBaseRequestProcessed);
        }

        CClientSocket::~CClientSocket() {
            for (auto it = m_vHandler.begin(), end = m_vHandler.end(); it != end; ++it) {
                (*it)->SetNULL();
            }
            m_vHandler.clear();

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

        CAsyncServiceHandler * CClientSocket::Seek(unsigned int nServiceId) {
            CAsyncServiceHandler *p = nullptr;
            std::vector<CAsyncServiceHandler*>::iterator it;
            std::vector<CAsyncServiceHandler*>::iterator end = m_vHandler.end();
            for (it = m_vHandler.begin(); it != end; ++it) {
                if ((*it)->GetSvsID() == nServiceId) {
                    p = *it;
                    break;
                }
            }
            return p;
        }

        bool CClientSocket::Attach(CAsyncServiceHandler * p) {
            if (!p)
                return false;
            bool ok = true;
            std::vector<CAsyncServiceHandler*>::iterator it;
            std::vector<CAsyncServiceHandler*>::iterator end = m_vHandler.end();
            for (it = m_vHandler.begin(); it != end; ++it) {
                if ((*it)->GetSvsID() == p->GetSvsID()) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                m_vHandler.push_back(p);
            return ok;
        }

        CAsyncServiceHandler * CClientSocket::GetCurrentHandler() {
            std::vector<CAsyncServiceHandler*>::iterator it;
            unsigned int svsId = GetCurrentServiceID();
            std::vector<CAsyncServiceHandler*>::iterator end = m_vHandler.end();
            for (it = m_vHandler.begin(); it != end; ++it) {
                if ((*it)->GetSvsID() == svsId) {
                    return *it;
                }
            }
            if (m_vHandler.size() > 0)
                return m_vHandler.front();
            return nullptr;
        }

        void CClientSocket::Detach(CAsyncServiceHandler * p) {
            if (!p)
                return;
            std::vector<CAsyncServiceHandler*>::iterator it;
            std::vector<CAsyncServiceHandler*>::iterator end = m_vHandler.end();
            for (it = m_vHandler.begin(); it != end; ++it) {
                if ((*it) == p) {
                    p->SetNULL();
                    m_vHandler.erase(it);
                    break;
                }
            }
        }

        USocket_Client_Handle CClientSocket::GetHandle() const {
            return m_hSocket;
        }

        void SetLastCallInfo(const char *str, int data, const char *func) {
            char buff[4097] =
            {0};
#ifdef WIN32_64
            _snprintf_s(buff, sizeof (buff), sizeof (buff), "lf: %s, what: %s, data: %d", func, str, data);
#else
            snprintf(buff, sizeof (buff), "lf: %s, what: %s, data: %d", func, str, data);
#endif
            ClientCoreLoader.SetLastCallInfo(buff);
        }

        CClientSocket * CClientSocket::Seek(USocket_Client_Handle h) {
            CClientSocket *p = nullptr;
            std::vector<CClientSocket*>::iterator it;
            m_mutex.lock();
            std::vector<CClientSocket*>::iterator end = m_vClientSocket.end();
            for (it = m_vClientSocket.begin(); it != end; ++it) {
                if ((*it)->GetHandle() == h) {
                    p = (*it);
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

        bool CClientSocket::IsRandom() const {
            return m_bRandom;
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

        CClientSocket::CPushImpl & CClientSocket::GetPush() {
            return m_PushImpl;
        }

        IClientQueue & CClientSocket::GetClientQueue() {
            return m_QueueImpl;
        }

        bool CClientSocket::CPushImpl::Subscribe(const unsigned int *pChatGroupId, unsigned int count) const {
            return ClientCoreLoader.Enter(m_cs->GetHandle(), pChatGroupId, count);
        }

        void CClientSocket::CPushImpl::Unsubscribe() const {
            ClientCoreLoader.Exit(m_cs->GetHandle());
        }

        bool CClientSocket::CPushImpl::Publish(const UVariant& vtMessage, const unsigned int *pGroups, unsigned int ulGroupCount) const {
            if (pGroups == nullptr || ulGroupCount == 0)
                return false;
            CScopeUQueue su;
            su << vtMessage;
            return ClientCoreLoader.Speak(m_cs->GetHandle(), su->GetBuffer(), su->GetSize(), pGroups, ulGroupCount);
        }

        bool CClientSocket::CPushImpl::SendUserMessage(const UVariant& vtMessage, const wchar_t * strUserId) const {
            CScopeUQueue su;
            su << vtMessage;
            return ClientCoreLoader.SendUserMessage(m_cs->GetHandle(), strUserId, su->GetBuffer(), su->GetSize());
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
            char ipAddr[256] =
            {0};
            bool ok = ClientCoreLoader.GetPeerName(m_hSocket, port, ipAddr, sizeof (ipAddr));
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

        bool CClientSocket::operator == (const CClientSocket & cs) const {
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
            return ClientCoreLoader.StartQueue(m_hSocket, qName, secure, dequeueShared, ttl);
        }

        void CClientSocket::CQueueImpl::StopQueue(bool permanent) {
            ClientCoreLoader.StopQueue(m_hSocket, permanent);
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
#ifdef WIN32_64
            tm.tm_isdst = 1;
#else
            tm.tm_isdst = -1;
#endif
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
            if (GetQueueOpenStatus() != SPA::qsMergePushing)
                return true;
            if (!handles || !count)
                return true; //don't do anything in case there is no target queue available
            std::vector<USocket_Client_Handle> vHandles;
            for (n = 0; n < count; ++n) {
                if (ClientCoreLoader.GetClientQueueStatus(handles[n]) != qsMergeComplete)
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
            CAutoLock al(ash->m_cs);
            size_t aborted = ash->m_vCallback.size() - m_nQIndex;
            if (ClientCoreLoader.AbortJob(m_hSocket)) {
                ash->EraseBack(aborted);
                return true;
            }
            return false;
        }

        bool CClientSocket::CQueueImpl::StartJob() const {
            CAsyncServiceHandler *ash = CClientSocket::Seek(m_hSocket)->GetCurrentHandler();
            CAutoLock al(ash->m_cs);
            (const_cast<CQueueImpl*> (this))->m_nQIndex = ash->m_vCallback.size();
            return ClientCoreLoader.StartJob(m_hSocket);
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

        unsigned int CClientSocket::GetCurrentServiceID() const {
            return m_nCurrSvsId;
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

        bool CClientSocket::IsRouting() const {
            return m_routing;
        }

        std::string CClientSocket::GetErrorMsg() const {
            char strErrorMsg[1025] =
            {0};
            ClientCoreLoader.GetErrorMessage(m_hSocket, strErrorMsg, sizeof (strErrorMsg));
            return strErrorMsg;
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
            if (p->SocketClosed)
                p->SocketClosed(p, nError);
            p->OnSocketClosed(nError);
        }

        void WINAPI CClientSocket::OnHandShakeCompleted(USocket_Client_Handle handler, int nError) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            if (p->HandShakeCompleted)
                p->HandShakeCompleted(p, nError);
            p->OnHandShakeCompleted(nError);
        }

        void WINAPI CClientSocket::OnSocketConnected(USocket_Client_Handle handler, int nError) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            if (!p->GetClientQueue().IsAvailable()) {
                CAsyncServiceHandler *ash = p->GetCurrentHandler();
                if (ash)
                    ash->CleanCallbacks();
            }
            if (p->SocketConnected)
                p->SocketConnected(p, nError);
            p->OnSocketConnected(nError);
        }

        void WINAPI CClientSocket::OnRequestProcessed(USocket_Client_Handle handler, unsigned short requestId, unsigned int len) {
            CClientSocket *p = Seek(handler);
            if (p) {
                if (p->IsRouting())
                    p->m_os = ClientCoreLoader.GetPeerOs(handler, &p->m_endian);
                SPA::CScopeUQueue sb(p->m_os, p->m_endian);
                if (len > sb->GetMaxSize())
                    sb->ReallocBuffer(len + 16);
                if (len) {
                    unsigned int get = ClientCoreLoader.RetrieveResult(p->m_hSocket, (unsigned char*) sb->GetBuffer(), len);
                    if (get == len)
                        sb->SetSize(get);
                    else if (get == 0)
                        return; //disconnected
                    else {
                        assert(false); //should never come here!
                    }
                }
                CAsyncServiceHandler *ash = p->Seek(p->m_nCurrSvsId);
                if (ash)
                    ash->OnRR(requestId, *sb);
                if (p->RequestProcessed)
                    p->RequestProcessed(p, requestId, *sb);
                p->OnRequestProcessed(requestId, *sb);
            }
        }

        void WINAPI CClientSocket::OnSubscribe(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            CPushImpl &push = p->GetPush();
            if (push.OnSubscribe)
                push.OnSubscribe(p, sender, pGroup, count);
            p->OnSubscribe(sender, pGroup, count);
        }

        void WINAPI CClientSocket::OnUnsubscribe(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            CPushImpl &push = p->GetPush();
            if (push.OnUnsubscribe)
                push.OnUnsubscribe(p, sender, pGroup, count);
            p->OnUnsubscribe(sender, pGroup, count);
        }

        void WINAPI WINAPI CClientSocket::OnBroadcast(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            SPA::UVariant vtMessage;
            CScopeUQueue su;
            su->Push(pMessage, size);
            su >> vtMessage;
            CPushImpl &push = p->GetPush();
            if (push.OnPublish)
                push.OnPublish(p, sender, pGroup, count, vtMessage);
            p->OnPublish(sender, pGroup, count, vtMessage);
        }

        void WINAPI CClientSocket::OnBroadcastEx(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            CPushImpl &push = p->GetPush();
            if (push.OnPublishEx) {
#if defined(WIN32_64) && _MSC_VER < 1800
                //Visual C++ has implementation limitation of std::function on the number of parameters -- temporary solution
                push.OnPublishEx(sender, pGroup, count, pMessage, size);
#else
                push.OnPublishEx(p, sender, pGroup, count, pMessage, size);
#endif
            }
            p->OnPublishEx(sender, pGroup, count, pMessage, size);
        }

        void WINAPI CClientSocket::OnPostUserMessage(USocket_Client_Handle handler, CMessageSender sender, const unsigned char *pMessage, unsigned int size) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            SPA::UVariant vtMessage;
            CScopeUQueue su;
            su->Push(pMessage, size);
            su >> vtMessage;
            CPushImpl &push = p->GetPush();
            if (push.OnSendUserMessage)
                push.OnSendUserMessage(p, sender, vtMessage);
            p->OnSendUserMessage(sender, vtMessage);
        }

        void WINAPI CClientSocket::OnPostUserMessageEx(USocket_Client_Handle handler, CMessageSender sender, const unsigned char *pMessage, unsigned int size) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            CPushImpl &push = p->GetPush();
            if (push.OnSendUserMessageEx)
                push.OnSendUserMessageEx(p, sender, pMessage, size);
            p->OnSendUserMessageEx(sender, pMessage, size);
        }

        void CClientSocket::OnAllRequestsProcessed(unsigned short lastRequestId) {

        }

        void WINAPI CClientSocket::OnAllRequestsProcessed(USocket_Client_Handle handler, unsigned short lastRequestId) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            if (p->AllRequestsProcessed)
                p->AllRequestsProcessed(p, lastRequestId);
            p->OnAllRequestsProcessed(lastRequestId);
        }

        void WINAPI CClientSocket::OnBaseRequestProcessed(USocket_Client_Handle handler, unsigned short requestId) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            if (requestId == SPA::idCancel) {
                CAsyncServiceHandler *ash = p->Seek(ClientCoreLoader.GetCurrentServiceId(handler));
                if (ash)
                    ash->CleanCallbacks();
            } else if (requestId == SPA::idSwitchTo) {
                p->m_bRandom = ClientCoreLoader.IsRandom(handler);
                p->m_os = ClientCoreLoader.GetPeerOs(handler, &p->m_endian);
                p->m_nCurrSvsId = ClientCoreLoader.GetCurrentServiceId(handler);
                p->m_routing = ClientCoreLoader.IsRouting(handler);
            }
            if (p->BaseRequestProcessed)
                p->BaseRequestProcessed(p, requestId);
            p->OnBaseRequestProcessed(requestId);
        }

        void WINAPI CClientSocket::OnServerException(USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) {
            CClientSocket *p = Seek(handler);
            if (!p)
                return;
            if (p->ExceptionFromServer) {
#if defined(WIN32_64) && _MSC_VER < 1800
                //Visual C++ has implementation limitation of std::function on the number of parameters -- temporary solution
                p->ExceptionFromServer(p, errMessage, errWhere, errCode);
#else
                p->ExceptionFromServer(p, requestId, errMessage, errWhere, errCode);
#endif
            }
            CAsyncServiceHandler *ash = p->Seek(ClientCoreLoader.GetCurrentServiceId(handler));
            if (ash)
                ash->OnSE(requestId, errMessage, errWhere, errCode);
            p->OnExceptionFromServer(requestId, errMessage, errWhere, errCode);
        }
    }//ClientSide
}//SPA