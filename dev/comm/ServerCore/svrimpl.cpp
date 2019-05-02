
#if defined(OLD_IMPL)
#include "stdafx.h"
#include "server.h"
#elif defined(WIN32_64)
#include "../ServerCoreWin/stdafx.h"
#include "../ServerCoreWin/server.h"
#else
#include "../ServerCoreUnix/stdafx.h"
#include "../ServerCoreUnix/server.h"
#endif

#include "../../include/userver.h"
#include "../../pinc/uzip.h"
#include "../../include/mysql/umysql.h"
#include "../../include/sqlite/usqlite.h"

CServerSession *GetSvrSession(USocket_Server_Handle h, unsigned int &index) {
    index = (h & MAX_SESSION_INDEX);
    h >>= INDEX_SHIFT_BITS;
    CServerSession *p = (CServerSession *) h;
    return p;
}

void WINAPI Close(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        pSession->Close();
    }
}

unsigned short WINAPI GetCurrentRequestID(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetCurrentRequestID();
    return 0;
}

unsigned int WINAPI GetCurrentRequestLen(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetCurrentRequestLen();
    return SOCKET_NOT_FOUND;
}

unsigned int WINAPI GetRcvBytesInQueue(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetRcvBytesInQueue();
    return SOCKET_NOT_FOUND;
}

const unsigned char* WINAPI GetRequestBuffer(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetRequestBuffer();
    return nullptr;
}

unsigned int WINAPI GetSndBytesInQueue(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetSndBytesInQueue();
    return SOCKET_NOT_FOUND;
}

std::vector<PThreadEvent> g_vThreadEvent;

void WINAPI SetThreadEvent(PThreadEvent func) {
    if (!func)
        return;
    std::vector<PThreadEvent>::iterator it = std::find(g_vThreadEvent.begin(), g_vThreadEvent.end(), func);
    if (it == g_vThreadEvent.end()) {
        g_vThreadEvent.push_back(func);
    } else {
        g_vThreadEvent.erase(it);
    }
}

void WINAPI PostClose(USocket_Server_Handle h, int errCode) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        pSession->PostClose(errCode);
    }
}

unsigned int WINAPI QueryRequestsInQueue(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->QueryRequestsQueued();
    return SOCKET_NOT_FOUND;
}

void WINAPI RegisterMe(unsigned int svsId, SPA::UINT64 secretNumber) {
    if (!CServerSession::IsBuiltinAllowed(svsId) || !g_pServer)
        return;
    CServiceContext *svs = g_pServer->SeekServiceContext(svsId);
    if (!svs)
        return;
    switch (svsId) {
        case SPA::sidChat:
            svs->m_bRegisterred = (0x5f00240039078012 == secretNumber);
            break;
        case SPA::sidODBC:
            svs->m_bRegisterred = (0x3f002501780560AF == secretNumber);
            break;
        case SPA::sidFile:
            svs->m_bRegisterred = (0x4f00990088000077 == secretNumber);
            break;
        case SPA::Sqlite::sidSqlite:
            svs->m_bRegisterred = (0x6f00AA00BB0000CC == secretNumber);
            break;
        case SPA::Mysql::sidMysql:
            svs->m_bRegisterred = (0x7f00110022003300 == secretNumber);
            break;
        default:
            break;
    }
}

bool WINAPI GetPeerName(USocket_Server_Handle h, unsigned int *peerPort, char *strIpAddr, unsigned short bufferLen) {
    unsigned int index;
    unsigned short port = 0;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        std::string str;
        pSession->GetPeerName(str, &port);
        if (peerPort)
            *peerPort = port;
        if (strIpAddr != nullptr && bufferLen != 0) {
            --bufferLen;
            if (bufferLen > (unsigned short) str.length())
                bufferLen = (unsigned short) str.length();
            ::memcpy(strIpAddr, str.c_str(), bufferLen);
            strIpAddr[bufferLen] = 0;
            return (bufferLen > 0);
        }
    }
    if (strIpAddr && bufferLen)
        strIpAddr[0] = 0;
    return false;
}

