
#include "stdafx.h"
#include "connectioncontext.h"
#include "httpcontext.h"
#include "webresponseProcessor.h"

namespace Connection {

    boost::unordered_map<std::string, CConnectionContext::SharedPtr> CConnectionContext::m_mapCC;
    SPA::CUCriticalSection CConnectionContext::m_cs;

    CConnectionContextBase::CConnectionContextBase()
    : m_ulRead(0),
    m_ulSent(0),
    m_bBatching(false),
    Pt(60000), IsOpera(false), IsGet(false) {
        ::memset(&SvsContext, 0, sizeof (CSvsContext));
    }

    void CConnectionContextBase::ChatGroupsAnd(const unsigned int *p0, unsigned int count0, const unsigned int *p1, unsigned int count1, std::vector<unsigned int> &vOut) {
        unsigned int j, k;
        vOut.clear();
        if (p0 == nullptr)
            count0 = 0;
        if (p1 == nullptr)
            count1 = 0;
        for (j = 0; j < count0; ++j) {
            if (p0[j] == 0 || std::find(vOut.begin(), vOut.end(), p0[j]) != vOut.end())
                continue;
            for (k = 0; k < count1; ++k) {
                if (p0[j] == p1[k]) {
                    vOut.push_back(p0[j]);
                    break;
                }
            }
        }
    }

    void CConnectionContextBase::ChatGroupsOr(const unsigned int *p0, unsigned int count0, const unsigned int *p1, unsigned int count1, std::vector<unsigned int> &vOut) {
        unsigned int n;
        vOut.clear();
        if (p0 == nullptr)
            count0 = 0;
        if (p1 == nullptr)
            count1 = 0;
        for (n = 0; n < count0; ++n) {
            if (p0[n] == 0 || std::find(vOut.begin(), vOut.end(), p0[n]) != vOut.end())
                continue;
            vOut.push_back(p0[n]);
        }
        for (n = 0; n < count1; ++n) {
            if (p1[n] == 0 || std::find(vOut.begin(), vOut.end(), p1[n]) != vOut.end())
                continue;
            vOut.push_back(p1[n]);
        }
    }

    void CConnectionContextBase::ChatGroupsNew(const unsigned int *p0, unsigned int count0, const unsigned int *p1, unsigned int count1, std::vector<unsigned int> &vAdd, std::vector<unsigned int> &vExit) {
        unsigned int n;
        vAdd.clear();
        vExit.clear();
        if (p0 == nullptr)
            count0 = 0;
        if (p1 == nullptr)
            count1 = 0;
        if (count1 == 0 && count0 == 0)
            return;
        vAdd.assign(p1, p1 + count1);
        std::vector<unsigned int>::iterator it;
        for (n = 0; n < count0; ++n) {
            it = std::find(vAdd.begin(), vAdd.end(), p0[n]);
            if (it != vAdd.end())
                vAdd.erase(it);
            else
                vExit.push_back(p0[n]);
        }
    }

    CConnectionContext::CConnectionContext() {

    }

    void CConnectionContext::AddConnectionContext(const std::string &id, SharedPtr cc) {
        assert(cc);
        cc->Id = id;
        m_cs.lock();
        m_mapCC[id] = cc;
        m_cs.unlock();
    }

    void CConnectionContext::RemoveConnectionContext(const char *id) {
        m_cs.lock();
        m_mapCC.erase(id);
        m_cs.unlock();
    }

    CConnectionContext::SharedPtr CConnectionContext::SeekConnectionContext(const char *id) {
        SPA::CAutoLock al(m_cs);
        boost::unordered_map<std::string, CConnectionContext::SharedPtr>::iterator it = m_mapCC.find(id);
        if (it == m_mapCC.end())
            return SharedPtr();
        return it->second;
    }

