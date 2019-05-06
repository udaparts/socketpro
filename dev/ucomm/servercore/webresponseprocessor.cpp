#include "webresponseProcessor.h"
#include "connectioncontext.h"
#include <assert.h>
#include "httpcontext.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "../core_shared/pinc/base64.h"

namespace UHTTP {

    std::string CWebResponseProcessor::HTTP_RESPONSE_METHOD = "method";
    std::string CWebResponseProcessor::HTTP_RESPONSE_MSG = "msg";
    std::string CWebResponseProcessor::HTTP_RESPONSE_IP = "ip";
    std::string CWebResponseProcessor::HTTP_RESPONSE_PORT = "port";
    std::string CWebResponseProcessor::HTTP_RESPONSE_SENDER = "sender";
    std::string CWebResponseProcessor::HTTP_RESPONSE_SERVICE_ID = "serviceId";
    std::string CWebResponseProcessor::HTTP_RESPONSE_REQUEST_ID = "reqId";

    CWebResponseProcessor::CWebResponseProcessor(CWebRequestProcessor *pWebRequestProcessor)
    : m_pWebRequestProcessor(pWebRequestProcessor) {
        assert(pWebRequestProcessor != nullptr);
        const UHttpRequest &ur = pWebRequestProcessor->GetUHttpRequest();
        m_nReqCount = ur.GetReqCount();
        if (m_nReqCount > 1) {
            m_bServerBatching = ur.IsServerBatching();
            m_bBatching = ur.IsBatching();
        } else {
            m_bServerBatching = false;
            m_bBatching = false;
        }
    }

    CWebResponseProcessor::~CWebResponseProcessor() {

    }

    CWebRequestProcessor* CWebResponseProcessor::GetRequestProcessor() const {
        return m_pWebRequestProcessor;
    }

    unsigned int CWebResponseProcessor::BounceBackEnter(unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response) {
        assert(m_pWebRequestProcessor->m_CurrentErrCode == seOk);
        SPA::UJsonDocument docRes;
        docRes.SetObject();
        unsigned int start = Response.GetSize();
        SetOwnBaseResponseInfo(docRes);
        docRes.AddMember(CHttpContext::SP_RESPONSE_RESULT.c_str(), SPA::MakeJsonValue(pGroups, count, docRes.GetAllocator()), docRes.GetAllocator());
        //MakeGroup(docRes, pGroups, count);
        docRes.AddMember(CHttpContext::SP_REQUEST_NAME.c_str(), CHttpContext::SP_REQUEST_ENTER.c_str(), docRes.GetAllocator());
        SetResponse(docRes, Response);
        return (Response.GetSize() - start);
    }

    unsigned int CWebResponseProcessor::BounceBackExit(unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response) {
        assert(m_pWebRequestProcessor->m_CurrentErrCode == seOk);
        SPA::UJsonDocument docRes;
        docRes.SetObject();
        unsigned int start = Response.GetSize();
        SetOwnBaseResponseInfo(docRes);
        docRes.AddMember(CHttpContext::SP_RESPONSE_RESULT.c_str(), SPA::MakeJsonValue(pGroups, count, docRes.GetAllocator()), docRes.GetAllocator());
        docRes.AddMember(CHttpContext::SP_REQUEST_NAME.c_str(), CHttpContext::SP_REQUEST_EXIT.c_str(), docRes.GetAllocator());
        SetResponse(docRes, Response);
        return (Response.GetSize() - start);
    }

    unsigned int CWebResponseProcessor::BounceBackSpeak(unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response) {
        assert(m_pWebRequestProcessor->m_CurrentErrCode == seOk);
        SPA::UJsonDocument docRes;
        docRes.SetObject();
        unsigned int start = Response.GetSize();
        SetOwnBaseResponseInfo(docRes);
        docRes.AddMember(CHttpContext::SP_RESPONSE_RESULT.c_str(), SPA::MakeJsonValue(pGroups, count, docRes.GetAllocator()), docRes.GetAllocator());
        docRes.AddMember(CHttpContext::SP_REQUEST_NAME.c_str(), CHttpContext::SP_REQUEST_SPEAK.c_str(), docRes.GetAllocator());
        SetResponse(docRes, Response);
        return (Response.GetSize() - start);
    }