unsigned int WINAPI RetrieveBuffer(USocket_Server_Handle h, unsigned int ulBufferSize, unsigned char *pBuffer, bool bPeek) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->RetrieveRequestBuffer(pBuffer, ulBufferSize, bPeek);
    return SOCKET_NOT_FOUND;
}

unsigned int WINAPI SendExceptionResult(USocket_Server_Handle h, const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        if (errMessage && SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            const SPA::UTF16 *str = (const SPA::UTF16 *)errMessage;
            unsigned int len = SPA::Utilities::GetLen(str);
            std::wstring s = SPA::ToNativeString(str, len);
            return pSession->SendExceptionResult(s.c_str(), errWhere, requestId, errCode);
        }
        return pSession->SendExceptionResult(errMessage, errWhere, requestId, errCode);
    }
    return SOCKET_NOT_FOUND;
}

unsigned int WINAPI SendExceptionResultIndex(USocket_Server_Handle h, SPA::UINT64 indexCall, const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        if (errMessage && SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            const SPA::UTF16 *str = (const SPA::UTF16 *)errMessage;
            unsigned int len = SPA::Utilities::GetLen(str);
            std::wstring s = SPA::ToNativeString(str, len);
            return pSession->SendExceptionResultIndex(indexCall, s.c_str(), errWhere, requestId, errCode);
        }
        return pSession->SendExceptionResultIndex(indexCall, errMessage, errWhere, requestId, errCode);
    }
    return SOCKET_NOT_FOUND;
}

bool WINAPI Enter(USocket_Server_Handle h, const unsigned int *pChatGroupId, unsigned int count) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->Enter(pChatGroupId, count);
    return false;
}

void WINAPI Exit(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        pSession->Exit();
}

SPA::tagOperationSystem WINAPI GetPeerOs(USocket_Server_Handle h, bool *endian) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetPeerOs(endian);
    if (endian)
        *endian = SPA::IsBigEndian();
    return SPA::GetOS();
}

bool WINAPI Speak(USocket_Server_Handle h, const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->Speak(message, size, pChatGroupId, count);
    return false;
}

bool WINAPI SpeakEx(USocket_Server_Handle h, const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->SpeakEx(message, size, pChatGroupId, count);
    return false;
}

bool WINAPI MakeRequest(USocket_Server_Handle handler, unsigned short requestId, const unsigned char *request, unsigned int size) {
    unsigned int index;
    if (requestId <= SPA::idReservedTwo)
        return false;
    CServerSession *pSession = GetSvrSession(handler, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->FakeAClientRequest(requestId, request, size);
    return false;
}

bool WINAPI SendUserMessageEx(USocket_Server_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        if (userId && SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            const SPA::UTF16 *str = (const SPA::UTF16 *)userId;
            unsigned int len = SPA::Utilities::GetLen(str);
            std::wstring s = SPA::ToNativeString(str, len);
            return pSession->SendUserMessageEx(s.c_str(), message, size);
        }
        return pSession->SendUserMessageEx(userId, message, size);
    }
    return false;
}

bool WINAPI SendUserMessage(USocket_Server_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        if (userId && SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            const SPA::UTF16 *str = (const SPA::UTF16 *)userId;
            unsigned int len = SPA::Utilities::GetLen(str);
            std::wstring s = SPA::ToNativeString(str, len);
            return pSession->SendUserMessage(s.c_str(), message, size);
        }
        return pSession->SendUserMessage(userId, message, size);
    }
    return false;
}

unsigned int WINAPI GetCountOfJoinedChatGroups(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetCountOfJoinedChatGroups();
    return 0;
}

unsigned int WINAPI GetJoinedGroupIds(USocket_Server_Handle h, unsigned int *pChatGroup, unsigned int count) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetJoinedGroupIds(pChatGroup, count);
    return 0;
}

