#pragma once

#include "connectioncontext.h"
#include "webrequestprocessor.h"

namespace UHTTP {

    class U_MODULE_HIDDEN CWebResponseProcessor : private boost::noncopyable {
    public:
        CWebResponseProcessor(CWebRequestProcessor *pWebRequestProcessor);
        virtual ~CWebResponseProcessor();

    public:
        CWebRequestProcessor* GetRequestProcessor() const;
        virtual void ProcessPing(SPA::CUQueue &Response);
        virtual std::string ProcessHttpSwitch(bool AuthOk, const char *ipAddr, unsigned short port, SPA::CUQueue &Response);
        virtual void ProcessBadRequest(SPA::CUQueue &Response, tagSpError se = seOk);
        virtual void ProcessClose(SPA::CUQueue &Response);
        static std::string GenerateId(const UHttpRequest &ur);
        virtual unsigned int ProcessUserRequest(const char *res, SPA::CUQueue &Response);
        virtual unsigned int BounceBackEnter(unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response);
        virtual unsigned int BounceBackExit(unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response);
        virtual unsigned int BounceBackSpeak(unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response);
        virtual unsigned int BounceBackSendUserMessage(SPA::CUQueue &Response);
        virtual unsigned int Enter(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response);
        virtual unsigned int Exit(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response);
        virtual unsigned int Speak(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response);
        virtual unsigned int SendUserMessage(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response);
        virtual unsigned int SendExceptionResult(const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode, SPA::CUQueue &Response);

    protected:
        virtual void SetResponse(SPA::UJsonDocument &docRes, SPA::CUQueue &Response) = 0;
        virtual void WriteEnd(SPA::UJsonDocument &docRes, SPA::CUQueue &Response) = 0;
        void SetSenderInfo(SPA::UJsonDocument &docRes, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count);
        void SetOwnBaseResponseInfo(SPA::UJsonDocument &docRes);
        unsigned int GetPt();

    public:
        unsigned int m_nReqCount;

    protected:
        bool m_bServerBatching;
        bool m_bBatching;
        CWebRequestProcessor *m_pWebRequestProcessor;

    public:
        static std::string HTTP_RESPONSE_METHOD;
        static std::string HTTP_RESPONSE_MSG;
        static std::string HTTP_RESPONSE_IP;
        static std::string HTTP_RESPONSE_PORT;
        static std::string HTTP_RESPONSE_SENDER;
        static std::string HTTP_RESPONSE_SERVICE_ID;
        static std::string HTTP_RESPONSE_REQUEST_ID;

    private:
        static const unsigned int DEFAULT_AJAX_SCRIPT_PING_TIME = 4 * 1000; //4 seconds
        static const unsigned int DEFAULT_WEBSOCKET_PING_TIME = 30 * 1000; //30 seconds //IE10 requires ping time not more than 30 seconds
    };

    class U_MODULE_HIDDEN CJavaScriptResponseProcessor : public CWebResponseProcessor {
    public:
        CJavaScriptResponseProcessor(CJavaScriptRequestProcessor *pJavaScriptRequestProcessor);

    protected:
        virtual void SetResponse(SPA::UJsonDocument &docRes, SPA::CUQueue &Response);
        virtual void WriteEnd(SPA::UJsonDocument &docRes, SPA::CUQueue &Response);
    };

    class U_MODULE_HIDDEN CAjaxResponseProcessor : public CWebResponseProcessor {
    public:
        CAjaxResponseProcessor(CAjaxRequestProcessor *pAjaxRequestProcessor);

    protected:
        virtual void SetResponse(SPA::UJsonDocument &docRes, SPA::CUQueue &Response);
        virtual void WriteEnd(SPA::UJsonDocument &docRes, SPA::CUQueue &Response);
    };

    class U_MODULE_HIDDEN CWebSocketResponseProcessor : public CWebResponseProcessor {
    public:
        CWebSocketResponseProcessor(CWebSocketRequestProcessor *pWebSocketRequestProcessor);

    public:
        virtual unsigned int Enter(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response);
        virtual unsigned int Exit(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response);
        virtual unsigned int Speak(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response);
        virtual unsigned int SendUserMessage(const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count, SPA::CUQueue &Response);

    protected:
        virtual void WriteEnd(SPA::UJsonDocument &docRes, SPA::CUQueue &Response);
        virtual void SetResponse(SPA::UJsonDocument &docRes, SPA::CUQueue &Response);
    };

};

