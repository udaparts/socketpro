

#include "scloader.h"
#include "aserverw.h"
#include <algorithm>

namespace SPA
{
    namespace ServerSide{

        Internal::CServerCoreLoader ServerCoreLoader;

        CSocketProServer * CSocketProServer::m_pServer = nullptr;

        CSocketPeer::CSocketPeer() : m_hHandler(0), m_pBase(nullptr), m_UQueue(*m_sb), m_bRandom(false) {

        }

        CSocketPeer::~CSocketPeer() {

        }

        CHttpPeerBase::CHttpPeerBase() : m_bHttpOk(false) {

        }

        CHttpPeerBase::~CHttpPeerBase() {

        }

        IPush & CHttpPeerBase::GetPush() {
            return m_PushImpl;
        }

        const std::string & CHttpPeerBase::GetUserRequestName() const {
            return m_WebRequestName;
        }

        const std::vector<UVariant>& CHttpPeerBase::GetArgs() const {
            return m_vArg;
        }

        bool CHttpPeerBase::IsAuthenticated() const {
            return m_bHttpOk;
        }

        bool CHttpPeerBase::SetResponseCode(unsigned int errCode) const {
            return ServerCoreLoader.SetHTTPResponseCode(GetSocketHandle(), errCode);
        }

        bool CHttpPeerBase::SetResponseHeader(const char *uft8Header, const char *utf8Value) const {
            return ServerCoreLoader.SetHTTPResponseHeader(GetSocketHandle(), uft8Header, utf8Value);
        }

        unsigned int CHttpPeerBase::GetRequestHeaders(CHttpHeaderValue *HeaderValue, unsigned int count) const {
            return ServerCoreLoader.GetHTTPRequestHeaders(GetSocketHandle(), HeaderValue, count);
        }

        unsigned int CHttpPeerBase::GetCurrentMultiplaxHeaders(CHttpHeaderValue *HeaderValue, unsigned int count) const {
            return ServerCoreLoader.GetHTTPCurrentMultiplaxHeaders(GetSocketHandle(), HeaderValue, count);
        }

        unsigned int CHttpPeerBase::StartChunkResponse() const {
            return ServerCoreLoader.StartHTTPChunkResponse(GetSocketHandle());
        }

        unsigned int CHttpPeerBase::SendChunk(const unsigned char *buffer, unsigned int len) const {
            return ServerCoreLoader.SendHTTPChunk(GetSocketHandle(), buffer, len);
        }

        unsigned int CHttpPeerBase::EndChunkResponse(const unsigned char *buffer, unsigned int len) const {
            return ServerCoreLoader.EndHTTPChunkResponse(GetSocketHandle(), buffer, len);
        }

        const char* CHttpPeerBase::GetId() const {
            return ServerCoreLoader.GetHTTPId(GetSocketHandle());
        }

        const char* CHttpPeerBase::GetPath() const {
            return ServerCoreLoader.GetHTTPPath(GetSocketHandle());
        }

        UINT64 CHttpPeerBase::GetContentLength() const {
            return ServerCoreLoader.GetHTTPContentLength(GetSocketHandle());
        }

        const char* CHttpPeerBase::GetQuery() const {
            return ServerCoreLoader.GetHTTPQuery(GetSocketHandle());
        }

        bool CHttpPeerBase::DownloadFile(const char *filePath) const {
            return ServerCoreLoader.DownloadFile(GetSocketHandle(), filePath);
        }

        tagHttpMethod CHttpPeerBase::GetMethod() const {
            return ServerCoreLoader.GetHTTPMethod(GetSocketHandle());
        }

        bool CHttpPeerBase::IsWebSocket() const {
            return ServerCoreLoader.IsWebSocket(GetSocketHandle());
        }

        bool CHttpPeerBase::IsCrossDomain() const {
            return ServerCoreLoader.IsCrossDomain(GetSocketHandle());
        }

        double CHttpPeerBase::GetVersion() const {
            return ServerCoreLoader.GetHTTPVersion(GetSocketHandle());
        }

        const char* CHttpPeerBase::GetHost() const {
            return ServerCoreLoader.GetHTTPHost(GetSocketHandle());
        }

        tagTransport CHttpPeerBase::GetTransport() const {
            return ServerCoreLoader.GetHTTPTransport(GetSocketHandle());
        }

        tagTransferEncoding CHttpPeerBase::GetTransferEncoding() const {
            return ServerCoreLoader.GetHTTPTransferEncoding(GetSocketHandle());
        }

        tagContentMultiplax CHttpPeerBase::GetContentMultiplax() const {
            return ServerCoreLoader.GetHTTPContentMultiplax(GetSocketHandle());
        }

        CClientPeer::CClientPeer() {

        }

        CClientPeer::~CClientPeer() {

        }

        const CBaseService * CSocketPeer::GetBaseService() const {
            return m_pBase;
        }

        USocket_Server_Handle CSocketPeer::GetSocketHandle() const {
            CAutoLock sl(CBaseService::m_mutex);
            return m_hHandler;
        }

        unsigned short CSocketPeer::GetCurrentRequestID() const {
            return ServerCoreLoader.GetCurrentRequestID(m_hHandler);
        }

        bool CSocketPeer::IsMainThread() {
            return ServerCoreLoader.IsMainThread();
        }

        UINT64 CSocketPeer::GetRequestCount() {
            return ServerCoreLoader.GetRequestCount();
        }

        unsigned int CSocketPeer::GetCurrentRequestLen() const {
            return ServerCoreLoader.GetCurrentRequestLen(m_hHandler);
        }

        unsigned int CSocketPeer::GetRcvBytesInQueue() const {
            return ServerCoreLoader.GetRcvBytesInQueue(m_hHandler);
        }

        unsigned int CSocketPeer::GetSndBytesInQueue() const {
            return ServerCoreLoader.GetSndBytesInQueue(m_hHandler);
        }

        void CSocketPeer::PostClose(int errCode) const {
            ServerCoreLoader.PostClose(m_hHandler, errCode);
        }

        void CSocketPeer::Close() const {
            ServerCoreLoader.Close(m_hHandler);
        }

        bool CSocketPeer::IsOpened() const {
            return ServerCoreLoader.IsOpened(m_hHandler);
        }

        UINT64 CSocketPeer::GetBytesReceived() const {
            return ServerCoreLoader.GetBytesReceived(m_hHandler);
        }

        UINT64 CSocketPeer::GetBytesSent() const {
            return ServerCoreLoader.GetBytesSent(m_hHandler);
        }

        bool CHttpPeerBase::CPushImpl::Subscribe(const unsigned int *pChatGroupId, unsigned int count) const {
            return ServerCoreLoader.Enter(m_hSocket, pChatGroupId, count);
        }

        void CHttpPeerBase::CPushImpl::Unsubscribe() const {
            ServerCoreLoader.Exit(m_hSocket);
        }

        bool CHttpPeerBase::CPushImpl::Publish(const UVariant& vtMessage, const unsigned int *pGroups, unsigned int ulGroupCount) const {
            if (pGroups == nullptr || ulGroupCount == 0)
                return false;
            CScopeUQueue su;
            su << vtMessage;
            return ServerCoreLoader.Speak(m_hSocket, su->GetBuffer(), su->GetSize(), pGroups, ulGroupCount);
        }

        bool CHttpPeerBase::CPushImpl::SendUserMessage(const UVariant& vtMessage, const wchar_t * strUserId) const {
            CScopeUQueue su;
            su << vtMessage;
            return ServerCoreLoader.SendUserMessage(m_hSocket, strUserId, su->GetBuffer(), su->GetSize());
        }

        bool CClientPeer::CPushImpl::Subscribe(const unsigned int *pChatGroupId, unsigned int count) const {
            return ServerCoreLoader.Enter(m_hSocket, pChatGroupId, count);
        }