void WINAPI SetOnSSLHandShakeCompleted(POnSSLHandShakeCompleted p) {
    if (g_pServer) g_pServer->SetOnSSLHandShakeCompleted(p);
}

void WINAPI SetOnClose(POnClose p) {
    if (g_pServer) g_pServer->SetOnClose(p);
}

void WINAPI SetOnIsPermitted(POnIsPermitted p) {
    if (g_pServer) g_pServer->SetOnIsPermitted(p);
}

bool WINAPI IsOpened(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->IsOpened();
    return false;
}

SPA::UINT64 WINAPI GetBytesReceived(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetBytesReceived();
    return 0;
}

SPA::UINT64 WINAPI GetBytesSent(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetBytesSent();
    return 0;
}

unsigned int WINAPI SendReturnDataIndex(USocket_Server_Handle h, SPA::UINT64 indexCall, unsigned short usReqId, unsigned int ulBufferSize, const unsigned char *pBuffer) {
    unsigned int index;
    if (usReqId <= SPA::idReservedTwo)
        return RESULT_SENDING_FAILED;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    if (g_pServer->m_bStopped) {
        return SOCKET_NOT_FOUND;
    }
    bool bMainThread = ::IsMainThread();
    while (!bMainThread && pSession->GetSndBytesInQueue() > 30 * IO_BUFFER_SIZE) {
        if (g_pServer->m_bStopped || !pSession->IsOpened()) {
            return SOCKET_NOT_FOUND;
        }
        pSession->Wait();
        if (index != pSession->GetConnIndex())
            return SOCKET_NOT_FOUND;
    }
    if (!bMainThread && index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    return pSession->SendReturnDataIndex(indexCall, usReqId, pBuffer, ulBufferSize);
}

unsigned int WINAPI SendReturnData(USocket_Server_Handle h, unsigned short usReqId, unsigned int ulBufferSize, const unsigned char *pBuffer) {
    unsigned int index;
    if (usReqId <= SPA::idReservedTwo)
        return RESULT_SENDING_FAILED;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    if (g_pServer->m_bStopped) {
        return SOCKET_NOT_FOUND;
    }
    bool bMainThread = ::IsMainThread();
    while (!bMainThread && pSession->GetSndBytesInQueue() > 60 * IO_BUFFER_SIZE) {
        if (g_pServer->m_bStopped || !pSession->IsOpened()) {
            return SOCKET_NOT_FOUND;
        }
        pSession->Wait();
        if (index != pSession->GetConnIndex())
            return SOCKET_NOT_FOUND;
    }
    if (!bMainThread && index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    return pSession->SendReturnData(usReqId, pBuffer, ulBufferSize);
}

unsigned int WINAPI GetSvsID(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetSvsID();
    return SPA::sidStartup;
}

int WINAPI GetServerSocketErrorCode(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetErrorCode();
    return 0;
}

void WINAPI EnableClientDequeue(USocket_Server_Handle h, bool enable) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->EnableClientDequeue(enable);
}

void WINAPI SetPeerDequeueFailed(USocket_Server_Handle h, bool fail) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        pSession->SetPeerDequeueFailed(fail);
}

bool WINAPI GetPeerDequeueFailed(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetPeerDequeueFailed();
    return false;
}

bool WINAPI IsDequeueRequest(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->IsDequeueRequest();
    return false;
}

bool WINAPI IsBatching(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->IsBatching();
    return false;
}

unsigned int WINAPI GetBytesBatched(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetBytesBatched();
    return 0;
}

bool WINAPI StartBatching(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->StartBatching();
    return false;
}

bool WINAPI CommitBatching(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        if (g_pServer->m_bStopped) {
            return false;
        }
        bool bMainThread = ::IsMainThread();
        while (!bMainThread && index == pSession->GetConnIndex() && pSession->GetSndBytesInQueue() > 60 * IO_BUFFER_SIZE && !::IsMainThread()) {
            if (g_pServer->m_bStopped || !pSession->IsOpened()) {
                return false;
            }
            pSession->Wait();
        }
        return pSession->CommitBatching();
    }
    return false;
}