    void CConnectionContext::ToString(SPA::UJsonValue &jv, unsigned int approSize, SPA::CUQueue &q) {
        if (approSize > q.GetTailSize())
            q.ReallocBuffer(q.GetMaxSize() + approSize - q.GetTailSize());
        SPA::UJsonWriter writer(q);
        jv.Accept(writer);
        q.SetNull();
    }

    std::string CConnectionContext::ToString(SPA::UJsonValue &jv, unsigned int approSize) {
        SPA::CScopeUQueue su;
        if (approSize > su->GetMaxSize())
            su->ReallocBuffer(approSize);
        SPA::UJsonWriter writer(*su);
        jv.Accept(writer);
        su->SetNull();
        return (const char*) su->GetBuffer();
    }

    void CConnectionContext::SetSenderInfo(SPA::UJsonDocument &jv, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroup, unsigned int count) {
        SPA::CScopeUQueue su;
        if (sendUserId)
            SPA::Utilities::ToUTF8(sendUserId, ::wcslen(sendUserId), *su);
        jv.AddMember(UHTTP::CHttpContext::HTTP_RESPONSE_SELF.c_str(), (unsigned int) 0, jv.GetAllocator());
        jv.AddMember(UHTTP::CWebResponseProcessor::HTTP_RESPONSE_SENDER.c_str(), SPA::MakeJsonValue((const char*) su->GetBuffer(), jv.GetAllocator()), jv.GetAllocator());
        jv.AddMember(UHTTP::CWebResponseProcessor::HTTP_RESPONSE_SERVICE_ID.c_str(), senderServiceId, jv.GetAllocator());
        //jv.AddMember(UHTTP::CWebResponseProcessor::HTTP_RESPONSE_IP.c_str(), SPA::MakeJsonValue(senderAddr, jv.GetAllocator()), jv.GetAllocator());
        //jv.AddMember(UHTTP::CWebResponseProcessor::HTTP_RESPONSE_PORT.c_str(), senderClientPort, jv.GetAllocator());
        jv.AddMember(UHTTP::CHttpContext::HTTP_JS_GROUPS.c_str(), SPA::MakeJsonValue(pGroup, count, jv.GetAllocator()), jv.GetAllocator());
    }

    unsigned int CConnectionContext::SendWSResult(unsigned short reqId, const char *res, unsigned int len, SPA::CUQueue &q, const char *callback) {
        if (!res)
            res = "";
        SPA::CScopeUQueue su;
        SPA::UJsonDocument jv;
        unsigned int start = q.GetSize();
        jv.SetObject();
        if (callback)
            jv.AddMember("cbk", callback, jv.GetAllocator());
        jv.AddMember(UHTTP::CWebResponseProcessor::HTTP_RESPONSE_REQUEST_ID.c_str(), reqId, jv.GetAllocator());
        jv.AddMember(UHTTP::CHttpContext::HTTP_RESPONSE_SELF.c_str(), (unsigned int) 1, jv.GetAllocator());
        jv.AddMember(UHTTP::CHttpContext::SP_RESPONSE_RESULT.c_str(), res, jv.GetAllocator());
        if (len == (~0))
            len = (unsigned int) ::strlen(res);
        ToString(jv, len, q);
        return (q.GetSize() - start);
    }

    unsigned int CConnectionContext::SendResult(const std::string &id, unsigned short reqId, const char *res, unsigned int len, const char *callback) {
        if (!res)
            res = "";
        SPA::CAutoLock al(m_cs);
        boost::unordered_map<std::string, CConnectionContext::SharedPtr>::iterator it = m_mapCC.find(id);
        if (it != m_mapCC.end()) {
            SPA::CScopeUQueue su;
            CConnectionContext::SharedPtr p = it->second;
            SPA::UJsonDocument jv;
            jv.SetObject();
            if (callback)
                jv.AddMember("cbk", callback, jv.GetAllocator());
            jv.AddMember(UHTTP::CWebResponseProcessor::HTTP_RESPONSE_REQUEST_ID.c_str(), reqId, jv.GetAllocator());
            jv.AddMember(UHTTP::CHttpContext::HTTP_RESPONSE_SELF.c_str(), (unsigned int) 1, jv.GetAllocator());
            jv.AddMember(UHTTP::CHttpContext::SP_RESPONSE_RESULT.c_str(), res, jv.GetAllocator());
            if (len == (~0))
                len = (unsigned int) ::strlen(res);
            std::string str = ToString(jv, len + 16);
            p->Responses.push_back(str);
            return len;
        }
        return RESULT_SENDING_FAILED;
    }

