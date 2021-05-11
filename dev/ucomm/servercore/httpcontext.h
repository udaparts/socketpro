#pragma once

#include <fstream>
#include "webresponseProcessor.h"
#include "../core_shared/pinc/hpdefines.h"

namespace UHTTP {

    class U_MODULE_HIDDEN CHttpContext : public boost::noncopyable {
    private:
        CHttpHeaderScopeUQueue suRequestHeaders;

        struct SetMap {
            SetMap();
        };
        static SetMap sm;

        //for headers
        CRequestContext m_RequestContext;

        CHttpContext();
        ~CHttpContext();

        const CHeaderValue* SeekHeader(const std::string &header) const;
        void SetDate(SPA::CUQueue &mq);
        void SetDefaultResponseHeaders();
        void GetResponeHeaders(SPA::CUQueue &mq);
        const char* ParseMultipartHeaders(const UHTTP::CHeaderValue *boundary, const char *multipart_header, unsigned int len);


        static CMultiplaxContext* LockMultiplaxContext();
        static void UnlockMultiplaxContext(CMultiplaxContext *p);

        static CChunkedContext* LockChunkedContext();
        static void UnlockChunkedContext(CChunkedContext *p);

        static CWebSocketMsg* LockWebSocketMsg();
        static void UnlockWebSocketMsg(CWebSocketMsg *p);

        bool IsResponseChunked();
        static tagSpRequest MapRequest(const char *reqName);
        static std::string GenerateId(const UHttpRequest &UReq);
        void ProcessRequestBatch(const UHttpRequest &UReq, SPA::CUQueue &Response);
        UHttpRequest ParseSPRequest(const SPA::UJsonValue &req);
        void ProcessSpRequest(const UHttpRequest &UReq, SPA::UJsonValue &res);

        void SetPt();
        bool AddSecWebSocketAcceptHeader(const char *key);

    public:
        const HttpResponse& GetResponseProgress() const;
        tagParseStatus GetPS() const;
        void SetPostPS(tagParseStatus ps = psComplete);
        const StringMapA& GetResponseHeaderMap() const;
        SPA::ServerSide::tagTransferEncoding GetTE() const;
        SPA::ServerSide::tagContentMultiplax GetCM() const;
        const char* ParseHeaders(const char *str, bool peek = false);
        SPA::ServerSide::tagHttpMethod GetMethod() const;
        bool IsWebSocket() const;
        const char* SeekResponseHeaderValue(const char *header) const;
        //Url is already decoded
        const CHttpUnit& GetUrl() const;
        const CHttpUnit& GetParams() const;
        double GetVersion() const;
        SPA::UINT64 GetContentLength() const;
        bool AddResponseHeader(const char *header, const char *value);
        bool IsGZipAccepted() const;
        bool IsKeepAlive() const;
        bool AskForClose() const;
        bool IsResponseKeepAlive() const;
        bool IsSpRequest() const;
        const char* GetUJSData() const;
        unsigned int GetResponseCode() const;
        void SetResponseCode(unsigned int errCode);
        const unsigned char* ParseWSMsg(const unsigned char *data, unsigned int len);
        const char* ParseMultipart(const char *multipart_data, unsigned int len);
        const char* ParseChunked(const char *chunked_data, unsigned int len);
        tagHttpResponseStatus PrepareResponse(const unsigned char *buffer, unsigned int len, SPA::CUQueue &qResponse, tagHttpResponseFeedPlan fp = hrfpAll);
        tagHttpResponseStatus PrepareBatchResponses(const UHttpRequest &ur, SPA::CUQueue &qResponse, tagHttpResponseFeedPlan fp = hrfpAll);
        void PrepareWSResponseMessage(const unsigned char *buffer, unsigned int len, tagWSOpCode oc, SPA::CUQueue &qResponse);
        void PrepareBatchWSResponseMessage(const UHttpRequest &ur, tagWSOpCode oc, SPA::CUQueue &qResponse);
        bool StartDownloadFile(const char *file, SPA::CUQueue &RecvBuffer);
        bool DownloadFile(SPA::CUQueue &RecvBuffer);
        bool IsCrossDomain() const;
        const char* GetHost() const;
        const char* GetReferer() const;
        const char* GetUserAgent() const;
        void ProcessSpRequest(const UHttpRequest &UReq, SPA::CUQueue &Response);
        unsigned int GetPingTime() const;
        void SetPingTime(unsigned int pt);
        SPA::ServerSide::tagTransport GetTransport() const;
        const CHeaderValue* GetRequestHeaders(unsigned int &count) const;
        const CHeaderValue* SeekMultipart() const;
        const CHeaderValue* SeekRequestHeader(const char *header) const;
        CMultiplaxContext* GetMultiplaxContext() const;
        CChunkedContext* GetChunkedContext() const;
        CWebSocketMsg* GetWebSocketMsg() const;
        tagHttpRequestType GetHttpRequestType() const;
        CWebRequestProcessor* GetWebRequestProcessor() const;
        CWebResponseProcessor* GetWebResponseProcessor() const;
        const UHttpRequest& ParseWebRequest();
        void SetContent(const unsigned char *data, unsigned int len);
        bool IsBinaryRequest() const;