bool WINAPI AbortBatching(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->AbortBatching();
    return true;
}

bool WINAPI IsCanceled(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->IsCanceled();
    return false;
}

void WINAPI UseUTF16() {
    SPA::g_bAdapterUTF16 = true;
}

bool WINAPI SetUserID(USocket_Server_Handle h, const wchar_t *userId) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        if (!userId) {
            pSession->SetUserID(L"");
        } else if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            const SPA::UTF16 *str = (const SPA::UTF16 *)userId;
            unsigned int len = SPA::Utilities::GetLen(str);
            std::wstring s = SPA::ToNativeString(str, len);
            pSession->SetUserID(s.c_str());
        } else
            pSession->SetUserID(userId);
        return true;
    }
    return false;
}

SPA::UINT64 WINAPI GetSocketNativeHandle(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetSocketNativeHandle();
    return INVALID_NUMBER;
}

unsigned int WINAPI GetUID(USocket_Server_Handle h, wchar_t *userId, unsigned int chars) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        if (!chars || !userId)
            return 0;
#ifdef WCHAR32
        else if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            SPA::CScopeUQueue su;
            SPA::CUQueue &q = *su;
            unsigned int bufferSize = (chars + 1) * sizeof (wchar_t);
            if (q.GetMaxSize() < bufferSize)
                q.ReallocBuffer(bufferSize);
            wchar_t *uid = (wchar_t *) q.GetBuffer();
            unsigned int len = pSession->GetUID(uid, chars);
            SPA::CScopeUQueue suUTF16;
            SPA::CUQueue &qUTF16 = *suUTF16;
            SPA::Utilities::ToUTF16(uid, len, qUTF16);
            ::memcpy(userId, qUTF16.GetBuffer(), len * sizeof (SPA::UTF16));
            return len;
        }
#endif
        return pSession->GetUID(userId, chars);
    }
    if (userId && chars) {
        if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16))
            *((SPA::UTF16*)userId) = 0;
        else
            userId[0] = 0;
    }
    return 0;
}

bool WINAPI SetPassword(USocket_Server_Handle h, const wchar_t *password) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        if (!password) {
            pSession->SetPassword(L"");
        } else if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            const SPA::UTF16 *str = (const SPA::UTF16 *)password;
            unsigned int len = SPA::Utilities::GetLen(str);
            std::wstring s = SPA::ToNativeString(str, len);
            pSession->SetPassword(s.c_str());
        } else
            pSession->SetPassword(password);
        return true;
    }
    return false;
}

unsigned int WINAPI GetPassword(USocket_Server_Handle h, wchar_t *password, unsigned int chars) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        if (!chars || !password)
            return 0;
#ifdef WCHAR32
        else if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16)) {
            SPA::CScopeUQueue su;
            SPA::CUQueue &q = *su;
            unsigned int bufferSize = (chars + 1) * sizeof (wchar_t);
            if (q.GetMaxSize() < bufferSize)
                q.ReallocBuffer(bufferSize);
            wchar_t *pwd = (wchar_t *) q.GetBuffer();
            unsigned int len = pSession->GetPassword(pwd, chars);
            SPA::CScopeUQueue suUTF16;
            SPA::CUQueue &qUTF16 = *suUTF16;
            SPA::Utilities::ToUTF16(pwd, len, qUTF16);
            ::memcpy(password, qUTF16.GetBuffer(), len * sizeof (SPA::UTF16));
            return len;
        }
#endif
        return pSession->GetPassword(password, chars);
    }
    if (password && chars) {
        if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16))
            *((SPA::UTF16*)password) = 0;
        else
            password[0] = 0;
    }
    return 0;
}