    void CConnectionContext::Exit(const std::string &id, const unsigned int *pGroups, unsigned int count) {
        SPA::CAutoLock al(m_cs);
        boost::unordered_map<std::string, CConnectionContext::SharedPtr>::iterator it = m_mapCC.find(id);
        if (it != m_mapCC.end()) {
            CConnectionContext::SharedPtr p = it->second;
            SPA::UJsonDocument jv;
            jv.SetObject();
            jv.AddMember(UHTTP::CHttpContext::SP_REQUEST_NAME.c_str(), UHTTP::CHttpContext::SP_REQUEST_EXIT.c_str(), jv.GetAllocator());
            jv.AddMember(UHTTP::CHttpContext::HTTP_RESPONSE_SELF.c_str(), (unsigned int) 1, jv.GetAllocator());
            jv.AddMember(UHTTP::CHttpContext::SP_RESPONSE_RESULT.c_str(), SPA::MakeJsonValue(pGroups, count, jv.GetAllocator()), jv.GetAllocator());
            p->Responses.push_back(ToString(jv, 1024));
        }
    }

    void CConnectionContext::Speak(const std::string &id, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count) {
        SPA::CAutoLock al(m_cs);
        boost::unordered_map<std::string, CConnectionContext::SharedPtr>::iterator it = m_mapCC.find(id);
        if (it != m_mapCC.end()) {
            CConnectionContext::SharedPtr p = it->second;
            SPA::UJsonDocument jv;
            jv.SetObject();
            jv.AddMember(UHTTP::CHttpContext::SP_REQUEST_NAME.c_str(), UHTTP::CHttpContext::SP_REQUEST_SPEAK.c_str(), jv.GetAllocator());
            jv.AddMember(UHTTP::CHttpContext::HTTP_RESPONSE_SELF.c_str(), (unsigned int) 1, jv.GetAllocator());
            //jv.AddMember(UHTTP::CWebResponseProcessor::HTTP_RESPONSE_MSG.c_str(), SPA::MakeJsonValue(vtMsg, jv.GetAllocator()), jv.GetAllocator());
            jv.AddMember(UHTTP::CHttpContext::SP_RESPONSE_RESULT.c_str(), SPA::MakeJsonValue(pGroups, count, jv.GetAllocator()), jv.GetAllocator());
            p->Responses.push_back(ToString(jv, 1024));
        }
    }

    unsigned int CConnectionContext::SendExceptionResult(const std::string &id, const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode) {
        SPA::CAutoLock al(m_cs);
        boost::unordered_map<std::string, CConnectionContext::SharedPtr>::iterator it = m_mapCC.find(id);
        if (it != m_mapCC.end()) {
            SPA::CScopeUQueue su;
            CConnectionContext::SharedPtr p = it->second;
            SPA::UJsonDocument jv;
            jv.SetObject();
            jv.AddMember(UHTTP::CHttpContext::HTTP_RESPONSE_SELF.c_str(), 1, jv.GetAllocator());
            jv.AddMember("errCode", (int) errCode, jv.GetAllocator());
            jv.AddMember(UHTTP::CWebResponseProcessor::HTTP_RESPONSE_REQUEST_ID.c_str(), requestId, jv.GetAllocator());
            jv.AddMember("stack", errWhere, jv.GetAllocator());
            if (errMessage)
                SPA::Utilities::ToUTF8(errMessage, ::wcslen(errMessage), *su);
            const char *s = (const char*) su->GetBuffer();
            jv.AddMember("errMsg", s, jv.GetAllocator());
            std::string str = ToString(jv, (unsigned int) (::strlen(s) + 100));
            unsigned int len = (unsigned int) str.size();
            p->Responses.push_back(str);
            return len;
        }
        return RESULT_SENDING_FAILED;
    }