    unsigned int CWebResponseProcessor::BounceBackSendUserMessage(SPA::CUQueue &Response) {
        assert(m_pWebRequestProcessor->m_CurrentErrCode == seOk);
        SPA::UJsonDocument docRes;
        docRes.SetObject();
        unsigned int start = Response.GetSize();
        SetOwnBaseResponseInfo(docRes);
        docRes.AddMember(CHttpContext::SP_REQUEST_NAME.c_str(), CHttpContext::SP_REQUEST_SENDUSERMESSAGE.c_str(), docRes.GetAllocator());
        SetResponse(docRes, Response);
        return (Response.GetSize() - start);
    }

    unsigned int CWebResponseProcessor::ProcessUserRequest(const char *res, SPA::CUQueue &Response) {
        unsigned int len;
        assert(m_pWebRequestProcessor->m_CurrentIndex > 0);
        assert(m_pWebRequestProcessor->m_CurrentErrCode == seOk);
        unsigned int start = Response.GetSize();
        SPA::UJsonDocument docRes;
        docRes.SetObject();
        SetOwnBaseResponseInfo(docRes);
        //!!!!docRes.AddMember(CHttpContext::SP_RESPONSE_RESULT.c_str(), SPA::MakeJsonValue(res, docRes.GetAllocator()), docRes.GetAllocator());

        //reduce one string memory copy
        docRes.AddMember(CHttpContext::SP_RESPONSE_RESULT.c_str(), res, docRes.GetAllocator());
        if (res) {
            len = (unsigned int) ::strlen(res) + 128;
            if (Response.GetTailSize() < len)
                Response.ReallocBuffer(Response.GetMaxSize() + len - Response.GetTailSize());
        }
        SetResponse(docRes, Response);
        return (Response.GetSize() - start);
    }

    void CWebResponseProcessor::ProcessPing(SPA::CUQueue &Response) {
        SPA::UJsonDocument docRes;
        docRes.SetObject();
        SetOwnBaseResponseInfo(docRes);
        SetResponse(docRes, Response);
    }

    void CWebResponseProcessor::ProcessBadRequest(SPA::CUQueue &Response, tagSpError se) {
        SPA::UJsonDocument docRes;
        const UHttpRequest &ur = m_pWebRequestProcessor->GetUHttpRequest();
        if (se == seOk)
            se = ur.ErrCode;
        docRes.SetObject();
        docRes.AddMember(CHttpContext::SP_REQUEST_CI.c_str(), ur.CallIndex, docRes.GetAllocator());
        if (se != seOk)
            docRes.AddMember(CHttpContext::SP_RESPONSE_CODE.c_str(), se, docRes.GetAllocator());
        docRes.AddMember(CHttpContext::HTTP_RESPONSE_SELF.c_str(), (unsigned int) 1, docRes.GetAllocator());
        CHttpContext *pHC = m_pWebRequestProcessor->GetHttpContext();
        pHC->AddResponseHeader(CHttpContext::Connection.c_str(), CHttpContext::SP_CONNECTION_CLOSE.c_str());
        UHTTP::CHttpContext *p = m_pWebRequestProcessor->GetHttpContext();
        if (p->IsWebSocket()) {
            SPA::UJsonWriter writer(Response);
            docRes.Accept(writer);
            Response.SetNull();
        } else {
            SPA::CScopeUQueue su;
            SPA::UJsonWriter writer(*su);
            docRes.Accept(writer);
            p->PrepareResponse(su->GetBuffer(), su->GetSize(), Response);
        }
    }