bool WINAPI SetZip(USocket_Server_Handle h, bool bZip) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex()) {
        unsigned int id = pSession->GetSvsID();
        if (SPA::sidStartup == id || SPA::sidHTTP == id)
            return false;
        pSession->SetZip(bZip);
        return true;
    }
    return false;
}

bool WINAPI GetZip(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetZip();
    return false;
}

void WINAPI SetZipLevel(USocket_Server_Handle h, SPA::tagZipLevel zl) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        pSession->SetZipLevel(zl);
}

SPA::tagZipLevel WINAPI GetZipLevel(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index != 0 && index == pSession->GetConnIndex())
        return pSession->GetZipLevel();
    return SPA::zlDefault;
}

unsigned int WINAPI GetServerSocketErrorMessage(USocket_Server_Handle h, char *str, unsigned int bufferLen) {
    if (str == nullptr || bufferLen == 0)
        return 0;
    if (bufferLen && str)
        str[0] = 0;
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return 0;
    --bufferLen; //for null-terminated
    if (bufferLen > 0) {
        std::string err = pSession->GetErrorMessage();
        if (bufferLen > (unsigned int) (err.size()))
            bufferLen = (unsigned int) (err.size());
        if (bufferLen > 0)
            ::memcpy(str, err.c_str(), bufferLen);
    }
    str[bufferLen] = 0;
    return bufferLen;
}

unsigned int WINAPI GetHTTPRequestHeaders(USocket_Server_Handle h, SPA::ServerSide::CHttpHeaderValue *HeaderValue, unsigned int count) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return 0;
    return pSession->GetHTTPRequestHeaders(HeaderValue, count);
}

const char* WINAPI GetHTTPPath(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return nullptr;
    return pSession->GetHTTPPath();
}

SPA::UINT64 WINAPI GetHTTPContentLength(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return 0;
    return pSession->GetHTTPContentLength();
}

const char* WINAPI GetHTTPQuery(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return nullptr;
    return pSession->GetHTTPQuery();
}

bool WINAPI DownloadFile(USocket_Server_Handle h, const char *filePath) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->DownloadFile(filePath);
}

SPA::ServerSide::tagHttpMethod WINAPI GetHTTPMethod(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SPA::ServerSide::hmUnknown;
    return pSession->GetHTTPMethod();
}

bool WINAPI HTTPKeepAlive(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->HTTPKeepAlive();
}

bool WINAPI IsWebSocket(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->IsWebSocket();
}

bool WINAPI IsCrossDomain(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->IsWebSocket();
}

double WINAPI GetHTTPVersion(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return 0.0;
    return pSession->GetHTTPVersion();
}

bool WINAPI HTTPGZipAccepted(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->HTTPGZipAccepted();
}

const char* WINAPI GetHTTPUrl(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return nullptr;
    return pSession->GetHTTPUrl();
}

const char* WINAPI GetHTTPHost(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return nullptr;
    return pSession->GetHTTPHost();
}

SPA::ServerSide::tagTransport WINAPI GetHTTPTransport(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SPA::ServerSide::tUnknown;
    return pSession->GetHTTPTransport();
}

SPA::ServerSide::tagTransferEncoding WINAPI GetHTTPTransferEncoding(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SPA::ServerSide::teUnknown;
    return pSession->GetHTTPTransferEncoding();
}

SPA::ServerSide::tagContentMultiplax WINAPI GetHTTPContentMultiplax(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SPA::ServerSide::cmUnknown;
    return pSession->GetHTTPContentMultiplax();
}

bool WINAPI SetHTTPResponseCode(USocket_Server_Handle h, unsigned int errCode) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->SetHTTPResponseCode(errCode);
}

const char* WINAPI GetHTTPId(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return nullptr;
    return pSession->GetHTTPId();
}

bool WINAPI IsFakeRequest(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->IsFakeRequest();
}

unsigned int WINAPI GetHTTPCurrentMultiplaxHeaders(USocket_Server_Handle h, SPA::ServerSide::CHttpHeaderValue *HeaderValue, unsigned int count) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return 0;
    return pSession->GetHTTPCurrentMultiplaxHeaders(HeaderValue, count);
}

