#pragma once

#include "uhttpdefines.h"
#include <boost/noncopyable.hpp>

namespace UHTTP {

    class CHttpContext;

    class CWebRequestProcessor : private boost::noncopyable {
    public:
        CWebRequestProcessor(CHttpContext *pHttpContext);
        virtual ~CWebRequestProcessor();

    public:
        virtual const UHttpRequest& Parse() = 0;
        virtual unsigned int GetApproxSize() = 0;

        const UHttpRequest& GetUHttpRequest() const;
        CHttpContext* GetHttpContext() const;
        UHttpRequest ProcessChildRequest(const SPA::UJsonValue &childRequest);
        static std::string GenerateId();
        SPA::UJsonDocument& GetDoc() const;
        const SPA::CUQueue& GetBinaryRequests() const;
        void RemoveStoredResponses() const;
        void ShrinkMemory();

    protected:
        void ParseSPRequest(char *str);

    private:
        void MakeBinaryRequest(const UHttpRequest &ur, SPA::CUQueue &q);
        void MakeBinaryBatchRequests();
        static tagSpRequest MapRequest(const char *reqName);
        static void MakeChatGroup(const SPA::UJsonValue &groups, bool bEnter, SPA::CUQueue &q);
        void MakeMsg(const SPA::UJsonValue &msg, SPA::CUQueue &q);
        void MakeForSpeak(const UHttpRequest &ur, SPA::CUQueue &q);
        void MakeForSendUserMessage(const UHttpRequest &ur, SPA::CUQueue &q);
        void MakeForEnter(const UHttpRequest &ur, SPA::CUQueue &q);
        void MakeForDoRequest(const UHttpRequest &ur, SPA::CUQueue &q);
        static void CheckRequest(const SPA::UJsonValue &json, UHttpRequest &ur);

    public:
        SPA::INT64 m_CurrentIndex;
        tagSpError m_CurrentErrCode;

    protected:
        SPA::UJsonDocument *m_doc;
        UHttpRequest m_ur;
        CHttpContext *m_pHttpContext;

    private:
        SPA::CScopeUQueue m_qRequest;
    };

    class CJavaScriptRequestProcessor : public CWebRequestProcessor {
    public:
        CJavaScriptRequestProcessor(CHttpContext *pHttpContext);

    public:
        const UHttpRequest& Parse();
        virtual unsigned int GetApproxSize();

    private:
        SPA::CScopeUQueue m_jsData;
    };

    class CAjaxRequestProcessor : public CWebRequestProcessor {
    public:
        CAjaxRequestProcessor(CHttpContext *pHttpContext);

    public:
        virtual unsigned int GetApproxSize();
        const UHttpRequest& Parse();
        void SetContent(const unsigned char *data, unsigned int len);

    private:
        SPA::CScopeUQueue m_qPost;
    };

    class CWebSocketRequestProcessor : public CWebRequestProcessor {
    public:
        CWebSocketRequestProcessor(CHttpContext *pHttpContext);

    public:
        const UHttpRequest& Parse();
        virtual unsigned int GetApproxSize();
    };

};