    std::string CWebResponseProcessor::ProcessHttpSwitch(bool AuthOk, const char *ipAddr, unsigned short port, SPA::CUQueue &Response) {
        const UHttpRequest &ur = m_pWebRequestProcessor->GetUHttpRequest();
        CHttpContext *pHC = m_pWebRequestProcessor->GetHttpContext();
        std::string id = GenerateId(ur);
        SPA::UJsonDocument docRes;
        if (!pHC->IsWebSocket()) {
            Connection::CConnectionContext::SharedPtr pCC = Connection::CConnectionContext::SeekConnectionContext(ur.Id);
            if (pCC)
                Connection::CConnectionContext::RemoveConnectionContext(ur.Id);
        }
        docRes.SetObject();
        docRes.AddMember(CHttpContext::SP_REQUEST_CI.c_str(), ur.CallIndex, docRes.GetAllocator());
        //docRes.AddMember(HTTP_RESPONSE_IP.c_str(), SPA::MakeJsonValue(ipAddr, docRes.GetAllocator()), docRes.GetAllocator());
        //docRes.AddMember(HTTP_RESPONSE_PORT.c_str(), port, docRes.GetAllocator());
        docRes.AddMember(CHttpContext::HTTP_RESPONSE_SELF.c_str(), 1, docRes.GetAllocator());

        if (AuthOk) {
            docRes.AddMember(UHTTP::CHttpContext::HTTP_RESPONSE_PT.c_str(), GetPt(), docRes.GetAllocator());
            docRes.AddMember(CHttpContext::SP_REQUEST_ID.c_str(), SPA::MakeJsonValue(id.c_str(), docRes.GetAllocator()), docRes.GetAllocator());
            if (!pHC->IsWebSocket()) {
                Connection::CConnectionContext::SharedPtr cc(new Connection::CConnectionContext);
                cc->UserId = ur.GetUserIdW();
                cc->Pt = GetPt();
                cc->IsGet = (pHC->GetMethod() == SPA::ServerSide::hmGet);
                const char *p = pHC->GetUserAgent();
                cc->IsOpera = (p && ::strstr(p, "Opera/"));
                Connection::CConnectionContext::AddConnectionContext(id, cc);
            }
        } else {
            id.clear();
            docRes.AddMember(CHttpContext::SP_REQUEST_ID.c_str(), SPA::MakeJsonValue("", docRes.GetAllocator()), docRes.GetAllocator());
            docRes.AddMember(CHttpContext::SP_RESPONSE_CODE.c_str(), seAuthenticationFailed, docRes.GetAllocator());
            pHC->AddResponseHeader(CHttpContext::Connection.c_str(), CHttpContext::SP_CONNECTION_CLOSE.c_str());
        }
        SetResponse(docRes, Response);
        return id;
    }

    std::string CWebResponseProcessor::GenerateId(const UHttpRequest &ur) {
        assert(ur.SpRequest == srSwitchTo);
        boost::uuids::random_generator rgen;
        boost::uuids::uuid u = rgen();
        unsigned char str[64] = {0};
        char out[64] = {0};
        memcpy(str, u.data, sizeof (u.data));
        ::srand((unsigned int) time(nullptr));
        str[sizeof (u.data)] = (unsigned char) (rand() % 256);
        SPA::CBase64::encode(str, sizeof (u.data) + 1, out);
        return out;
    }

    void CWebResponseProcessor::SetSenderInfo(SPA::UJsonDocument &docRes, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroup, unsigned int count) {
        SPA::CScopeUQueue su;
        if (sendUserId)
            SPA::Utilities::ToUTF8(sendUserId, ::wcslen(sendUserId), *su);
        docRes.AddMember(CHttpContext::HTTP_RESPONSE_SELF.c_str(), (unsigned int) 0, docRes.GetAllocator());
        docRes.AddMember(HTTP_RESPONSE_SENDER.c_str(), SPA::MakeJsonValue((const char*) su->GetBuffer(), docRes.GetAllocator()), docRes.GetAllocator());
        docRes.AddMember(HTTP_RESPONSE_SERVICE_ID.c_str(), senderServiceId, docRes.GetAllocator());
        //docRes.AddMember(HTTP_RESPONSE_IP.c_str(), SPA::MakeJsonValue(senderAddr, docRes.GetAllocator()), docRes.GetAllocator());
        //docRes.AddMember(HTTP_RESPONSE_PORT.c_str(), senderClientPort, docRes.GetAllocator());
        docRes.AddMember(UHTTP::CHttpContext::HTTP_JS_GROUPS.c_str(), SPA::MakeJsonValue(pGroup, count, docRes.GetAllocator()), docRes.GetAllocator());
    }