bool WINAPI SetHTTPResponseHeader(USocket_Server_Handle h, const char *header, const char *value) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->SetHTTPResponseHeader(header, value);
}

unsigned int WINAPI SendHTTPReturnDataA(USocket_Server_Handle h, const char *str, unsigned int chars) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    if (SPA::sidHTTP != pSession->GetSvsID())
        return BAD_OPERATION;
    bool bMainThread = ::IsMainThread();
    if (g_pServer->m_bStopped) {
        return SOCKET_NOT_FOUND;
    }
    while (!bMainThread && pSession->GetSndBytesInQueue() > 60 * IO_BUFFER_SIZE) {
        if (g_pServer->m_bStopped || !pSession->IsOpened()) {
            return SOCKET_NOT_FOUND;
        }
        pSession->Wait();
        if (index != pSession->GetConnIndex())
            return SOCKET_NOT_FOUND;
    }
    if (!bMainThread && index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    return pSession->SendHTTPReturnDataA(str, chars);
}

unsigned int WINAPI SendHTTPReturnDataW(USocket_Server_Handle h, const wchar_t *str, unsigned int chars) {
    SPA::CScopeUQueue su;
    if (str) {
#ifdef WCHAR32
        if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16))
            SPA::Utilities::ToUTF8((const SPA::UTF16 *)str, SPA::Utilities::GetLen((const SPA::UTF16 *)str), *su);
        else
#endif
            SPA::Utilities::ToUTF8(str, ::wcslen(str), *su);
    }
    return SendHTTPReturnDataA(h, (const char*) su->GetBuffer(), su->GetSize());
}

unsigned int WINAPI HTTPCallbackA(USocket_Server_Handle h, const char *name, const char *str, unsigned int chars) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    if (SPA::sidHTTP != pSession->GetSvsID())
        return BAD_OPERATION;
    if (g_pServer->m_bStopped) {
        return SOCKET_NOT_FOUND;
    }
    bool bMainThread = ::IsMainThread();
    while (!bMainThread && pSession->GetSndBytesInQueue() > 60 * IO_BUFFER_SIZE) {
        if (g_pServer->m_bStopped) {
            return SOCKET_NOT_FOUND;
        }
        pSession->Wait();
        if (index != pSession->GetConnIndex())
            return SOCKET_NOT_FOUND;
    }
    if (!bMainThread && index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    return pSession->HTTPCallbackA(name, str, chars);
}

unsigned int WINAPI HTTPCallbackW(USocket_Server_Handle h, const char *name, const wchar_t *str, unsigned int chars) {
    SPA::CScopeUQueue su;
    if (str) {
        if (chars == (~0)) {
#ifdef WCHAR32
            if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16))
                chars = SPA::Utilities::GetLen((const SPA::UTF16 *)str);
            else
#endif
                chars = (unsigned int) ::wcslen(str);
        }
#ifdef WCHAR32
        if (SPA::g_bAdapterUTF16 && sizeof (wchar_t) != sizeof (SPA::UTF16))
            SPA::Utilities::ToUTF8((const SPA::UTF16 *)str, chars, *su);
        else
#endif
            SPA::Utilities::ToUTF8(str, chars, *su);
    }
    return HTTPCallbackA(h, name, (const char*) su->GetBuffer(), su->GetSize());
}