        bool StartChunkedResponse(SPA::CUQueue &RecvBuffer);
        bool SendChunkedData(const unsigned char *buffer, unsigned int len, SPA::CUQueue &RecvBuffer);

    public:
        static CHttpContext* Lock();
        static void Unlock(CHttpContext *p);

    public:
        static std::string SP_REQUEST_NAME;
        static std::string SP_REQUEST_CI;
        static std::string SP_REQUEST_VERSION;
        static std::string SP_REQUEST_ARGS;
        static std::string SP_REQUEST_ID;
        static std::string SP_RESPONSE_CODE;
        static std::string SP_RESPONSE_RESULT;
        static std::string UserAgent;
        static std::string Connection;
        static std::string SP_REQUEST_EXIT;
        static std::string SP_REQUEST_CLOSE;
        static std::string SP_REQUEST_ENTER;
        static std::string SP_REQUEST_SPEAK;
        static std::string SP_REQUEST_SENDUSERMESSAGE;
        static std::string SP_REQUEST_SWITCHTO;
        static std::string SP_REQUEST_PING;
        static std::string SP_REQUEST_DOBATCH;
        static std::string SP_CONNECTION_CLOSE;
        static std::string SP_CONNECTION_KEEP_ALIVE;
        static std::string HTTP_RESPONSE_SELF;
        static std::string HTTP_JS_CALLBACK_HEAD;
        static std::string HTTP_JS_CALLBACK_END;
        static std::string HTTP_JS_GROUPS;
        static std::string HTTP_RESPONSE_PT;

        static const std::map<int, std::string>& GetErrorCodeMap();
        static const unsigned int FILE_READ_BUFFER_SIZE = 10 * 1024;
        static const unsigned int RESPONSE_GZIP_CHUNK_SIZE = 23 * 1024; /* zlib 1.13*FILE_READ_BUFFER_SIZE */

    private:
        //for request headers only
        SPA::CUQueue &m_RequestHeaders;
        StringMapA m_mapResponse;
        unsigned int m_ResponseCode;
        CMultiplaxContext *m_pMultiplaxContext;
        CChunkedContext *m_pChunkedContext;
        CWebSocketMsg *m_pWebSocketMsg;
        HttpResponse m_HttpResponse;
        bool m_bWebSocket;
        std::ifstream m_file;
        tagHttpRequestType m_HttpRequestType;
        unsigned int m_pt;
        SPA::ServerSide::tagTransport m_transport;
        CWebRequestProcessor *m_pWebRequestProcessor;
        CWebResponseProcessor *m_pWebResponseProcessor;

        static std::map<int, std::string> ErrorCodeMap;
#ifdef WIN32_64
        static StringMapA BuiltinFileContentTypeMap;
#else
        static std::map<std::string, std::string> BuiltinFileContentTypeMap;
#endif
        static SPA::CUCriticalSection m_cs;
        static std::vector<CHttpContext*> m_vHC;
        static std::vector<CMultiplaxContext *> m_vMC;
        static std::vector<CChunkedContext*> m_vCC;
        static std::vector<CWebSocketMsg*> m_vWSM;
        static std::string JS_SP_ADAPTER;
        static std::string UJS_DATA;
        static std::string UHTTP_REQUEST;
        static std::string Host;
        static std::string Referer;
        static std::string AcceptEncoding;
        static std::string ContentEncoding;

        static std::string JS_ULOADER;
    };

    typedef SPA::CScopeUQueueEx <CHttpContext::FILE_READ_BUFFER_SIZE, SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE> CHttpDownFileBuffer;
};