    void CWebResponseProcessor::SetOwnBaseResponseInfo(SPA::UJsonDocument &docRes) {
        docRes.AddMember(CHttpContext::SP_REQUEST_CI.c_str(), m_pWebRequestProcessor->m_CurrentIndex, docRes.GetAllocator());
        if (m_pWebRequestProcessor->m_CurrentErrCode != seOk)
            docRes.AddMember(CHttpContext::SP_RESPONSE_CODE.c_str(), m_pWebRequestProcessor->m_CurrentErrCode, docRes.GetAllocator());
        docRes.AddMember(CHttpContext::HTTP_RESPONSE_SELF.c_str(), 1, docRes.GetAllocator());
    }

    unsigned int CWebResponseProcessor::SendExceptionResult(const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode, SPA::CUQueue &Response) {
        SPA::CScopeUQueue su;
        SPA::UJsonDocument docRes;
        docRes.SetObject();
        unsigned int start = Response.GetSize();
        SetOwnBaseResponseInfo(docRes);
        docRes.AddMember("errCode", (int) errCode, docRes.GetAllocator());
        docRes.AddMember(HTTP_RESPONSE_REQUEST_ID.c_str(), requestId, docRes.GetAllocator());
        docRes.AddMember("stack", errWhere, docRes.GetAllocator());
        if (errMessage)
            SPA::Utilities::ToUTF8(errMessage, ::wcslen(errMessage), *su);
        docRes.AddMember("errMsg", (const char*) su->GetBuffer(), docRes.GetAllocator());
        SetResponse(docRes, Response);
        return (Response.GetSize() - start);
    }

    unsigned int CWebResponseProcessor::GetPt() {
        const CHttpContext *pHC = m_pWebRequestProcessor->GetHttpContext();
        if (!pHC->IsWebSocket())
            return DEFAULT_AJAX_SCRIPT_PING_TIME;
        return 30 * 1000; //DEFAULT_WEBSOCKET_PING_TIME;
    }

    void CWebResponseProcessor::ProcessClose(SPA::CUQueue &Response) {
        SPA::UJsonDocument docRes;
        CHttpContext *pHC = m_pWebRequestProcessor->GetHttpContext();
        if (!pHC->IsWebSocket())
            pHC->AddResponseHeader(CHttpContext::Connection.c_str(), CHttpContext::SP_CONNECTION_CLOSE.c_str());
        docRes.SetObject();
        SetOwnBaseResponseInfo(docRes);
        docRes.AddMember(CHttpContext::SP_REQUEST_NAME.c_str(), CHttpContext::SP_REQUEST_CLOSE.c_str(), docRes.GetAllocator());
        SetResponse(docRes, Response);
        if (!pHC->IsWebSocket())
            Connection::CConnectionContext::RemoveConnectionContext(m_pWebRequestProcessor->GetUHttpRequest().Id);
    }

    unsigned int CWebResponseProcessor::Enter(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response) {
        assert(false); //should not come here
        return 0;
    }

    unsigned int CWebResponseProcessor::Exit(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response) {
        assert(false); //should not come here
        return 0;
    }

    unsigned int CWebResponseProcessor::Speak(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response) {
        assert(false); //should not come here
        return 0;
    }

    unsigned int CWebResponseProcessor::SendUserMessage(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response) {
        assert(false); //should not come here
        return 0;
    }

    CJavaScriptResponseProcessor::CJavaScriptResponseProcessor(CJavaScriptRequestProcessor *pJavaScriptRequestProcessor)
    : CWebResponseProcessor(pJavaScriptRequestProcessor) {
        assert(pJavaScriptRequestProcessor->GetHttpContext()->GetTransport() == SPA::ServerSide::tScript);
    }

