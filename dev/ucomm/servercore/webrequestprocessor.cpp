#include "webrequestprocessor.h"
#include "../core_shared/pinc/base64.h"
#include <boost/uuid/uuid_generators.hpp>
#include "connectioncontext.h"
#include "../core_shared/shared/streamhead.h"
#include "httpcontext.h"

namespace UHTTP
{

    CWebRequestProcessor::CWebRequestProcessor(CHttpContext * pHttpContext)
            : m_CurrentIndex(0), m_CurrentErrCode(seOk), m_pHttpContext(pHttpContext) {
        ::memset(&m_ur, 0, sizeof (m_ur));
    }

    CWebRequestProcessor::~CWebRequestProcessor() {

    }

    CHttpContext * CWebRequestProcessor::GetHttpContext() const {
        return m_pHttpContext;
    }

    const SPA::UJsonValue & CWebRequestProcessor::GetDoc() {
        return m_doc;
    }

    void CWebRequestProcessor::RemoveStoredResponses() const {
        Connection::CConnectionContext::RemoveConnectionContext(m_ur.Id);
    }

    std::string CWebRequestProcessor::GenerateId() {
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

    UHttpRequest CWebRequestProcessor::ProcessChildRequest(const SPA::UJsonValue & req) {
        UHttpRequest UReq;
        UReq.SpRequest = srUnknown;
        if (!req.is_object()) {
            UReq.ErrCode = seUnexpectedRequest;
            return UReq;
        }
        const SPA::UJsonObject& doc = req.as_object();
        size_t size = doc.size();
        if (size != 5) {
            UReq.ErrCode = seBadNumberOfArgs;
            return UReq;
        }

        if (!doc.contains(CHttpContext::SP_REQUEST_NAME) ||
                !doc.contains(CHttpContext::SP_REQUEST_CI) ||
                !doc.contains(CHttpContext::SP_REQUEST_VERSION) ||
                !doc.contains(CHttpContext::SP_REQUEST_ARGS) ||
                !doc.contains(CHttpContext::SP_REQUEST_ID)
                ) {
            UReq.ErrCode = seBadArgs;
            return UReq;
        }

        const SPA::UJsonValue &reqName = doc.at(CHttpContext::SP_REQUEST_NAME);
        if (!reqName.is_string()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }

        UReq.ReqName = reqName.as_string().c_str();
        UReq.SpRequest = MapRequest(UReq.ReqName);

        const SPA::UJsonValue &id = doc.at(CHttpContext::SP_REQUEST_ID);
        if (!id.is_string()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }
        UReq.Id = id.as_string().c_str();

        const SPA::UJsonValue &ci = doc.at(CHttpContext::SP_REQUEST_CI);
        if (ci.is_int64()) {
            UReq.CallIndex = ci.as_int64();
        } else if (ci.is_uint64()) {
            UReq.CallIndex = (SPA::INT64)ci.as_uint64();
        } else {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }

        const SPA::UJsonValue &v = doc.at(CHttpContext::SP_REQUEST_VERSION);
        if (!v.is_double()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }
        UReq.Version = v.as_double();

        const SPA::UJsonValue &args = doc.at(CHttpContext::SP_REQUEST_ARGS);
        if (!args.is_array()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }

        if (UReq.IsBatching() || UReq.SpRequest == srClose || UReq.SpRequest == srPing || UReq.SpRequest == srSwitchTo) {
            UReq.ErrCode = seUnexpectedRequest;
            return UReq;
        }

        UReq.Args = &args;
        UReq.ErrCode = seOk;
        return UReq;
    }

    void CWebRequestProcessor::ShrinkMemory() {

    }

    tagSpRequest CWebRequestProcessor::MapRequest(const char *reqName) {
        if (!reqName)
            return srUnknown;
        if (strcmp(CHttpContext::SP_REQUEST_SWITCHTO.c_str(), reqName) == 0)
            return srSwitchTo;
        if (strcmp(CHttpContext::SP_REQUEST_DOBATCH.c_str(), reqName) == 0)
            return srDoBatch;
        if (strcmp(CHttpContext::SP_REQUEST_ENTER.c_str(), reqName) == 0)
            return srEnter;
        if (strcmp(CHttpContext::SP_REQUEST_EXIT.c_str(), reqName) == 0)
            return srExit;
        if (strcmp(CHttpContext::SP_REQUEST_SPEAK.c_str(), reqName) == 0)
            return srSpeak;
        if (strcmp(CHttpContext::SP_REQUEST_SENDUSERMESSAGE.c_str(), reqName) == 0)
            return srSendUserMessage;
        if (strcmp(CHttpContext::SP_REQUEST_PING.c_str(), reqName) == 0)
            return srPing;
        if (strcmp(CHttpContext::SP_REQUEST_CLOSE.c_str(), reqName) == 0)
            return srClose;
        return srRequest;
    }

    void CWebRequestProcessor::MakeChatGroup(const SPA::UJsonValue &groups, bool bEnter, SPA::CUQueue & q) {
        SPA::CScopeUQueue su;
        if (groups.is_array()) {
            const SPA::UJsonArray& arr = groups.as_array();
            unsigned int m = (unsigned int) arr.size();
            for (unsigned int n = 0; n < m; ++n) {
                const SPA::UJsonValue &chatId = arr[n];
                if (chatId.is_int64()) {
                    unsigned int id = (unsigned int) chatId.as_int64();
                    su << id;
                } else if (chatId.is_uint64()) {
                    unsigned int id = (unsigned int) chatId.as_uint64();
                    su << id;
                }
            }
        }
        if (!bEnter) {
            unsigned int count = su->GetSize() / sizeof (unsigned int);
            q << count;
        }
        q.Push(su->GetBuffer(), su->GetSize());
    }

    void CWebRequestProcessor::MakeMsg(const SPA::UJsonValue &msg, SPA::CUQueue & q) {
        SPA::UVariant vtMsg;
        switch (msg.kind()) {
            case boost::json::kind::array:
            case boost::json::kind::object:
            {

                SPA::CScopeUQueue su;
                unsigned int approxSize = GetApproxSize();
                if (su->GetMaxSize() < approxSize)
                    su->ReallocBuffer(approxSize);
                su << msg;

                SPA::CScopeUQueue su2;
                SPA::Utilities::ToUTF16((const char*) su->GetBuffer(), su->GetSize(), *su2);
                const char16_t *str = (const char16_t*) su2->GetBuffer();

                VARTYPE vt = VT_BSTR;
                //don't use vtMsg and remove one memory copy
                q.Push((const unsigned char*) &vt, sizeof (vt));
                q << str;
            }
                break;
            case boost::json::kind::string:
            {
                SPA::CScopeUQueue su;
                const boost::json::string& str = msg.as_string();
                const char* src = str.c_str();
                SPA::Utilities::ToUTF16(src, str.size(), *su);
                const char16_t *s = (const char16_t*) su->GetBuffer();

                VARTYPE vt = VT_BSTR;
                //don't use vtMsg and remove one memory copy
                q.Push((const unsigned char*) &vt, sizeof (vt));
                q << s;
            }
                break;
            case boost::json::kind::null:
                q << vtMsg;
                break;
            case boost::json::kind::bool_:
                vtMsg = msg.as_bool();
                q << vtMsg;
                break;

            case boost::json::kind::double_:
                vtMsg = msg.as_double();
                q << vtMsg;
                break;
            case boost::json::kind::uint64:
            {
                static constexpr unsigned int MAX_UINT = 0xffffffff;
                SPA::UINT64 data = msg.get_uint64();
                if (data <= MAX_UINT) {
                    vtMsg = (unsigned int) data;
                } else {
                    vtMsg = data;
                }
            }
                q << vtMsg;
                break;
            case boost::json::kind::int64:
            {
                SPA::INT64 data = msg.get_int64();
                static constexpr int MAX_INT = 0x7fffffff;
                static constexpr int MIN_INT = -1 - MAX_INT;
                if (data <= MAX_INT && data >= MIN_INT) {
                    vtMsg = (int) data;
                } else {
                    vtMsg = data;
                }
            }
                q << vtMsg;
                break;
            default:
                assert(false);
                break;
        }
    }

    void CWebRequestProcessor::MakeForSpeak(const UHttpRequest &ur, SPA::CUQueue & q) {
        unsigned int count = ur.GetArgCount();
        if (count > 1) {
            const SPA::UJsonValue &groups = ur.GetArg(1);
            unsigned short vt = (VT_ARRAY | VT_UINT);
            q << vt;
            MakeChatGroup(groups, false, q);
        } else {
            unsigned short vt = (VT_ARRAY | VT_UINT);
            unsigned int count = 0;
            q << vt << count;
        }
        if (count) {
            const SPA::UJsonValue &msg = ur.GetArg(0);
            MakeMsg(msg, q);
        } else {
            SPA::UVariant vtMsg;
            q << vtMsg;
        }
    }

    void CWebRequestProcessor::MakeForSendUserMessage(const UHttpRequest &ur, SPA::CUQueue & q) {
        unsigned int count = ur.GetArgCount();
        if (count) {
            if (ur.GetArg(0).if_string()) {
                const char* str = ur.GetArg(0).as_string().c_str();
                SPA::CScopeUQueue su;
                if (str)
                    SPA::Utilities::ToWide(str, ::strlen(str), *su);
                q << (const wchar_t*) su->GetBuffer();
            }
        } else {
            q << L"";
        }

        if (count > 1) {
            const SPA::UJsonValue &msg = ur.GetArg(1);
            MakeMsg(msg, q);
        } else {
            SPA::UVariant vtMsg;
            q << vtMsg;
        }
    }

    void CWebRequestProcessor::MakeForEnter(const UHttpRequest &ur, SPA::CUQueue & q) {
        unsigned short vt = (VT_ARRAY | VT_UINT);
        q << vt;
        unsigned int count = ur.GetArgCount();
        if (count) {
            const SPA::UJsonValue &groups = ur.GetArg(0);
            MakeChatGroup(groups, false, q);
        } else
            q << count;
    }

    void CWebRequestProcessor::MakeForDoRequest(const UHttpRequest &ur, SPA::CUQueue & q) {
        unsigned int n, count = ur.GetArgCount();
        q << ur.ReqName;
        q << count;
        for (n = 0; n < count; ++n) {
            const SPA::UJsonValue &v = ur.GetArg(n);
            MakeMsg(v, q);
        }
    }

    void CWebRequestProcessor::MakeBinaryBatchRequests() {
        SPA::CScopeUQueue su;
        unsigned int n, count = m_ur.GetReqCount();
        bool ServerBatching = (count > 1 && m_ur.IsServerBatching());
        for (n = 0; n < count; ++n) {
            const SPA::UJsonValue &req = m_ur.GetChildRequest(n);
            UHttpRequest ur;
            ur.SpRequest = srUnknown;
            ur.ErrCode = seOk;
            CheckRequest(req, ur);
            if (ur.ErrCode == seOk) {
                MakeBinaryRequest(ur, *su);
                m_qRequest->Push(su->GetBuffer(), su->GetSize());
                su->SetSize(0);
            } else {
                m_ur.ErrCode = ur.ErrCode;
                break;
            }
        }
    }

    void CWebRequestProcessor::MakeBinaryRequest(const UHttpRequest &ur, SPA::CUQueue & q) {
        SPA::CStreamHeader reqInfo;
        switch (ur.SpRequest) {
            case srPing:
                reqInfo.RequestId = (unsigned short) SPA::tagBaseRequestID::idPing;
                break;
            case srExit:
                q << ur.CallIndex;
                q << ur.ErrCode;
                reqInfo.RequestId = (unsigned short) SPA::tagChatRequestID::idExit;
                break;
            case srEnter:
                reqInfo.RequestId = (unsigned short) SPA::tagChatRequestID::idEnter;
                MakeForEnter(ur, q);
            {
                SPA::CScopeUQueue su;
                su << ur.CallIndex;
                su << ur.ErrCode;
                q.Insert(su->GetBuffer(), su->GetSize());
            }
                break;
            case srSpeak:
                reqInfo.RequestId = (unsigned short) SPA::tagChatRequestID::idSpeak;
                MakeForSpeak(ur, q);
            {
                SPA::CScopeUQueue su;
                su << ur.CallIndex;
                su << ur.ErrCode;
                q.Insert(su->GetBuffer(), su->GetSize());
            }
                break;
            case srSendUserMessage:
                reqInfo.RequestId = (unsigned short) SPA::tagChatRequestID::idSendUserMessage;
                MakeForSendUserMessage(ur, q);
            {
                SPA::CScopeUQueue su;
                su << ur.CallIndex;
                su << ur.ErrCode;
                q.Insert(su->GetBuffer(), su->GetSize());
            }
                break;
            case srSwitchTo:
                reqInfo.RequestId = (unsigned short) SPA::tagBaseRequestID::idSwitchTo;
                break;
            case srRequest:
                reqInfo.RequestId = (unsigned short) SPA::ServerSide::tagHttpRequestID::idUserRequest;
                MakeForDoRequest(ur, q);
            {
                SPA::CScopeUQueue su;
                su << ur.CallIndex;
                su << ur.ErrCode;
                q.Insert(su->GetBuffer(), su->GetSize());
            }
                break;
            case srClose:
                reqInfo.RequestId = (unsigned short) SPA::tagBaseRequestID::idHttpClose;
                break;
            default:
                assert(false);
                break;
        }
        reqInfo.Size = q.GetSize();
        q.Insert((const unsigned char*) &reqInfo, sizeof (reqInfo), 0);
    }

    void CWebRequestProcessor::CheckRequest(const SPA::UJsonValue &req, UHttpRequest & ur) {
        if (!req.is_object()) {
            ur.ErrCode = seUnexpectedRequest;
            return;
        }
        const SPA::UJsonObject& json = req.as_object();
        size_t size = json.size();
        if (size != 5) {
            ur.ErrCode = seBadNumberOfArgs;
            return;
        }

        if (!json.contains(CHttpContext::SP_REQUEST_NAME) ||
                !json.contains(CHttpContext::SP_REQUEST_CI) ||
                !json.contains(CHttpContext::SP_REQUEST_VERSION) ||
                !json.contains(CHttpContext::SP_REQUEST_ARGS) ||
                !json.contains(CHttpContext::SP_REQUEST_ID)
                ) {
            ur.ErrCode = seBadArgs;
            return;
        }

        const SPA::UJsonValue &reqName = json.at(CHttpContext::SP_REQUEST_NAME);
        if (!reqName.is_string()) {
            ur.ErrCode = seWrongArgType;
            return;
        }

        ur.ReqName = reqName.as_string().c_str();
        ur.SpRequest = MapRequest(ur.ReqName);

        const SPA::UJsonValue &id = json.at(CHttpContext::SP_REQUEST_ID);
        if (!id.is_string()) {
            ur.ErrCode = seWrongArgType;
            return;
        }
        ur.Id = id.as_string().c_str();

        if (ur.SpRequest != srSwitchTo && ::strlen(ur.Id) == 0) {
            ur.ErrCode = seUnexpectedRequest;
            return;
        }

        const SPA::UJsonValue &ci = json.at(CHttpContext::SP_REQUEST_CI);
        if (ci.is_int64()) {
            ur.CallIndex = ci.as_int64();
        } else if (ci.is_uint64()) {
            ur.CallIndex = (SPA::INT64)ci.as_uint64();
        } else {
            ur.ErrCode = seWrongArgType;
            return;
        }

        const SPA::UJsonValue &v = json.at(CHttpContext::SP_REQUEST_VERSION);
        if (!v.is_double()) {
            ur.ErrCode = seWrongArgType;
            return;
        }
        ur.Version = v.as_double();

        const SPA::UJsonValue &args = json.at(CHttpContext::SP_REQUEST_ARGS);
        if (!args.is_array()) {
            ur.ErrCode = seWrongArgType;
            return;
        }

        const SPA::UJsonArray& arr = args.as_array();
        if (ur.IsBatching()) {
            ur.ErrCode = seBadArgs;
            do {
                if (arr.size() != 2)
                    break;
                if (!arr[(unsigned int) 0].is_bool())
                    break;
                const SPA::UJsonValue &objs = arr[(unsigned int) 1];
                if (!objs.is_array() || objs.as_array().size() == 0)
                    break;
                if (!objs.as_array()[(unsigned int) 0].is_object())
                    break;
                ur.ErrCode = seOk;
            } while (false);
        }
        ur.Args = &args;
    }

    void CWebRequestProcessor::ParseSPRequest(const char *str) {
        m_qRequest->SetSize(0);
        m_ur.SpRequest = srUnknown;
        m_ur.ErrCode = seOk;
        unsigned int len = 0;
        if (str == nullptr || (len = (unsigned int) ::strlen(str)) == 0) {
            m_ur.ErrCode = seEmptyRequest;
            m_CurrentErrCode = m_ur.ErrCode;
            return;
        }

        if (m_qRequest->GetMaxSize() > 5 * 1460 && len < m_qRequest->GetMaxSize())
            m_qRequest->ReallocBuffer(SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE);

        boost::json::error_code ec;
        m_doc = boost::json::parse(str, ec);
        if (ec) {
            m_ur.ErrCode = seBadJson;
            m_CurrentErrCode = m_ur.ErrCode;
            return;
        }
        CheckRequest(m_doc, m_ur);
        if (m_ur.ErrCode == seOk) {
            if (m_ur.SpRequest == srDoBatch)
                MakeBinaryBatchRequests();
            else
                MakeBinaryRequest(m_ur, *m_qRequest);
        }
        m_CurrentIndex = m_ur.CallIndex;
        m_CurrentErrCode = m_ur.ErrCode;
    }

    const UHttpRequest & CWebRequestProcessor::GetUHttpRequest() const {
        return m_ur;
    }

    const SPA::CUQueue & CWebRequestProcessor::GetBinaryRequests() const {
        return *m_qRequest;
    }

    CJavaScriptRequestProcessor::CJavaScriptRequestProcessor(CHttpContext * pHttpContext)
            : CWebRequestProcessor(pHttpContext) {
        assert(pHttpContext);
        assert(pHttpContext->GetTransport() == SPA::ServerSide::tagTransport::tScript);
    }

    const UHttpRequest & CJavaScriptRequestProcessor::Parse() {
        m_jsData->SetSize(0);
        m_jsData->Push(m_pHttpContext->GetUJSData());
        m_jsData->SetNull();
        if (m_jsData->GetTailSize() > 5 * 1460 && m_jsData->GetSize() < 2 * 1460) {
            m_jsData->ReallocBuffer(2 * 1460 + sizeof (wchar_t));
        }
        ParseSPRequest((char*) m_jsData->GetBuffer());
        return m_ur;
    }

    unsigned int CJavaScriptRequestProcessor::GetApproxSize() {
        return m_jsData->GetSize();
    }

    CAjaxRequestProcessor::CAjaxRequestProcessor(CHttpContext * pHttpContext)
            : CWebRequestProcessor(pHttpContext) {
        assert(pHttpContext);
        assert(pHttpContext->GetTransport() == SPA::ServerSide::tagTransport::tAjax);
    }

    const UHttpRequest & CAjaxRequestProcessor::Parse() {
        if (m_qPost->GetTailSize() > 5 * 1460 && m_qPost->GetSize() < 2 * 1460) {
            m_qPost->ReallocBuffer(2 * 1460 + sizeof (wchar_t));
        }
        ParseSPRequest((char*) m_qPost->GetBuffer());
        return m_ur;
    }

    unsigned int CAjaxRequestProcessor::GetApproxSize() {
        return m_qPost->GetSize();
    }

    void CAjaxRequestProcessor::SetContent(const unsigned char *data, unsigned int len) {
        m_qPost->SetSize(0);
        m_qPost->Push(data, len);
        m_qPost->SetNull();
    }

    CWebSocketRequestProcessor::CWebSocketRequestProcessor(CHttpContext * pHttpContext)
            : CWebRequestProcessor(pHttpContext) {
        assert(pHttpContext);
        assert(pHttpContext->IsWebSocket());
    }

    const UHttpRequest & CWebSocketRequestProcessor::Parse() {
        SPA::CUQueue &q = m_pHttpContext->GetWebSocketMsg()->Content;
        q.SetNull();
        ParseSPRequest((char*) q.GetBuffer());
        return m_ur;
    }

    unsigned int CWebSocketRequestProcessor::GetApproxSize() {
        return m_pHttpContext->GetWebSocketMsg()->Content.GetSize();
    }

}
