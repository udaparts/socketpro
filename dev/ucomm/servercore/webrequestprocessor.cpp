#include "webrequestprocessor.h"
#include "../core_shared/pinc/base64.h"
#include <boost/uuid/uuid_generators.hpp>
#include "connectioncontext.h"
#include "../core_shared/shared/streamhead.h"
#include "httpcontext.h"

namespace UHTTP
{

    CWebRequestProcessor::CWebRequestProcessor(CHttpContext * pHttpContext)
            : m_CurrentIndex(0), m_CurrentErrCode(seOk), m_doc(nullptr), m_pHttpContext(pHttpContext) {
        ::memset(&m_ur, 0, sizeof (m_ur));
    }

    CWebRequestProcessor::~CWebRequestProcessor() {
        delete m_doc;
    }

    CHttpContext * CWebRequestProcessor::GetHttpContext() const {
        return m_pHttpContext;
    }

    SPA::UJsonDocument & CWebRequestProcessor::GetDoc() const {
        return *m_doc;
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

    UHttpRequest CWebRequestProcessor::ProcessChildRequest(const SPA::UJsonValue & doc) {
        UHttpRequest UReq;
        UReq.SpRequest = srUnknown;
        rapidjson::SizeType size = doc.MemberSize();
        if (size != 5) {
            UReq.ErrCode = seBadNumberOfArgs;
            return UReq;
        }

        if (!doc.HasMember(CHttpContext::SP_REQUEST_NAME.c_str()) ||
                !doc.HasMember(CHttpContext::SP_REQUEST_CI.c_str()) ||
                !doc.HasMember(CHttpContext::SP_REQUEST_VERSION.c_str()) ||
                !doc.HasMember(CHttpContext::SP_REQUEST_ARGS.c_str()) ||
                !doc.HasMember(CHttpContext::SP_REQUEST_ID.c_str())
                ) {
            UReq.ErrCode = seBadArgs;
            return UReq;
        }

        const SPA::UJsonValue &reqName = doc[CHttpContext::SP_REQUEST_NAME.c_str()];
        if (!reqName.IsString()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }

        UReq.ReqName = reqName.GetString();
        UReq.SpRequest = MapRequest(UReq.ReqName);

        const SPA::UJsonValue &id = doc[CHttpContext::SP_REQUEST_ID.c_str()];
        if (!id.IsString()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }
        UReq.Id = id.GetString();

        const SPA::UJsonValue &ci = doc[CHttpContext::SP_REQUEST_CI.c_str()];
        if (!ci.IsInt() && !ci.IsInt64()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }
        UReq.CallIndex = ci.GetInt64();

        const SPA::UJsonValue &v = doc[CHttpContext::SP_REQUEST_VERSION.c_str()];
        if (!v.IsDouble()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }
        UReq.Version = v.GetDouble();

        const SPA::UJsonValue &args = doc[CHttpContext::SP_REQUEST_ARGS.c_str()];
        if (!args.IsArray()) {
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
        if (groups.IsArray()) {
            unsigned int n, m = groups.Size();
            for (n = 0; n < m; ++n) {
                const SPA::UJsonValue &chatId = groups[n];
                if (chatId.IsUint() || chatId.IsUint64()) {
                    unsigned int id = chatId.GetUint();
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
        switch (msg.GetType()) {
            case rapidjson::kArrayType:
            case rapidjson::kObjectType:
            {
                unsigned short vt = VT_BSTR;
                SPA::UJsonValue &doc = (SPA::UJsonValue&)msg;
                SPA::CScopeUQueue su, su2;
                unsigned int approxSize = GetApproxSize();
                if (su->GetMaxSize() < approxSize)
                    su->ReallocBuffer(approxSize);
                SPA::UJsonWriter writer(*su);
                doc.Accept(writer);
                su->SetNull();
                SPA::Utilities::ToWide((const char*) su->GetBuffer(), su->GetSize(), *su2);
                const wchar_t *str = (const wchar_t*) su2->GetBuffer();

                //don't use vtMsg and remove one memory copy
                q.Push((const unsigned char*) &vt, sizeof (vt));
                q << str;
            }
                break;
            case rapidjson::kStringType:
            {
                unsigned short vt = VT_BSTR;
                SPA::CScopeUQueue su;
                const char *src = msg.GetString();
                SPA::Utilities::ToWide(msg.GetString(), ::strlen(src), *su);
                const wchar_t *str = (const wchar_t*) su->GetBuffer();

                //don't use vtMsg and remove one memory copy
                q.Push((const unsigned char*) &vt, sizeof (vt));
                q << str;
            }
                break;
            case rapidjson::kNullType:
                q << vtMsg;
                break;
            case rapidjson::kFalseType:
                vtMsg = false;
                q << vtMsg;
                break;
            case rapidjson::kTrueType:
                vtMsg = true;
                q << vtMsg;
                break;
            case rapidjson::kNumberType:
                if (msg.IsInt())
                    vtMsg = msg.GetInt();
                else if (msg.IsUint())
                    vtMsg = msg.GetUint();
                else if (msg.IsInt64())
                    vtMsg = msg.GetInt64();
                else if (msg.IsUint64())
                    vtMsg = msg.GetUint64();
                else
                    vtMsg = msg.GetDouble();
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
            if (ur.GetArg(0).IsString()) {
                const char *str = ur.GetArg(0).GetString();
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

    void CWebRequestProcessor::CheckRequest(const SPA::UJsonValue &json, UHttpRequest & ur) {
        rapidjson::SizeType size = json.MemberSize();
        if (size != 5) {
            ur.ErrCode = seBadNumberOfArgs;
            return;
        }

        if (!json.HasMember(CHttpContext::SP_REQUEST_NAME.c_str()) ||
                !json.HasMember(CHttpContext::SP_REQUEST_CI.c_str()) ||
                !json.HasMember(CHttpContext::SP_REQUEST_VERSION.c_str()) ||
                !json.HasMember(CHttpContext::SP_REQUEST_ARGS.c_str()) ||
                !json.HasMember(CHttpContext::SP_REQUEST_ID.c_str())
                ) {
            ur.ErrCode = seBadArgs;
            return;
        }

        const SPA::UJsonValue &reqName = json[CHttpContext::SP_REQUEST_NAME.c_str()];
        if (!reqName.IsString()) {
            ur.ErrCode = seWrongArgType;
            return;
        }

        ur.ReqName = reqName.GetString();
        ur.SpRequest = MapRequest(ur.ReqName);

        const SPA::UJsonValue &id = json[CHttpContext::SP_REQUEST_ID.c_str()];
        if (!id.IsString()) {
            ur.ErrCode = seWrongArgType;
            return;
        }
        ur.Id = id.GetString();

        if (ur.SpRequest != srSwitchTo && ::strlen(ur.Id) == 0) {
            ur.ErrCode = seUnexpectedRequest;
            return;
        }

        const SPA::UJsonValue &ci = json[CHttpContext::SP_REQUEST_CI.c_str()];
        if (!ci.IsInt() && !ci.IsInt64()) {
            ur.ErrCode = seWrongArgType;
            return;
        }
        ur.CallIndex = ci.GetInt64();

        const SPA::UJsonValue &v = json[CHttpContext::SP_REQUEST_VERSION.c_str()];
        if (!v.IsDouble()) {
            ur.ErrCode = seWrongArgType;
            return;
        }
        ur.Version = v.GetDouble();

        const SPA::UJsonValue &args = json[CHttpContext::SP_REQUEST_ARGS.c_str()];
        if (!args.IsArray()) {
            ur.ErrCode = seWrongArgType;
            return;
        }

        if (ur.IsBatching()) {
            ur.ErrCode = seBadArgs;
            do {
                if (args.Size() != 2)
                    break;
                if (!args[(unsigned int) 0].IsBool())
                    break;
                const SPA::UJsonValue &objs = args[(unsigned int) 1];
                if (!objs.IsArray() || objs.Size() == 0)
                    break;
                if (!objs[(unsigned int) 0].IsObject())
                    break;
                ur.ErrCode = seOk;
            } while (false);
        }
        ur.Args = &args;
    }

    void CWebRequestProcessor::ParseSPRequest(char *str) {
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

        const SPA::UJsonValue &json = m_doc->ParseInsitu < rapidjson::kParseInsituFlag > (str);
        if (m_doc->HasParseError()) {
            m_ur.ErrCode = seBadJson;
            m_CurrentErrCode = m_ur.ErrCode;
            return;
        }

        CheckRequest(json, m_ur);

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
        delete m_doc;
        m_doc = new SPA::UJsonDocument;
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
        delete m_doc;
        m_doc = new SPA::UJsonDocument;
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
        delete m_doc;
        m_doc = new SPA::UJsonDocument;
        ParseSPRequest((char*) q.GetBuffer());
        return m_ur;
    }

    unsigned int CWebSocketRequestProcessor::GetApproxSize() {
        return m_pHttpContext->GetWebSocketMsg()->Content.GetSize();
    }

}
