#pragma once

#include "stdafx.h"
#include <map>
#include <queue>
#include "httpgrammar.h"
#include <fstream>
#include "../../pinc/zlib.h"

namespace UHTTP {

    typedef std::map<int, std::string>::const_iterator CEcPointer;

    enum enumHttpResponseFeedPlan {
        hrfpAll = 0,
        hrfpHeaderOnly,
        hrfpLengthUnknown,
        hrfpChuncked,
    };

    enum enumHttpResponseStatus {
        hrsInitial = 0,
        hrsHeadersOnly,
        hrsContent,
        hrsDownloadingFile,
        hrsCompleted,
    };

    struct HttpResponse {
        HttpResponse() {
            ::memset(this, 0, sizeof (HttpResponse));
        }
        enumHttpResponseFeedPlan FeedPlan;
        enumHttpResponseStatus Status;
        bool GZip;
        bool Chunked;
    };

    class CHttpContext : public boost::noncopyable {
    private:
        CHttpRequestScopeUQueue suRequestHeaders;

        struct SetMap {
            SetMap();
        };
        static SetMap sm;

        //for headers
        CRequestContext m_RequestContext;

        CHttpContext();
        ~CHttpContext();

        const CHeaderValue* SeekHeader(const std::string &header) const;
        void SetDate();
        void SetDefaultResponseHeaders();
        void GetResponeHeaders(MB::CUQueue &mq);

        static void SetAppName();
        void CloseFile();

    public:
        const HttpResponse& GetHttpResponseSituation() const;

        tagParseStatus GetPS() const;
        const StringMapA& GetResponseHeaderMap() const;
        tagTransferEncoding GetTE() const;
        tagContentMultiplax GetCM() const;
        const char* ParseHeaders(const char *str);
        const CHeaderValue* GetRequestHeaders(unsigned int &count) const;
        const CHeaderValue* SeekMultipart() const;
        const CHeaderValue* SeekRequestHeader(const char *header) const;
        tagHttpMethod GetMethod() const;
        bool IsWebSocket() const;
        const char* SeekResponseHeaderValue(const char *header) const;

        //Url is already decoded
        const CHttpUnit& GetUrl() const;
        const CHttpUnit& GetParams() const;
        double GetVersion() const;
        MB::U_UINT64 GetContentLength() const;
        bool AddResponseHeader(const char *header, const char *value);
        bool AddSecWebSocketAcceptHeader(const char *key);
        bool IsGZipAccepted() const;
        bool IsChunkOk() const;
        bool IsKeepAlive() const;
        unsigned int GetResponseCode() const;
        void SetResponseCode(unsigned int errCode);
        const char* ParseMultipart(const char *subdata, unsigned int len);
        enumHttpResponseStatus PrepareHttpResponse(const unsigned char *buffer, unsigned int len, MB::CUQueue &qResponse, enumHttpResponseFeedPlan fp = hrfpAll);
        bool StartDownloadFile(const char *file, MB::CUQueue &RecvBuffer);
        bool DownloadFile(MB::CUQueue &RecvBuffer);

    public:
        static CHttpContext* Lock();
        static void Unlock(CHttpContext *p);

    public:
        static const std::map<int, std::string>& GetErrorCodeMap();
        static const unsigned int INITIAL_RESPONSE_BUFFER_SIZE = 10 * 1024;
        static const unsigned int RESPONSE_CHUNK_SIZE = 20 * 1024;
        //static const unsigned int RESPONSE_GZIP_CHUNK_SIZE = 23 * 1024; /* zlib 1.13*RESPONSE_CHUNK_SIZE */

    private:
        //for request headers only
        MB::CUQueue &RequestHeaders;
        StringMapA m_mapResponse;
        unsigned int m_ResponseCode;
        CMultiplaxContext *m_pMultiplaxContext;
        HttpResponse m_HttpResponse;
        bool m_bWebSocket;
        FILE *m_file;
        static std::map<int, std::string> ErrorCodeMap;
        static StringMapA BuiltinFileContentTypeMap;
        static MB::CUCriticalSection m_cs;
        static std::queue<CHttpContext*> m_qHC;
        static std::string AppName;
    };

    typedef MB::CScopeUQueueEx <CHttpContext::RESPONSE_CHUNK_SIZE, DEFAULT_MEMORY_BUFFER_BLOCK_SIZE> CHttpDownFileBuffer;

};