        void CClientPeer::CPushImpl::Unsubscribe() const {
            ServerCoreLoader.Exit(m_hSocket);
        }

        bool CClientPeer::CPushImpl::Publish(const UVariant& vtMessage, const unsigned int *pGroups, unsigned int ulGroupCount) const {
            if (pGroups == nullptr || ulGroupCount == 0)
                return false;
            CScopeUQueue su;
            su << vtMessage;
            return ServerCoreLoader.Speak(m_hSocket, su->GetBuffer(), su->GetSize(), pGroups, ulGroupCount);
        }

        bool CClientPeer::CPushImpl::SendUserMessage(const UVariant& vtMessage, const wchar_t * strUserId) const {
            CScopeUQueue su;
            su << vtMessage;
            return ServerCoreLoader.SendUserMessage(m_hSocket, strUserId, su->GetBuffer(), su->GetSize());
        }

        bool CClientPeer::CPushImpl::PublishEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count) const {
            return ServerCoreLoader.SpeakEx(m_hSocket, message, size, pChatGroupId, count);
        }

        bool CClientPeer::CPushImpl::SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size) const {
            return ServerCoreLoader.SendUserMessageEx(m_hSocket, userId, message, size);
        }

        unsigned int CSocketPeer::GetSvsID() const {
            return m_pBase->GetSvsID();
        }

        int CSocketPeer::GetErrorCode() const {
            return ServerCoreLoader.GetServerSocketErrorCode(m_hHandler);
        }

        std::string CSocketPeer::GetErrorMessage() const {
            char strErr[2045];
            ServerCoreLoader.GetServerSocketErrorMessage(m_hHandler, strErr, sizeof (strErr));
            return strErr;
        }

        std::string CSocketPeer::GetPeerName(unsigned int *port) const {
            char strIpAddr[128] =
            {0};
            ServerCoreLoader.GetPeerName(m_hHandler, port, strIpAddr, sizeof (strIpAddr));
            return strIpAddr;
        }

        bool CSocketPeer::IsBatching() const {
            return ServerCoreLoader.IsBatching(m_hHandler);
        }

        unsigned int CClientPeer::GetBytesBatched() const {
            return ServerCoreLoader.GetBytesBatched(GetSocketHandle());
        }

        bool CClientPeer::IsDequeueRequest() const {
            return ServerCoreLoader.IsDequeueRequest(GetSocketHandle());
        }

        void CClientPeer::AbortDequeuedMessage() const {
            ServerCoreLoader.AbortDequeuedMessage(GetSocketHandle());
        }

        bool CClientPeer::IsDequeuedMessageAborted() const {
            return ServerCoreLoader.IsDequeuedMessageAborted(GetSocketHandle());
        }

        bool CClientPeer::StartBatching() const {
            return ServerCoreLoader.StartBatching(GetSocketHandle());
        }

        void CClientPeer::EnableClientDequeue(bool enable) const {
            ServerCoreLoader.EnableClientDequeue(GetSocketHandle(), enable);
        }

        bool CClientPeer::CommitBatching() const {
            return ServerCoreLoader.CommitBatching(GetSocketHandle());
        }

        UINT64 CClientPeer::Dequeue(unsigned int qHandle, unsigned int messageCount, bool beNotifiedWhenAvailable, unsigned int waitTime) const {
            return ServerCoreLoader.Dequeue(qHandle, GetSocketHandle(), messageCount, beNotifiedWhenAvailable, waitTime);
        }

        UINT64 CClientPeer::Dequeue(unsigned int qHandle, bool beNotifiedWhenAvailable, unsigned int maxBytes, unsigned int waitTime) const {
            return ServerCoreLoader.Dequeue2(qHandle, GetSocketHandle(), maxBytes, beNotifiedWhenAvailable, waitTime);
        }

        bool CClientPeer::AbortBatching() const {
            return ServerCoreLoader.AbortBatching(GetSocketHandle());
        }

        bool CClientPeer::SetZip(bool bZip) const {
            return ServerCoreLoader.SetZip(GetSocketHandle(), bZip);
        }

        bool CClientPeer::GetZip() const {
            return ServerCoreLoader.GetZip(GetSocketHandle());
        }

        void CClientPeer::SetZipLevel(tagZipLevel zl) const {
            ServerCoreLoader.SetZipLevel(GetSocketHandle(), zl);
        }

        tagZipLevel CClientPeer::GetZipLevel() const {
            return ServerCoreLoader.GetZipLevel(GetSocketHandle());
        }

        void CSocketPeer::SetUserID(const wchar_t * userId) const {
            ServerCoreLoader.SetUserID(m_hHandler, userId);
        }

        void CSocketPeer::SetUserID(const std::wstring & userId) const {
            ServerCoreLoader.SetUserID(m_hHandler, userId.data());
        }

        IPushEx & CClientPeer::GetPush() {
            return m_PushImpl;
        }

        bool CSocketPeer::MakeRequest(unsigned short requestId, const unsigned char *request, unsigned int size) const {
            return ServerCoreLoader.MakeRequest(m_hHandler, requestId, request, size);
        }

        bool CSocketPeer::IsFakeRequest() const {
            return ServerCoreLoader.IsFakeRequest(m_hHandler);
        }

        bool CSocketPeer::IsCanceled() const {
            return ServerCoreLoader.IsCanceled(m_hHandler);
        }

        std::wstring CSocketPeer::GetUID() const {
            wchar_t str[MAX_USERID_CHARS + 1];
            ServerCoreLoader.GetUID(m_hHandler, str, sizeof (str) / sizeof (wchar_t));
            return str;
        }

        unsigned int CSocketPeer::QueryRequestsInQueue() const {
            return ServerCoreLoader.QueryRequestsInQueue(m_hHandler);
        }

        unsigned int CSocketPeer::GetCountOfJoinedChatGroups() const {
            return ServerCoreLoader.GetCountOfJoinedChatGroups(m_hHandler);
        }

        UINT64 CSocketPeer::GetCurrentRequestIndex() const {
            return ServerCoreLoader.GetCurrentRequestIndex(m_hHandler);
        }

        std::vector<unsigned int> CSocketPeer::GetChatGroups() const {
            CScopeUQueue sb;
            std::vector<unsigned int> vChatGroups;
            unsigned int size = (ServerCoreLoader.GetCountOfJoinedChatGroups(m_hHandler) + 10) * sizeof (unsigned int);
            if (sb->GetMaxSize() < size)
                sb->ReallocBuffer(size);
            unsigned int *p = (unsigned int*) sb->GetBuffer();
            size = ServerCoreLoader.GetJoinedGroupIds(m_hHandler, p, size / sizeof (unsigned int));
            vChatGroups.assign(p, p + size);
            return vChatGroups;
        }

        void* CSocketPeer::GetSSL() const {
            return ServerCoreLoader.GetSSL(m_hHandler);
        }

        bool CBaseService::GetReturnRandom() const {
            return ServerCoreLoader.GetReturnRandom(m_nServiceId);
        }

        void CBaseService::SetReturnRandom(bool random) const {
            ServerCoreLoader.SetReturnRandom(m_nServiceId, random);
        }

        void CSocketPeer::DropCurrentSlowRequest() const {
            ServerCoreLoader.DropCurrentSlowRequest(m_hHandler);
        }

        tagOperationSystem CClientPeer::GetPeerOs(bool * endian) const {
            return ServerCoreLoader.GetPeerOs(GetSocketHandle(), endian);
        }

        unsigned int CSocketPeer::SendExceptionResult(const wchar_t* errMessage, const char* errWhere, unsigned int errCode, unsigned short requestId, UINT64 index) const {
            if (index == INVALID_NUMBER)
                return ServerCoreLoader.SendExceptionResult(m_hHandler, errMessage, errWhere, requestId, errCode);
            return ServerCoreLoader.SendExceptionResultIndex(m_hHandler, index, errMessage, errWhere, requestId, errCode);
        }

        unsigned int CSocketPeer::SendExceptionResult(const char* errMessage, const char* errWhere, unsigned int errCode, unsigned short requestId, UINT64 index) const {
            CScopeUQueue q;
            Utilities::ToWide(errMessage, ::strlen(errMessage), *q);
            return SendExceptionResult((const wchar_t*) q->GetBuffer(), errWhere, errCode, requestId, index);
        }

        UINT64 CSocketPeer::GetSocketNativeHandle() const {
            return ServerCoreLoader.GetSocketNativeHandle(m_hHandler);
        }

        void CSocketPeer::OnReleaseSource(bool bClosing, unsigned int info) {

        }

        void CSocketPeer::OnSwitchFrom(unsigned int nServiceId) {

        }

        void CSocketPeer::OnSubscribe(const unsigned int *pGroup, unsigned int count) {

        }

        void CSocketPeer::OnUnsubscribe(const unsigned int *pGroup, unsigned int count) {

        }

        void CSocketPeer::OnPublish(const UVariant& vtMessage, const unsigned int *pGroup, unsigned int count) {

        }

        void CClientPeer::OnPublishEx(const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size) {

        }

        void CClientPeer::OnSendUserMessageEx(const wchar_t* receiver, const unsigned char *pMessage, unsigned int size) {

        }

        void CSocketPeer::OnSendUserMessage(const wchar_t* receiver, const UVariant & vtMessage) {

        }

        void CSocketPeer::OnChatRequestCame(tagChatRequestID chatRequestId) {

        }

        void CSocketPeer::OnRequestArrive(unsigned short requestId, unsigned int len) {

        }

        void CSocketPeer::OnSlowRequestProcessed(unsigned short requestId) {

        }

        void CSocketPeer::OnResultsSent() {

        }

        void CSocketPeer::OnBaseRequestArrive(unsigned short requestId) {

        }

        bool CHttpPeerBase::DoAuthentication(const wchar_t *userId, const wchar_t * password) {
            return true;
        }

        unsigned int CHttpPeerBase::SendResult(const char *utf8, unsigned int chars) const {
            return ServerCoreLoader.SendHTTPReturnDataA(GetSocketHandle(), utf8, chars);
        }

        unsigned int CHttpPeerBase::SendResult(const wchar_t *str, unsigned int chars) const {
            return ServerCoreLoader.SendHTTPReturnDataW(GetSocketHandle(), str, chars);
        }

        unsigned int CClientPeer::SendResult(unsigned short reqId, const unsigned char* pResult, unsigned int size) const {
            if (!m_bRandom)
                return ServerCoreLoader.SendReturnData(GetSocketHandle(), reqId, size, pResult);
            return ServerCoreLoader.SendReturnDataIndex(GetSocketHandle(), GetCurrentRequestIndex(), reqId, size, pResult);
        }

        unsigned int CClientPeer::SendResult(unsigned short reqId, const CUQueue & mc) const {
            return SendResult(reqId, mc.GetBuffer(), mc.GetSize());
        }

        unsigned int CClientPeer::SendResult(unsigned short reqId, const CScopeUQueue & sb) const {
            return SendResult(reqId, sb->GetBuffer(), sb->GetSize());
        }

        unsigned int CClientPeer::SendResult(unsigned short reqId) const {
            return SendResult(reqId, (const unsigned char*) nullptr, (unsigned int) 0);
        }

        unsigned int CClientPeer::SendResultIndex(UINT64 callIndex, unsigned short reqId, const unsigned char* pResult, unsigned int size) const {
            return ServerCoreLoader.SendReturnDataIndex(GetSocketHandle(), callIndex, reqId, size, pResult);
        }

        unsigned int CClientPeer::SendResultIndex(UINT64 callIndex, unsigned short reqId, const CUQueue & mc) const {
            return SendResultIndex(callIndex, reqId, mc.GetBuffer(), mc.GetSize());
        }

        unsigned int CClientPeer::SendResultIndex(UINT64 callIndex, unsigned short reqId, const CScopeUQueue & sb) const {
            return SendResultIndex(callIndex, reqId, sb->GetBuffer(), sb->GetSize());
        }

        unsigned int CClientPeer::SendResultIndex(UINT64 callIndex, unsigned short reqId) const {
            return SendResultIndex(callIndex, reqId, (const unsigned char*) nullptr, (unsigned int) 0);
        }

        unsigned int CBaseService::m_nMainThreads = (~0);
        CUCriticalSection CBaseService::m_mutex;
        std::vector<CBaseService*> CBaseService::m_vService;

        CBaseService::CBaseService(unsigned int svsId, tagThreadApartment ta)
        : m_nServiceId(svsId) {
            ServerCoreLoader.Load();
            if (!ServerCoreLoader.IsLoaded())
                throw CUExCode("Server core library not accessible!", MB_BAD_OPERATION);
            m_SvsContext.m_OnChatRequestCame = &CBaseService::OnChatCame;
            m_SvsContext.m_OnChatRequestComing = &CBaseService::OnChatComing;
            m_SvsContext.m_OnBaseRequestCame = &CBaseService::OnBaseCame;
            m_SvsContext.m_OnRequestArrive = &CBaseService::OnReqArrive;
            m_SvsContext.m_OnClose = &CBaseService::OnClose;
            m_SvsContext.m_OnFastRequestArrive = &CBaseService::OnFast;
            m_SvsContext.m_OnRequestProcessed = &CBaseService::OnSlowRequestProcessed;
            m_SvsContext.m_OnSwitchTo = &CBaseService::OnSwitch;
            m_SvsContext.m_SlowProcess = &CBaseService::OnSlow;
            m_SvsContext.m_OnHttpAuthentication = &CBaseService::OnHttpAuthentication;
            m_SvsContext.m_OnResultsSent = &CBaseService::OnResultsSent;
            m_SvsContext.m_ta = ta;
            CAutoLock sl(m_mutex);
            m_vService.push_back(this);
        }

        CBaseService::~CBaseService() {
            Clean();
            RemoveMe();
        }

        void CALLBACK CBaseService::OnReqArrive(USocket_Server_Handle hSocket, unsigned short usRequestID, unsigned int len) {
            assert(ServerCoreLoader.IsMainThread());
            unsigned int ServiceId = ServerCoreLoader.GetSvsID(hSocket);
            const CBaseService *pService = SeekService(ServiceId);
            if (!pService)
                return;
            CSocketPeer *p = pService->Seek(hSocket);
            if (p) {
                CUQueue &mc = p->m_UQueue;
                mc.SetSize(0);
                if (ServiceId != sidHTTP) {
                    bool endian;
                    tagOperationSystem os = ServerCoreLoader.GetPeerOs(hSocket, &endian);
                    mc.SetOS(os);
                    mc.SetEndian(endian);
                }
                if (len > mc.GetMaxSize())
                    mc.ReallocBuffer(len + 16);
                if (len) {
                    if (m_nMainThreads <= 1) {
                        const unsigned char *bytes = ServerCoreLoader.GetRequestBuffer(hSocket);
                        mc.Push(bytes, len);
                    } else {
                        len = ServerCoreLoader.RetrieveBuffer(hSocket, len, (unsigned char*) mc.GetBuffer(), false);
                        mc.SetSize(len);
                    }
                }
                if (ServiceId == sidHTTP) {
                    CHttpPeerBase *pHttpPerr = (CHttpPeerBase*) p;
                    if (usRequestID == idUserRequest) {
                        unsigned int count;
                        mc >> pHttpPerr->m_WebRequestName;
                        mc >> count;
                        pHttpPerr->m_vArg.resize(count);
                        for (unsigned int n = 0; n < count; ++n) {
                            mc >> pHttpPerr->m_vArg[n];
                        }
                    } else {
                        pHttpPerr->m_WebRequestName.clear();
                        pHttpPerr->m_vArg.clear();
                    }
                }
                p->OnRequestArrive(usRequestID, len);
            }
        }

        void CALLBACK CBaseService::OnFast(USocket_Server_Handle hSocket, unsigned short usRequestID, unsigned int len) {
            assert(ServerCoreLoader.IsMainThread());
            unsigned int ServiceId = ServerCoreLoader.GetSvsID(hSocket);
            const CBaseService *pService = SeekService(ServiceId);
            if (!pService)
                return;
            CSocketPeer *p = pService->Seek(hSocket);
            if (p) {
                len = p->m_UQueue.GetSize();
                try{
                    p->OnFastRequestArrive(usRequestID, len);
                }

                catch(const CUException & err) {
                    p->SendExceptionResult(err.what(), err.GetStack().c_str(), (unsigned int) err.GetErrCode(), usRequestID, p->GetCurrentRequestIndex());
                }

                catch(const std::exception & err) {
                    p->SendExceptionResult(err.what(), __FUNCTION__, MB_STL_EXCEPTION, usRequestID, p->GetCurrentRequestIndex());
                }

                catch(...) {
                    p->SendExceptionResult(L"Unknown exception caught", __FUNCTION__, MB_UNKNOWN_EXCEPTION, usRequestID, p->GetCurrentRequestIndex());
                }
            }
        }

        void CALLBACK CBaseService::OnSwitch(USocket_Server_Handle hSocket, unsigned int oldServiceId, unsigned int newServiceId) {
            const CBaseService *p;
            if ((unsigned int) (~0) == m_nMainThreads) {
                m_nMainThreads = CSocketProServer::Config::GetMainThreads();
            }
            assert(ServerCoreLoader.IsMainThread());
            if (oldServiceId != sidStartup && (p = SeekService(oldServiceId)) != nullptr) {
                ((CBaseService*) p)->ReleasePeer(hSocket, false, newServiceId);
            }
            p = SeekService(newServiceId);
            if (p) {
                CSocketPeer *pPeer = ((CBaseService*) p)->CreatePeer(hSocket, newServiceId);
                pPeer->m_bRandom = p->GetReturnRandom();
                if (newServiceId == sidHTTP) {
                    CHttpPeerBase *pCHttpPeer = (CHttpPeerBase*) pPeer;
                    pCHttpPeer->m_PushImpl.m_hSocket = hSocket;
                    pCHttpPeer->m_bHttpOk = false;
                } else {
                    bool endian;
                    CClientPeer *pUClientPeer = (CClientPeer*) pPeer;
                    pUClientPeer->m_PushImpl.m_hSocket = hSocket;
                    tagOperationSystem os = ServerCoreLoader.GetPeerOs(hSocket, &endian);
                    pUClientPeer->m_UQueue.SetEndian(endian);
                    pUClientPeer->m_UQueue.SetOS(os);
                }
                pPeer->OnSwitchFrom(oldServiceId);
            }
        }

        int CALLBACK CBaseService::OnSlow(unsigned short usRequestID, unsigned int len, USocket_Server_Handle hSocket) {
            int res = 0;
            assert(!ServerCoreLoader.IsMainThread());
            unsigned int ServiceId = ServerCoreLoader.GetSvsID(hSocket);
            const CBaseService *pService = SeekService(ServiceId);
            if (!pService)
                return res;
            CSocketPeer *p = pService->Seek(hSocket);
            if (p) {
                len = p->m_UQueue.GetSize();
                try{
                    res = p->OnSlowRequestArrive(usRequestID, len);
                }

                catch(const SPA::CUException & err) {
                    p->SendExceptionResult(err.what(), err.GetStack().c_str(), (unsigned int) err.GetErrCode(), usRequestID, p->GetCurrentRequestIndex());
                }

                catch(const std::exception & err) {
                    p->SendExceptionResult(err.what(), __FUNCTION__, MB_STL_EXCEPTION, usRequestID, p->GetCurrentRequestIndex());
                }

                catch(...) {
                    p->SendExceptionResult(L"Unknown exception caught", __FUNCTION__, MB_UNKNOWN_EXCEPTION, usRequestID, p->GetCurrentRequestIndex());
                }
            }
            return res;
        }

        void CALLBACK CBaseService::OnClose(USocket_Server_Handle hSocket, int nError) {
            assert(ServerCoreLoader.IsMainThread());
            unsigned int ServiceId = ServerCoreLoader.GetSvsID(hSocket);
            CBaseService *pService = SeekService(ServiceId);
            if (!pService)
                return;
            CSocketPeer *p = pService->Seek(hSocket);
            if (p) {
                pService->ReleasePeer(hSocket, true, (unsigned int) nError);
            }
        }

        void CALLBACK CBaseService::OnBaseCame(USocket_Server_Handle hSocket, unsigned short usRequestID) {
            assert(ServerCoreLoader.IsMainThread());
            unsigned int ServiceId = ServerCoreLoader.GetSvsID(hSocket);
            const CBaseService *pService = SeekService(ServiceId);
            if (!pService)
                return;
            CSocketPeer *p = pService->Seek(hSocket);
            if (p) {
                if (usRequestID == idPing) {
                    if (p->m_UQueue.GetSize() == 0 && p->m_UQueue.GetMaxSize() > 4 * DEFAULT_INITIAL_MEMORY_BUFFER_SIZE) {
                        p->m_UQueue.ReallocBuffer(DEFAULT_INITIAL_MEMORY_BUFFER_SIZE);
                    }
                    CHttpPeerBase *pHttp = dynamic_cast<CHttpPeerBase *> (p);
                    if (pHttp) {
                        pHttp->m_vArg.clear();
                        pHttp->m_WebRequestName.clear();
                    }
                }
                p->OnBaseRequestArrive(usRequestID);
            }
        }

        void CALLBACK CBaseService::OnResultsSent(USocket_Server_Handle handler) {
            assert(ServerCoreLoader.IsMainThread());
            unsigned int ServiceId = ServerCoreLoader.GetSvsID(handler);
            const CBaseService *pService = SeekService(ServiceId);
            if (!pService)
                return;
            CSocketPeer *p = pService->Seek(handler);
            if (p)
                p->OnResultsSent();
        }

        bool CALLBACK CBaseService::OnHttpAuthentication(USocket_Server_Handle h, const wchar_t *userId, const wchar_t * password) {
            assert(ServerCoreLoader.IsMainThread());
            const CBaseService *pService = SeekService((unsigned int) sidHTTP);
            if (!pService)
                return false;
            CSocketPeer *p = pService->Seek(h);
            if (p) {
                CHttpPeerBase *pHttp = (CHttpPeerBase *) p;
                pHttp->m_bHttpOk = pHttp->DoAuthentication(userId, password);
                return pHttp->m_bHttpOk;
            }
            return false;
        }

        void CALLBACK CBaseService::OnSlowRequestProcessed(USocket_Server_Handle hSocket, unsigned short usRequestID) {
            assert(ServerCoreLoader.IsMainThread());
            unsigned int ServiceId = ServerCoreLoader.GetSvsID(hSocket);
            const CBaseService *pService = SeekService(ServiceId);
            if (!pService)
                return;
            CSocketPeer *p = pService->Seek(hSocket);
            if (p)
                p->OnSlowRequestProcessed(usRequestID);
        }

        void CALLBACK CBaseService::OnChatComing(USocket_Server_Handle handler, tagChatRequestID chatRequestID, unsigned int len) {
            assert(ServerCoreLoader.IsMainThread());
            unsigned int ServiceId = ServerCoreLoader.GetSvsID(handler);
            const CBaseService *pService = SeekService(ServiceId);
            if (!pService)
                return;
            CSocketPeer *p = pService->Seek(handler);
            if (p) {
                CUQueue &mc = p->m_UQueue;
                mc.SetSize(0);
                if (ServiceId != sidHTTP) {
                    bool endian;
                    tagOperationSystem os = ServerCoreLoader.GetPeerOs(handler, &endian);
                    mc.SetEndian(endian);
                    mc.SetOS(os);
                }

                if (len > mc.GetMaxSize())
                    mc.ReallocBuffer(len + 16);
                if (len) {
                    len = ServerCoreLoader.RetrieveBuffer(handler, len, (unsigned char*) mc.GetBuffer(), true);
                    mc.SetSize(len);
                    mc.SetNull();
                }

                switch (chatRequestID) {
                    case idEnter:
                    {
                        unsigned short vt;
                        mc >> vt;
                        assert(vt == (VT_UINT | VT_ARRAY));
                        mc >> len;
                        assert(len * sizeof (unsigned int) == mc.GetSize());
                        p->OnSubscribe((const unsigned int*) mc.GetBuffer(), len);
                    }
                        break;
                    case idExit:
                    {
                        std::vector<unsigned int> groups = p->GetChatGroups();
                        p->OnUnsubscribe(groups.data(), (unsigned int) groups.size());
                    }
                        break;
                    case idSpeak:
                    {
                        unsigned short vt;
                        const unsigned int *pGroup;
                        UVariant vtParam0;
                        mc >> vt;
                        assert(vt == (VT_UINT | VT_ARRAY));
                        mc >> len;
                        pGroup = (const unsigned int*) mc.GetBuffer();
                        mc.Pop((unsigned int) (len * sizeof (unsigned int)));
                        mc >> vtParam0;
                        p->OnPublish(vtParam0, pGroup, len);
                    }
                        break;
                    case idSendUserMessage:
                    {
                        UVariant vtParam0;
                        std::wstring userId;
                        mc >> userId;
                        mc >> vtParam0;
                        p->OnSendUserMessage(userId.c_str(), vtParam0);
                        assert(mc.GetSize() == 0);
                    }
                        break;
                    case idSendUserMessageEx:
                    {
                        std::wstring userId;
                        mc >> userId;
                        len = mc.GetSize();
                        assert(ServiceId != sidHTTP);
                        ((CClientPeer*) p)->OnSendUserMessageEx(userId.c_str(), mc.GetBuffer(), len);
                    }
                        break;
                    case idSpeakEx:
                    {
                        unsigned int size;
                        mc >> size;
                        assert(size <= mc.GetSize());
                        const unsigned char *msg = mc.GetBuffer();
                        const unsigned int *pGroup = (const unsigned int *) mc.GetBuffer(size);
                        len = (mc.GetSize() - size) / sizeof (unsigned int);
                        assert(ServiceId != sidHTTP);
                        ((CClientPeer*) p)->OnPublishEx(pGroup, len, msg, size);
                    }
                        break;
                    default:
                        break;
                }
            }
        }

        void CALLBACK CBaseService::OnChatCame(USocket_Server_Handle handler, tagChatRequestID chatRequestId) {
            assert(ServerCoreLoader.IsMainThread());
            unsigned int ServiceId = ServerCoreLoader.GetSvsID(handler);
            const CBaseService *pService = SeekService(ServiceId);
            if (!pService)
                return;
            CSocketPeer *p = pService->Seek(handler);
            if (p)
                p->OnChatRequestCame(chatRequestId);
        }

        bool CBaseService::AddMe(unsigned int nServiceId, tagThreadApartment ta) {
            m_SvsContext.m_ta = ta;
            if (nServiceId && !Seek(nServiceId) && ServerCoreLoader.AddSvsContext(nServiceId, m_SvsContext)) {
                m_nServiceId = nServiceId;
                return true;
            }
            return false;
        }

        void CBaseService::RemoveMe() {
            if (m_nServiceId && Seek(m_nServiceId)) {
                ServerCoreLoader.RemoveASvsContext(m_nServiceId);
                CAutoLock sl(m_mutex);
                std::vector<CBaseService*>::iterator location = std::find(m_vService.begin(), m_vService.end(), this);
                if (location != m_vService.end())
                    m_vService.erase(location);
            }
            m_nServiceId = 0;
        }

        unsigned int CBaseService::GetSvsID() const {
            return m_nServiceId;
        }

        unsigned int CBaseService::GetCountOfSlowRequests() const {
            return ServerCoreLoader.GetCountOfSlowRequests(m_nServiceId);
        }

        CSvsContext CBaseService::GetSvsContext(unsigned int serviceId) {
            return ServerCoreLoader.GetSvsContext(serviceId);
        }

        const CSvsContext & CBaseService::GetSvsContext() const {
            return m_SvsContext;
        }

        std::vector<unsigned short> CBaseService::GetAllSlowRequestIds() const {
            CScopeUQueue sb;
            unsigned int count = GetCountOfSlowRequests() + 10;
            if (sb->GetMaxSize() < count * sizeof (unsigned short))
                sb->ReallocBuffer(count * sizeof (unsigned short));
            count = ServerCoreLoader.GetAllSlowRequestIds(m_nServiceId, (unsigned short*) sb->GetBuffer(), count + 10);
            const unsigned short *req = (const unsigned short*) sb->GetBuffer();
            std::vector<unsigned short> vReq(req, req + count);
            return vReq;
        }

        std::vector<unsigned short> CBaseService::GetAlphaRequestIds() const {
            CScopeUQueue sb;
            unsigned int count = ServerCoreLoader.GetAlphaRequestIds(m_nServiceId, (unsigned short*) sb->GetBuffer(), sb->GetMaxSize() / sizeof (unsigned short));
            const unsigned short *req = (const unsigned short*) sb->GetBuffer();
            std::vector<unsigned short> vReq(req, req + count);
            return vReq;
        }

        //if sRequestId <= idReservedTwo, it will return false

        bool CBaseService::AddSlowRequest(unsigned short sReqId) const {
            return ServerCoreLoader.AddSlowRequest(m_nServiceId, sReqId);
        }

        //if reqId <= idReservedTwo, it will return false

        bool CBaseService::AddAlphaRequest(unsigned short reqId) const {
            return ServerCoreLoader.AddAlphaRequest(m_nServiceId, reqId);
        }

        void CBaseService::RemoveSlowRequest(unsigned short sReqId) const {
            ServerCoreLoader.RemoveSlowRequest(m_nServiceId, sReqId);
        }

        void CBaseService::RemoveAllSlowRequests() const {
            ServerCoreLoader.RemoveAllSlowRequests(m_nServiceId);
        }

        bool CBaseService::Seek(unsigned int nServiceId) {
            unsigned int n, count = ServerCoreLoader.GetCountOfServices() + 10;
            CScopeUQueue sb;
            if (sb->GetMaxSize() < count * sizeof (unsigned int))
                sb->ReallocBuffer(count * sizeof (unsigned int));
            unsigned int *p = (unsigned int*) sb->GetBuffer();
            count = ServerCoreLoader.GetServices(p, count + 10);
            for (n = 0; n < count; n++) {
                if (p[n] == nServiceId)
                    return true;
            }
            return false;
        }

        CBaseService * CBaseService::SeekService(USocket_Server_Handle h) {
            unsigned int serviceId = ServerCoreLoader.GetSvsID(h);
            return SeekService(serviceId);
        }

        CBaseService * CBaseService::SeekService(unsigned int nServiceId) {
#ifdef MAY_CHANGE_SERVICES_AFTER_STARTING_SERVER
            /*
             1. Remove this lock as you will not add or remove a service after running a server under most cases.
             2. Make this lock if you will surely add or remove a service after running a server.
             3. Adding extra lock may slightly degrade performance.
             */
            CAutoLock sl(m_mutex);
#endif
            std::vector<CBaseService*>::const_iterator it, end = m_vService.cend();
            for (it = m_vService.cbegin(); it != end; ++it) {
                if ((*it)->GetSvsID() == nServiceId)
                    return *it;
            }
            return nullptr;
        }

        CSocketPeer * CBaseService::CreatePeer(USocket_Server_Handle h, unsigned int oldServiceId) {
            CSocketPeer *p = nullptr;
            CAutoLock sl(m_mutex);
            size_t size = m_vDeadPeer.size();
            if (size) {
                p = m_vDeadPeer.front();
                m_vDeadPeer.pop_front();
            }
            if (!p) {
                p = GetPeerSocket();
                p->m_pBase = this;
            }
            p->m_hHandler = h;
            m_vPeer.push_back(p);
            return p;
        }

        CSocketPeer * CBaseService::Seek(USocket_Server_Handle h) const {
            std::vector<CSocketPeer*>::const_iterator it;
            CAutoLock sl(m_mutex);
            std::vector<CSocketPeer*>::const_iterator end = m_vPeer.cend();
            for (it = m_vPeer.cbegin(); it != end; ++it) {
                if ((*it)->m_hHandler == h) {
                    return (*it);
                }
            }
            return nullptr;
        }

        void CBaseService::ReleasePeer(USocket_Server_Handle h, bool bClosing, unsigned int info) {
            std::vector<CSocketPeer*>::iterator it;
            CAutoLock sl(m_mutex);
            std::vector<CSocketPeer*>::iterator end = m_vPeer.end();
            for (it = m_vPeer.begin(); it != end; ++it) {
                CSocketPeer *pPeer = *it;
                if (pPeer->m_hHandler == h) {
                    pPeer->OnReleaseSource(bClosing, info);
                    pPeer->m_hHandler = 0;
                    if (pPeer->m_UQueue.GetMaxSize() > 2 * DEFAULT_INITIAL_MEMORY_BUFFER_SIZE) {
                        pPeer->m_UQueue.ReallocBuffer(DEFAULT_INITIAL_MEMORY_BUFFER_SIZE);
                    }
                    CHttpPeerBase *pHttp = dynamic_cast<CHttpPeerBase *> (pPeer);
                    if (pHttp) {
                        pHttp->m_vArg.clear();
                        pHttp->m_WebRequestName.clear();
                    }
                    m_vPeer.erase(it);
                    m_vDeadPeer.push_back(pPeer);
                    break;
                }
            }
        }

        HINSTANCE CSocketProServer::DllManager::AddALibrary(const char *libFile, int param) {
            return ServerCoreLoader.AddADll(libFile, param);
        }

        bool CSocketProServer::DllManager::RemoveALibrary(HINSTANCE hLib) {
            return ServerCoreLoader.RemoveADllByHandle(hLib);
        }

        void CBaseService::Clean() {
            CAutoLock sl(m_mutex);
            for (auto it = m_vDeadPeer.begin(), end = m_vDeadPeer.end(); it != end; ++it) {
                delete(*it);
            }
            m_vDeadPeer.clear();
            for (auto it = m_vPeer.begin(), end = m_vPeer.end(); it != end; ++it) {
                //comment out the below call to avoid crashing here
                //::PostClose((*it)->m_hHandler); 
                delete(*it);
            }
            m_vPeer.clear();
        }

        CSocketProServer::CSocketProServer(int nParam)
        : m_listeningPort(20901), m_maxBacklog(32) {
            ServerCoreLoader.Load();
            if (m_pServer != nullptr)
                throw CUExCode("One instance supported only per application!", MB_BAD_OPERATION);
            if (!ServerCoreLoader.IsLoaded())
                throw CUExCode("Server core library not accessible!", MB_BAD_OPERATION);
            bool ok = ServerCoreLoader.InitSocketProServer(nParam);
            assert(ok);
            ServerCoreLoader.SetOnAccept(&CSocketProServer::OnAcceptedInternal);
            ServerCoreLoader.SetOnIdle(&CSocketProServer::OnIdleInternal);
            ServerCoreLoader.SetOnClose(&CSocketProServer::OnClientClosedInternal);
            ServerCoreLoader.SetOnSSLHandShakeCompleted(&CSocketProServer::OnSSLShakeCompletedInternal);
            ServerCoreLoader.SetOnIsPermitted(&CSocketProServer::IsPermittedInternal);
            m_pServer = this;
        }

        CSocketProServer::~CSocketProServer() {
            StopSocketProServer();
            ServerCoreLoader.UninitSocketProServer();
            m_pServer = nullptr;
        }

        CSocketProServer * CSocketProServer::GetServer() {
            return m_pServer;
        }

        UINT64 CSocketProServer::GetRequestCount() {
            return ServerCoreLoader.GetRequestCount();
        }

        bool CSocketProServer::IsMainThread() {
            return ServerCoreLoader.IsMainThread();
        }

        bool CSocketProServer::IsServerSSLEnabled() {
            return ServerCoreLoader.IsServerSSLEnabled();
        }

        int CSocketProServer::GetErrorCode() {
            return ServerCoreLoader.GetServerErrorCode();
        }

        bool CSocketProServer::PushManager::AddAChatGroup(unsigned int chatGroupId, const wchar_t * description) {
            ServerCoreLoader.AddAChatGroup(chatGroupId, description);
            return true;
        }

        std::wstring CSocketProServer::PushManager::GetAChatGroupDescription(unsigned int groupId) {
            wchar_t description[4097] =
            {0};
            ServerCoreLoader.GetAChatGroup(groupId, description, sizeof (description) / sizeof (wchar_t));
            return description;
        }

        unsigned int CSocketProServer::PushManager::GetCountOfChatGroups() {
            return ServerCoreLoader.GetCountOfChatGroups();
        }

        bool CSocketProServer::PushManager::Publish(const UVariant& vtMessage, const unsigned int *pGroups, unsigned int ulGroupCount) {
            CScopeUQueue sb;
            sb << vtMessage;
            return ServerCoreLoader.SpeakPush(sb->GetBuffer(), sb->GetSize(), pGroups, ulGroupCount);
        }

        bool CSocketProServer::PushManager::SendUserMessage(const UVariant& vtMessage, const wchar_t * strUserId) {
            CScopeUQueue sb;
            sb << vtMessage;
            return ServerCoreLoader.SendUserMessagePush(strUserId, sb->GetBuffer(), sb->GetSize());
        }

        bool CSocketProServer::PushManager::Publish(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count) {
            return ServerCoreLoader.SpeakExPush(message, size, pChatGroupId, count);
        }

        bool CSocketProServer::PushManager::SendUserMessage(const wchar_t *userId, const unsigned char *message, unsigned int size) {
            return ServerCoreLoader.SendUserMessageExPush(userId, message, size);
        }

        std::vector<unsigned int> CSocketProServer::PushManager::GetAllCreatedChatGroups() {
            std::vector<unsigned int> groups(ServerCoreLoader.GetCountOfChatGroups());
            unsigned int res = ServerCoreLoader.GetAllCreatedChatGroups(groups.data(), (unsigned int) groups.size());
            groups.resize(res);
            return groups;
        }

        void CSocketProServer::PushManager::RemoveChatGroup(unsigned int chatGroupId) {
            ServerCoreLoader.RemoveChatGroup(chatGroupId);
        }

        CServerQueue CSocketProServer::QueueManager::StartQueue(const char *qName, unsigned int ttl, bool dequeueShared) {
            return CServerQueue(ServerCoreLoader.StartQueue(qName, dequeueShared, ttl));
        }

        bool CSocketProServer::QueueManager::IsServerQueueIndexPossiblyCrashed() {
            return ServerCoreLoader.IsServerQueueIndexPossiblyCrashed();
        }

        void CSocketProServer::QueueManager::SetMessageQueuePassword(const char *pwd) {
            ServerCoreLoader.SetMessageQueuePassword(pwd);
        }

        CServerQueue::CServerQueue(unsigned int handle)
        : m_handle(handle) {
        }

        unsigned int CServerQueue::GetHandle() const {
            return m_handle;
        }

        unsigned int CServerQueue::GetMessagesInDequeuing() const {
            return ServerCoreLoader.GetMessagesInDequeuing(m_handle);
        }

        tagOptimistic CServerQueue::GetOptimistic() const {
            return ServerCoreLoader.GetOptimistic(m_handle);
        }

        void CServerQueue::SetOptimistic(tagOptimistic optimistic) const {
            ServerCoreLoader.SetOptimistic(m_handle, optimistic);
        }

        std::time_t CServerQueue::GetLastMessageTime() const {
            std::tm tm;
            ::memset(&tm, 0, sizeof (tm));
            tm.tm_year = 113;
            tm.tm_mday = 1;
            return std::mktime(&tm) + ServerCoreLoader.GetLastQueueMessageTime(m_handle);
        }

        unsigned int CServerQueue::GetTTL() const {
            return ServerCoreLoader.GetTTL(m_handle);
        }

        UINT64 CServerQueue::RemoveByTTL() const {
            return ServerCoreLoader.RemoveQueuedRequestsByTTL(m_handle);
        }

        tagQueueStatus CServerQueue::GetQueueOpenStatus() const {
            return ServerCoreLoader.GetServerQueueStatus(m_handle);
        }

        void CServerQueue::Reset() const {
            ServerCoreLoader.ResetQueue(m_handle);
        }

        bool CServerQueue::AppendTo(const IServerQueue & serverQueue) const {
            unsigned int handle = serverQueue.GetHandle();
            return ServerCoreLoader.PushQueueTo(m_handle, &handle, 1);
        }

        bool CServerQueue::AppendTo(const unsigned int *handles, unsigned int count) const {
            return ServerCoreLoader.PushQueueTo(m_handle, handles, count);
        }

        bool CServerQueue::EnsureAppending(const IServerQueue & serverQueue) const {
            unsigned int handle = serverQueue.GetHandle();
            return EnsureAppending(&handle, 1);
        }

        bool CServerQueue::EnsureAppending(const unsigned int *handles, unsigned int count) const {
            if (!IsAvailable())
                return false;
            if (GetQueueOpenStatus() != qsMergePushing)
                return true;
            if (!handles || !count)
                return true; //don't do anything in case there is no target queue available
            std::vector<unsigned int> vHandles;
            for (unsigned int n = 0; n < count; ++n) {
                if (ServerCoreLoader.GetServerQueueStatus(handles[n]) != qsMergeComplete)
                    vHandles.push_back(handles[n]);
            }
            count = (unsigned int) vHandles.size();
            if (count > 0) {
                const unsigned int &header = vHandles.front();
                return AppendTo(&header, count);
            }
            Reset();
            return true;
        }

        bool CServerQueue::AbortJob() const {
            return ServerCoreLoader.AbortJob(m_handle);
        }

        bool CServerQueue::StartJob() const {
            return ServerCoreLoader.StartJob(m_handle);
        }

        bool CServerQueue::EndJob() const {
            return ServerCoreLoader.EndJob(m_handle);
        }

        UINT64 CServerQueue::GetJobSize() const {
            return ServerCoreLoader.GetJobSize(m_handle);
        }

        UINT64 CServerQueue::GetLastIndex() const {
            return ServerCoreLoader.GetQueueLastIndex(m_handle);
        }

        UINT64 CServerQueue::CancelQueuedRequests(UINT64 startIndex, UINT64 endIndex) const {
            return ServerCoreLoader.CancelQueuedRequestsByIndex(m_handle, startIndex, endIndex);
        }

        UINT64 CServerQueue::BatchEnqueue(unsigned int count, const unsigned char *msgStruct) const {
            return ServerCoreLoader.BatchEnqueue(m_handle, count, msgStruct);
        }

        UINT64 CServerQueue::Enqueue(unsigned short reqId, const unsigned char *buffer, unsigned int size) const {
            return ServerCoreLoader.Enqueue(m_handle, reqId, buffer, size);
        }

        UINT64 CServerQueue::Enqueue(unsigned short reqId, const CUQueue & qBuffer) const {
            return Enqueue(reqId, qBuffer.GetBuffer(), qBuffer.GetSize());
        }

        UINT64 CServerQueue::GetMessageCount() const {
            return ServerCoreLoader.GetMessageCount(m_handle);
        }

        void CServerQueue::StopQueue(bool permanent) {
            ServerCoreLoader.StopQueueByHandle(m_handle, permanent);
            m_handle = 0;
        }

        bool CSocketProServer::QueueManager::StopQueue(const char *qName, bool permanent) {
            return ServerCoreLoader.StopQueueByName(qName, permanent);
        }

        bool CSocketProServer::QueueManager::IsQueueStarted(const char *qName) {
            return ServerCoreLoader.IsQueueStartedByName(qName);
        }

        bool CServerQueue::IsDequeueShared() const {
            return ServerCoreLoader.IsDequeueShared(m_handle);
        }

        bool CServerQueue::IsAvailable() const {
            return ServerCoreLoader.IsQueueStartedByHandle(m_handle);
        }

        bool CSocketProServer::QueueManager::IsQueueSecured(const char *qName) {
            return ServerCoreLoader.IsQueueSecuredByName(qName);
        }

        void CSocketProServer::QueueManager::SetWorkDirectory(const char *dir) {
            ServerCoreLoader.SetServerWorkDirectory(dir);
        }

        const char* CSocketProServer::QueueManager::GetWorkDirectory() {
            return ServerCoreLoader.GetServerWorkDirectory();
        }

        bool CServerQueue::IsSecure() const {
            return ServerCoreLoader.IsQueueSecuredByHandle(m_handle);
        }

        const char* CServerQueue::GetQueueFileName() const {
            return ServerCoreLoader.GetQueueFileName(m_handle);
        }

        const char* CServerQueue::GetQueueName() const {
            return ServerCoreLoader.GetQueueName(m_handle);
        }

        UINT64 CServerQueue::GetQueueSize() const {
            return ServerCoreLoader.GetQueueSize(m_handle);
        }

        std::string CSocketProServer::GetErrorMessage() {
            char strErrorMsg[2049];
            ServerCoreLoader.GetServerErrorMessage(strErrorMsg, sizeof (strErrorMsg));
            return strErrorMsg;
        }

        void CSocketProServer::UseSSL(const char *certFile, const char *keyFile, const char *pwdForPrivateKeyFile, const char *dhFile) {
            ServerCoreLoader.SetPrivateKeyFile(keyFile);
            ServerCoreLoader.SetCertFile(certFile);
            ServerCoreLoader.SetPKFPassword(pwdForPrivateKeyFile);
            ServerCoreLoader.SetDHParmsFile(dhFile);
            ServerCoreLoader.SetDefaultEncryptionMethod(TLSv1);
        }

        bool CSocketProServer::Router::SetRouting(unsigned int serviceId0, unsigned int serviceId1, tagRoutingAlgorithm ra0, tagRoutingAlgorithm ra1) {
            return ServerCoreLoader.SetRouting(serviceId0, ra0, serviceId1, ra1);
        }

        unsigned int CSocketProServer::Router::CheckRouting(unsigned int serviceId) {
            return ServerCoreLoader.CheckRouting(serviceId);
        }

        void CSocketProServer::Config::SetMaxThreadIdleTimeBeforeSuicide(unsigned int maxThreadIdleTimeBeforeSuicide) {
            ServerCoreLoader.SetMaxThreadIdleTimeBeforeSuicide(maxThreadIdleTimeBeforeSuicide);
        }

        void CSocketProServer::Config::SetMaxConnectionsPerClient(unsigned int maxConnectionsPerClient) {
            ServerCoreLoader.SetMaxConnectionsPerClient(maxConnectionsPerClient);
        }

        unsigned int CSocketProServer::Config::GetMaxThreadIdleTimeBeforeSuicide() {
            return ServerCoreLoader.GetMaxThreadIdleTimeBeforeSuicide();
        }

        unsigned int CSocketProServer::Config::GetMaxConnectionsPerClient() {
            return ServerCoreLoader.GetMaxConnectionsPerClient();
        }

        void CSocketProServer::Config::SetTimerElapse(unsigned int timerElapse) {
            ServerCoreLoader.SetTimerElapse(timerElapse);
        }

        unsigned int CSocketProServer::Config::GetTimerElapse() {
            return ServerCoreLoader.GetTimerElapse();
        }

        void CSocketProServer::Config::SetSMInterval(unsigned int SMInterval) {
            ServerCoreLoader.SetSMInterval(SMInterval);
        }

        unsigned int CSocketProServer::Config::GetSMInterval() {
            return ServerCoreLoader.GetSMInterval();
        }

        unsigned int CSocketProServer::Config::GetPingInterval() {
            return ServerCoreLoader.GetPingInterval();
        }

        void CSocketProServer::Config::SetPingInterval(unsigned int pingInterval) {
            ServerCoreLoader.SetPingInterval(pingInterval);
        }

        tagAuthenticationMethod CSocketProServer::Config::GetAuthenticationMethod() {
            return ServerCoreLoader.GetAuthenticationMethod();
        }

        void CSocketProServer::Config::SetPassword(const char* pwd) {
            ServerCoreLoader.SetPKFPassword(pwd);
        }

        void CSocketProServer::Config::SetSharedAM(bool b) {
            ServerCoreLoader.SetSharedAM(b);
        }

        bool CSocketProServer::Config::GetSharedAM() {
            return ServerCoreLoader.GetSharedAM();
        }

        unsigned int CSocketProServer::Config::GetMainThreads() {
            return ServerCoreLoader.GetMainThreads();
        }

        void CSocketProServer::Config::SetAuthenticationMethod(tagAuthenticationMethod am) {
            ServerCoreLoader.SetAuthenticationMethod(am);
        }

        void CSocketProServer::Config::SetDefaultEncryptionMethod(tagEncryptionMethod em) {
            ServerCoreLoader.SetDefaultEncryptionMethod(em);
        }

        tagEncryptionMethod CSocketProServer::Config::GetDefaultEncryptionMethod() {
            return ServerCoreLoader.GetDefaultEncryptionMethod();
        }

        void CSocketProServer::Config::SetSwitchTime(unsigned int switchTime) {
            ServerCoreLoader.SetSwitchTime(switchTime);
        }

        unsigned int CSocketProServer::Config::GetSwitchTime() {
            return ServerCoreLoader.GetSwitchTime();
        }

        unsigned int CSocketProServer::GetCountOfClients() {
            return ServerCoreLoader.GetCountOfClients();
        }

        USocket_Server_Handle CSocketProServer::GetClient(unsigned int index) {
            return ServerCoreLoader.GetClient(index);
        }

        std::string CSocketProServer::GetLocalName() {
            char localname[256] =
            {0};
            ServerCoreLoader.GetLocalName(localname, sizeof (localname));
            return localname;
        }

        const char* CSocketProServer::GetVersion() {
            return ServerCoreLoader.GetUServerSocketVersion();
        }

        void SetLastCallInfo(const char *str, int data, const char *func) {
            char buff[4097] =
            {0};
#ifdef WIN32_64
            _snprintf_s(buff, sizeof (buff), sizeof (buff), "lf: %s, what: %s, data: %d", func, str, data);
#else
            snprintf(buff, sizeof (buff), "lf: %s, what: %s, data: %d", func, str, data);
#endif
            ServerCoreLoader.SetLastCallInfo(buff);
        }

        bool CSocketProServer::CredentialManager::HasUserId(const wchar_t * userId) {
            return ServerCoreLoader.HasUserId(userId);
        }

        std::wstring CSocketProServer::CredentialManager::GetUserID(USocket_Server_Handle h) {
            wchar_t str[256] =
            {0};
            ServerCoreLoader.GetUID(h, str, sizeof (str) / sizeof (wchar_t));
            return str;
        }

        std::wstring CSocketProServer::CredentialManager::GetPassword(USocket_Server_Handle h) {
            wchar_t str[256] =
            {0};
            ServerCoreLoader.GetPassword(h, str, sizeof (str) / sizeof (wchar_t));
            return str;
        }

        bool CSocketProServer::CredentialManager::SetPassword(USocket_Server_Handle h, const wchar_t * password) {
            return ServerCoreLoader.SetPassword(h, password);
        }

        bool CSocketProServer::CredentialManager::SetUserID(USocket_Server_Handle h, const wchar_t * userId) {
            return ServerCoreLoader.SetUserID(h, userId);
        }

        bool CSocketProServer::Run(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
            if (!OnSettingServer(listeningPort, maxBacklog, v6))
                return false;
            bool ok = ServerCoreLoader.StartSocketProServer(listeningPort, maxBacklog, v6);
            CBaseService::m_nMainThreads = (~0);
            return ok;
        }

        void CSocketProServer::StopSocketProServer() {
            ServerCoreLoader.StopSocketProServer();
        }

        void CSocketProServer::PostQuit() {
            ServerCoreLoader.PostQuitPump();
        }

        bool CSocketProServer::IsRunning() {
            return ServerCoreLoader.IsRunning();
        }

        void CSocketProServer::OnAccept(USocket_Server_Handle h, int errCode) {

        }

        void CSocketProServer::OnIdle(INT64 milliseconds) {
            UINT64 size = CScopeUQueue::GetMemoryConsumed();
            if (size / 1024 > SHARED_BUFFER_CLEAN_SIZE) {
                CScopeUQueue::DestroyUQueuePool();
            }
        }

        void CSocketProServer::OnClose(USocket_Server_Handle h, int errCode) {

        }

        bool CSocketProServer::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
            return true;
        }

        void CSocketProServer::OnSSLShakeCompleted(USocket_Server_Handle Handler, int errCode) {

        }

        void WINAPI CSocketProServer::OnClientClosedInternal(USocket_Server_Handle Handler, int errCode) {
            assert(IsMainThread());
            m_pServer->OnClose(Handler, errCode);
        }

        void WINAPI CSocketProServer::OnSSLShakeCompletedInternal(USocket_Server_Handle Handler, int errCode) {
            assert(IsMainThread());
            m_pServer->OnSSLShakeCompleted(Handler, errCode);
        }

        void WINAPI CSocketProServer::OnAcceptedInternal(USocket_Server_Handle Handler, int errCode) {
            assert(IsMainThread());
            m_pServer->OnAccept(Handler, errCode);
        }

        bool WINAPI CSocketProServer::IsPermittedInternal(USocket_Server_Handle Handler, unsigned int serviceId) {
            wchar_t userId[MAX_USERID_CHARS + 1];
            wchar_t password[MAX_PASSWORD_CHARS + 1];
            ServerCoreLoader.GetUID(Handler, userId, sizeof (userId) / sizeof (wchar_t));
            ServerCoreLoader.GetPassword(Handler, password, sizeof (password) / sizeof (wchar_t));
            assert(IsMainThread());
            bool ok = m_pServer->OnIsPermitted(Handler, userId, password, serviceId);
            memset(password, 0, sizeof (password));
            return ok;
        }

        void WINAPI CSocketProServer::OnIdleInternal(INT64 milliseconds) {
            assert(IsMainThread());
            if ((milliseconds % ServerCoreLoader.GetSMInterval()) < 3 * ServerCoreLoader.GetTimerElapse() / 2)
                CScopeUQueue::ResetSize();
            m_pServer->OnIdle(milliseconds);
        }

    }//ServerSide
}//SPA