    void CConnectionContext::SendUserMessage(const std::string &id, const SPA::UVariant &vtMsg, const wchar_t *receiver, const unsigned int *pGroups, unsigned int count) {
        SPA::CAutoLock al(m_cs);
        boost::unordered_map<std::string, CConnectionContext::SharedPtr>::iterator it = m_mapCC.find(id);
        if (it != m_mapCC.end()) {
            CConnectionContext::SharedPtr p = it->second;
            SPA::UJsonDocument jv;
            jv.SetObject();
            jv.AddMember(UHTTP::CHttpContext::SP_REQUEST_NAME.c_str(), UHTTP::CHttpContext::SP_REQUEST_SENDUSERMESSAGE.c_str(), jv.GetAllocator());
            jv.AddMember(UHTTP::CHttpContext::HTTP_RESPONSE_SELF.c_str(), (unsigned int) 1, jv.GetAllocator());
            //jv.AddMember(UHTTP::CWebResponseProcessor::HTTP_RESPONSE_MSG.c_str(), SPA::MakeJsonValue(vtMsg, jv.GetAllocator()), jv.GetAllocator());
            jv.AddMember(UHTTP::CHttpContext::SP_RESPONSE_RESULT.c_str(), SPA::MakeJsonValue(pGroups, count, jv.GetAllocator()), jv.GetAllocator());
            p->Responses.push_back(ToString(jv, 1024));
        }
    }

    void CConnectionContext::Enter(const std::string &id, const unsigned int *pGroups, unsigned int count) {
        SPA::CAutoLock al(m_cs);
        boost::unordered_map<std::string, CConnectionContext::SharedPtr>::iterator it = m_mapCC.find(id);
        if (it != m_mapCC.end()) {
            CConnectionContext::SharedPtr p = it->second;
            SPA::UJsonDocument jv;
            jv.SetObject();
            jv.AddMember(UHTTP::CHttpContext::SP_REQUEST_NAME.c_str(), UHTTP::CHttpContext::SP_REQUEST_ENTER.c_str(), jv.GetAllocator());
            jv.AddMember(UHTTP::CHttpContext::HTTP_RESPONSE_SELF.c_str(), (unsigned int) 1, jv.GetAllocator());
            jv.AddMember(UHTTP::CHttpContext::SP_RESPONSE_RESULT.c_str(), SPA::MakeJsonValue(pGroups, count, jv.GetAllocator()), jv.GetAllocator());
            p->Responses.push_back(ToString(jv, 1024));
        }
    }

    void CConnectionContext::Enter(const std::string &id, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count) {
        std::vector<unsigned int> vChatGroup;
        SPA::CAutoLock al(m_cs);
        boost::unordered_map<std::string, CConnectionContext::SharedPtr>::iterator it, end = m_mapCC.end();
        for (it = m_mapCC.begin(); it != end; ++it) {
            if (it->first == id)
                continue;
            SharedPtr p = it->second;
            std::vector<unsigned int> &groups = p->ChatGroups;
            ChatGroupsAnd(pGroups, count, groups.data(), (unsigned int) groups.size(), vChatGroup);
            if (vChatGroup.size()) {
                SPA::UJsonDocument jv;
                jv.SetObject();
                p->SetSenderInfo(jv, senderAddr, senderClientPort, sendUserId, senderServiceId, vChatGroup.data(), (unsigned int) vChatGroup.size());
                jv.AddMember(UHTTP::CHttpContext::SP_REQUEST_NAME.c_str(), UHTTP::CHttpContext::SP_REQUEST_ENTER.c_str(), jv.GetAllocator());
                p->Responses.push_back(ToString(jv, 1024));
            }
        }
    }