unsigned int WINAPI StartHTTPChunkResponse(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    if (g_pServer->m_bStopped) {
        return SOCKET_NOT_FOUND;
    }
    bool bMainThread = ::IsMainThread();
    while (!bMainThread && pSession->GetSndBytesInQueue() > 60 * IO_BUFFER_SIZE) {
        if (g_pServer->m_bStopped) {
            return SOCKET_NOT_FOUND;
        }
        pSession->Wait();
        if (index != pSession->GetConnIndex())
            return SOCKET_NOT_FOUND;
    }
    if (!bMainThread && index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    return pSession->StartChunkResponse();
}

unsigned int WINAPI SendHTTPChunk(USocket_Server_Handle h, const unsigned char *buffer, unsigned int len) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    if (g_pServer->m_bStopped) {
        return SOCKET_NOT_FOUND;
    }
    bool bMainThread = ::IsMainThread();
    while (!bMainThread && pSession->GetSndBytesInQueue() > 60 * IO_BUFFER_SIZE) {
        if (g_pServer->m_bStopped || !pSession->IsOpened()) {
            return SOCKET_NOT_FOUND;
        }
        pSession->Wait();
        if (index != pSession->GetConnIndex())
            return SOCKET_NOT_FOUND;
    }
    if (!bMainThread && index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    return pSession->SendChunk(buffer, len);
}

unsigned int WINAPI EndHTTPChunkResponse(USocket_Server_Handle h, const unsigned char *buffer, unsigned int len) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    if (g_pServer->m_bStopped) {
        return SOCKET_NOT_FOUND;
    }
    bool bMainThread = ::IsMainThread();
    while (!bMainThread && pSession->GetSndBytesInQueue() > 60 * IO_BUFFER_SIZE) {
        if (g_pServer->m_bStopped) {
            return SOCKET_NOT_FOUND;
        }
        pSession->Wait();
        if (index != pSession->GetConnIndex())
            return SOCKET_NOT_FOUND;
    }
    if (!bMainThread && index != pSession->GetConnIndex())
        return SOCKET_NOT_FOUND;
    return pSession->EndChunkResponse(buffer, len);
}

void* WINAPI GetSSL(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return nullptr;
    return pSession->GetSSL();
}

bool WINAPI GetClientInfo(USocket_Server_Handle h, SPA::CSwitchInfo *pClientInfo) {
    unsigned int index;
    if (!pClientInfo)
        return false;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->GetClientInfo(pClientInfo);
}

bool WINAPI GetServerInfo(USocket_Server_Handle h, SPA::CSwitchInfo *pServerInfo) {
    unsigned int index;
    if (!pServerInfo)
        return false;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->GetServerInfo(pServerInfo);
}

bool WINAPI SetServerInfo(USocket_Server_Handle h, SPA::CSwitchInfo *pServerInfo) {
    unsigned int index;
    if (!pServerInfo)
        return false;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->SetServerInfo(pServerInfo);
}

void WINAPI DropCurrentSlowRequest(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return;
    pSession->DropCurrentSlowRequest();
}

bool WINAPI GetSockAddr(USocket_Server_Handle h, unsigned int *sockPort, char *strIPAddrBuffer, unsigned short chars) {
    unsigned int index;
    if ((sockPort == nullptr && strIPAddrBuffer == nullptr) || !chars)
        return true;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->GetSockAddr(sockPort, strIPAddrBuffer, chars);
}

unsigned int WINAPI GetLocalName(char *localName, unsigned short chars) {
    unsigned short n;
    if (!localName || !chars)
        return 0;
    char str[1025] = {0};
    int res = ::gethostname(str, sizeof (str));
    if (res == -1)
        return false;
    unsigned short len = (unsigned short) ::strlen(str);
    for (n = 0; n < len && n < chars; ++n) {
        localName[n] = str[n];
    }
    return n;
}

bool WINAPI IsServerQueueIndexPossiblyCrashed() {
    if (CServerSession::m_pQLastIndex.get() == nullptr)
        return false;
    return CServerSession::m_pQLastIndex->IsCrashed();
}

void WINAPI AbortDequeuedMessage(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return;
    pSession->SetPeerDequeueFailed(true);
}

bool WINAPI IsDequeuedMessageAborted(USocket_Server_Handle h) {
    unsigned int index;
    CServerSession *pSession = GetSvrSession(h, index);
    if (index == 0 || index != pSession->GetConnIndex())
        return false;
    return pSession->GetPeerDequeueFailed();
}