    void CJavaScriptResponseProcessor::SetResponse(SPA::UJsonDocument &docRes, SPA::CUQueue &Response) {
        SPA::CScopeUQueue su;
        if (Response.GetTailSize() > su->GetMaxSize())
            su->ReallocBuffer(Response.GetTailSize());
        WriteEnd(docRes, *su);
        su->SetNull();
        CHttpContext *p = m_pWebRequestProcessor->GetHttpContext();
        if (p->GetResponseProgress().Chunked) {
            p->SendChunkedData(su->GetBuffer(), su->GetSize(), Response);
            if (m_nReqCount == 0)
                p->SendChunkedData(nullptr, 0, Response);
        } else
            p->PrepareResponse(su->GetBuffer(), su->GetSize(), Response);
    }

    void CJavaScriptResponseProcessor::WriteEnd(SPA::UJsonDocument &docRes, SPA::CUQueue &Response) {
        const UHttpRequest &ur = m_pWebRequestProcessor->GetUHttpRequest();
        Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(ur.Id);
        if (ur.SpRequest != UHTTP::srSwitchTo) {
            if (sp) {
                std::vector<std::string> &responses = sp->Responses;
                std::vector<std::string>::iterator it, end = responses.end();
                for (it = responses.begin(); it != end; ++it) {
                    Response.Push(UHTTP::CHttpContext::HTTP_JS_CALLBACK_HEAD.c_str(), (unsigned int) UHTTP::CHttpContext::HTTP_JS_CALLBACK_HEAD.size());
                    Response.Push(it->c_str());
                    Response.Push(UHTTP::CHttpContext::HTTP_JS_CALLBACK_END.c_str(), (unsigned int) UHTTP::CHttpContext::HTTP_JS_CALLBACK_END.size());
                }
                responses.clear();
            }
        }
        Response.Push(UHTTP::CHttpContext::HTTP_JS_CALLBACK_HEAD.c_str(), (unsigned int) UHTTP::CHttpContext::HTTP_JS_CALLBACK_HEAD.size());
        SPA::UJsonWriter writer(Response);
        docRes.Accept(writer);
        Response.Push(UHTTP::CHttpContext::HTTP_JS_CALLBACK_END.c_str(), (unsigned int) UHTTP::CHttpContext::HTTP_JS_CALLBACK_END.size());
        Response.SetNull();
    }

    CAjaxResponseProcessor::CAjaxResponseProcessor(CAjaxRequestProcessor *pAjaxRequestProcessor)
    : CWebResponseProcessor(pAjaxRequestProcessor) {
        assert(pAjaxRequestProcessor->GetHttpContext()->GetTransport() == SPA::ServerSide::tAjax);
    }

    void CAjaxResponseProcessor::SetResponse(SPA::UJsonDocument &docRes, SPA::CUQueue &Response) {
        SPA::CScopeUQueue su;
        if (Response.GetTailSize() > su->GetMaxSize())
            su->ReallocBuffer(Response.GetTailSize());
        WriteEnd(docRes, *su);
        if (m_nReqCount == 0) {
            CHttpContext *p = m_pWebRequestProcessor->GetHttpContext();
            p->PrepareResponse(su->GetBuffer(), su->GetSize(), Response);
        }
    }

    void CAjaxResponseProcessor::WriteEnd(SPA::UJsonDocument &docRes, SPA::CUQueue &Response) {
        const UHttpRequest &ur = m_pWebRequestProcessor->GetUHttpRequest();
        Connection::CConnectionContext::SharedPtr sp = Connection::CConnectionContext::SeekConnectionContext(ur.Id);
        if (ur.SpRequest != UHTTP::srSwitchTo) {
            if (m_nReqCount == 0) {
                Response.Push("{\"rb\":[", 7);
                if (sp) {
                    sp->Responses.push_back(Connection::CConnectionContext::ToString(docRes, (unsigned int) (docRes.GetAllocator().Size() + 1024)));
                    std::vector<std::string>::iterator it, end = sp->Responses.end();
                    for (it = sp->Responses.begin(); it != end; ++it) {
                        if (it != sp->Responses.begin())
                            Response.Push(",", 1);
                        Response.Push(it->c_str(), (unsigned int) it->size());
                    }
                }
                Response.Push("]}", 2);
                Response.SetNull();
                if (sp)
                    sp->Responses.clear();
            } else if (sp) {
                sp->Responses.push_back(Connection::CConnectionContext::ToString(docRes, (unsigned int) (docRes.GetAllocator().Size() + 1024)));
                return;
            }
        } else {
            Connection::CConnectionContext::ToString(docRes, (unsigned int) (docRes.GetAllocator().Size() + 1024), Response);
        }
    }