    void CConnectionContext::Exit(const std::string &id, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count) {
        std::vector<unsigned int> vChatGroup;
        SPA::CAutoLock al(m_cs);
        boost::unordered_map<std::string, CConnectionContext::SharedPtr>::iterator it, end = m_mapCC.end();
        for (it = m_mapCC.begin(); it != end; ++it) {
            if (it->first == id)
                continue;
            SharedPtr p = it->second;
            std::vector<unsigned int> &groups = p->ChatGroups;
            ChatGroupsAnd(pGroups, count, groups.data(), (unsigned int) groups.size(), vChatGroup);
            if (vChatGroup.size()) {
                SPA::UJsonDocument jv;
                jv.SetObject();
                p->SetSenderInfo(jv, senderAddr, senderClientPort, sendUserId, senderServiceId, vChatGroup.data(), (unsigned int) vChatGroup.size());
                jv.AddMember(UHTTP::CHttpContext::SP_REQUEST_NAME.c_str(), UHTTP::CHttpContext::SP_REQUEST_EXIT.c_str(), jv.GetAllocator());
                p->Responses.push_back(ToString(jv, 1024));
            }
        }
    }

    void CConnectionContext::Speak(const std::string &id, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count) {
        std::vector<unsigned int> vChatGroup;
        SPA::CAutoLock al(m_cs);
        boost::unordered_map<std::string, CConnectionContext::SharedPtr>::iterator it, end = m_mapCC.end();
        for (it = m_mapCC.begin(); it != end; ++it) {
            if (it->first == id)
                continue;
            SharedPtr p = it->second;
            std::vector<unsigned int> &groups = p->ChatGroups;
            ChatGroupsAnd(pGroups, count, groups.data(), (unsigned int) groups.size(), vChatGroup);
            if (vChatGroup.size()) {
                SPA::UJsonDocument jv;
                jv.SetObject();
                p->SetSenderInfo(jv, senderAddr, senderClientPort, sendUserId, senderServiceId, vChatGroup.data(), (unsigned int) vChatGroup.size());
                jv.AddMember(UHTTP::CHttpContext::SP_REQUEST_NAME.c_str(), UHTTP::CHttpContext::SP_REQUEST_SPEAK.c_str(), jv.GetAllocator());
                jv.AddMember(UHTTP::CWebResponseProcessor::HTTP_RESPONSE_MSG.c_str(), SPA::MakeJsonValue(vtMsg, jv.GetAllocator()), jv.GetAllocator());
                p->Responses.push_back(ToString(jv, 4 * 1024));
            }
        }
    }

    void CConnectionContext::SendUserMessage(const std::string &id, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const wchar_t *receiver, const unsigned int *pGroups, unsigned int count) {
        SPA::CAutoLock al(m_cs);
        boost::unordered_map<std::string, CConnectionContext::SharedPtr>::iterator it, end = m_mapCC.end();
        for (it = m_mapCC.begin(); it != end; ++it) {
            if (it->first == id)
                continue;
            const SharedPtr& p = it->second;
            const std::wstring &s = p->UserId;
            if (UHTTP::iequals(s.c_str(), receiver)) {
                SPA::UJsonDocument jv;
                jv.SetObject();
                p->SetSenderInfo(jv, senderAddr, senderClientPort, sendUserId, senderServiceId, pGroups, count);
                jv.AddMember(UHTTP::CHttpContext::SP_REQUEST_NAME.c_str(), UHTTP::CHttpContext::SP_REQUEST_SENDUSERMESSAGE.c_str(), jv.GetAllocator());
                jv.AddMember(UHTTP::CWebResponseProcessor::HTTP_RESPONSE_MSG.c_str(), SPA::MakeJsonValue(vtMsg, jv.GetAllocator()), jv.GetAllocator());
                p->Responses.push_back(ToString(jv, 4 * 1024));
            }
        }
    }
};