    CWebSocketResponseProcessor::CWebSocketResponseProcessor(CWebSocketRequestProcessor *pWebSocketRequestProcessor)
    : CWebResponseProcessor(pWebSocketRequestProcessor) {
        assert(pWebSocketRequestProcessor->GetHttpContext()->GetTransport() == SPA::ServerSide::tWebSocket);
    }

    void CWebSocketResponseProcessor::WriteEnd(SPA::UJsonDocument &docRes, SPA::CUQueue &Response) {
        SPA::CScopeUQueue su;
        if (Response.GetTailSize() > su->GetMaxSize())
            su->ReallocBuffer(Response.GetTailSize());
        SPA::UJsonWriter writer(*su);
        docRes.Accept(writer);
        GetRequestProcessor()->GetHttpContext()->PrepareWSResponseMessage(su->GetBuffer(), su->GetSize(), ocTextMsg, Response);
    }

    void CWebSocketResponseProcessor::SetResponse(SPA::UJsonDocument &docRes, SPA::CUQueue &Response) {
        WriteEnd(docRes, Response);
        Response.SetNull();
    }

    unsigned int CWebSocketResponseProcessor::Enter(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response) {
        unsigned int start = Response.GetSize();
        SPA::UJsonDocument docRes;
        docRes.SetObject();
        SetSenderInfo(docRes, senderAddr, senderClientPort, sendUserId, senderServiceId, pGroups, count);
        docRes.AddMember(CHttpContext::SP_REQUEST_NAME.c_str(), CHttpContext::SP_REQUEST_ENTER.c_str(), docRes.GetAllocator());
        SetResponse(docRes, Response);
        return (Response.GetSize() - start);
    }

    unsigned int CWebSocketResponseProcessor::Exit(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response) {
        unsigned int start = Response.GetSize();
        SPA::UJsonDocument docRes;
        docRes.SetObject();
        SetSenderInfo(docRes, senderAddr, senderClientPort, sendUserId, senderServiceId, pGroups, count);
        docRes.AddMember(CHttpContext::SP_REQUEST_NAME.c_str(), CHttpContext::SP_REQUEST_EXIT.c_str(), docRes.GetAllocator());
        SetResponse(docRes, Response);
        return (Response.GetSize() - start);
    }

    unsigned int CWebSocketResponseProcessor::Speak(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response) {
        unsigned int start = Response.GetSize();
        SPA::UJsonDocument docRes;
        docRes.SetObject();
        SetSenderInfo(docRes, senderAddr, senderClientPort, sendUserId, senderServiceId, pGroups, count);
        docRes.AddMember(CHttpContext::SP_REQUEST_NAME.c_str(), CHttpContext::SP_REQUEST_SPEAK.c_str(), docRes.GetAllocator());
        docRes.AddMember(HTTP_RESPONSE_MSG.c_str(), SPA::MakeJsonValue(vtMsg, docRes.GetAllocator()), docRes.GetAllocator());
        SetResponse(docRes, Response);
        return (Response.GetSize() - start);
    }

    unsigned int CWebSocketResponseProcessor::SendUserMessage(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response) {
        unsigned int start = Response.GetSize();
        SPA::UJsonDocument docRes;
        docRes.SetObject();
        SetSenderInfo(docRes, senderAddr, senderClientPort, sendUserId, senderServiceId, pGroups, count);
        docRes.AddMember(CHttpContext::SP_REQUEST_NAME.c_str(), CHttpContext::SP_REQUEST_SENDUSERMESSAGE.c_str(), docRes.GetAllocator());
        docRes.AddMember(HTTP_RESPONSE_MSG.c_str(), SPA::MakeJsonValue(vtMsg, docRes.GetAllocator()), docRes.GetAllocator());
        SetResponse(docRes, Response);
        return (Response.GetSize() - start);
    }
}
