#include "httpcontext.h"
#include <boost/filesystem.hpp>
#include "../core_shared/pinc/base64.h"
#include "../core_shared/pinc/sha1.h"
#include <assert.h>
#include <boost/uuid/uuid_generators.hpp>
#include "jsloader.h"
#include "connectioncontext.h"
#include "../core_shared/pinc/getsysid.h"

namespace UHTTP
{

    std::map<int, std::string> CHttpContext::ErrorCodeMap;

#ifdef WIN32_64
    StringMapA CHttpContext::BuiltinFileContentTypeMap;
#else
    std::map<std::string, std::string> CHttpContext::BuiltinFileContentTypeMap;
#endif

    CHttpContext::SetMap CHttpContext::sm;
    SPA::CUCriticalSection CHttpContext::m_cs;
    std::vector<CHttpContext*> CHttpContext::m_vHC;
    std::vector<CMultiplaxContext *> CHttpContext::m_vMC;
    std::vector<CChunkedContext*> CHttpContext::m_vCC;
    std::vector<CWebSocketMsg*> CHttpContext::m_vWSM;
    std::string CHttpContext::Connection = "Connection";
    std::string CHttpContext::AcceptEncoding = "Accept-Encoding";
    std::string CHttpContext::ContentEncoding = "Content-Encoding";
    std::string CHttpContext::JS_SP_ADAPTER = "/spadapter.js";
    std::string CHttpContext::JS_ULOADER = "/uloader.js";
    std::string CHttpContext::Referer = "Referer";
    std::string CHttpContext::UserAgent = "User-Agent";
    std::string CHttpContext::Host = "Host";
    std::string CHttpContext::UJS_DATA = "UJS_DATA=";
    std::string CHttpContext::UHTTP_REQUEST = "uhttprequest";
    std::string CHttpContext::SP_REQUEST_NAME = "n";
    std::string CHttpContext::SP_REQUEST_CI = "i";
    std::string CHttpContext::SP_REQUEST_VERSION = "v";
    std::string CHttpContext::SP_REQUEST_ARGS = "a";
    std::string CHttpContext::SP_REQUEST_ID = "id";
    std::string CHttpContext::SP_RESPONSE_CODE = "rc";
    std::string CHttpContext::SP_RESPONSE_RESULT = "rt";
    std::string CHttpContext::SP_REQUEST_EXIT = "uexit";
    std::string CHttpContext::SP_REQUEST_CLOSE = "uclose";
    std::string CHttpContext::SP_REQUEST_ENTER = "uenter";
    std::string CHttpContext::SP_REQUEST_SPEAK = "uspeak";
    std::string CHttpContext::SP_REQUEST_SENDUSERMESSAGE = "usendUserMessage";
    std::string CHttpContext::SP_REQUEST_SWITCHTO = "uswitchTo";
    std::string CHttpContext::SP_REQUEST_PING = "uping";
    std::string CHttpContext::SP_REQUEST_DOBATCH = "udoBatch";
    std::string CHttpContext::SP_CONNECTION_CLOSE = "close";
    std::string CHttpContext::SP_CONNECTION_KEEP_ALIVE = "keep-alive";
    std::string CHttpContext::HTTP_RESPONSE_SELF = "me";
    std::string CHttpContext::HTTP_JS_CALLBACK_HEAD = "UHTTP.jsCallback(";
    std::string CHttpContext::HTTP_JS_CALLBACK_END = ");";
    std::string CHttpContext::HTTP_JS_GROUPS = "groups";
    std::string CHttpContext::HTTP_RESPONSE_PT = "pt";

    CHttpContext::SetMap::SetMap() {
        CHttpContext::ErrorCodeMap[100] = "Continue";
        CHttpContext::ErrorCodeMap[101] = "Switching Protocols";
        CHttpContext::ErrorCodeMap[102] = "Processing";
        CHttpContext::ErrorCodeMap[103] = "Checkpoint";
        CHttpContext::ErrorCodeMap[122] = "Request-URI too long";

        CHttpContext::ErrorCodeMap[200] = "OK";
        CHttpContext::ErrorCodeMap[201] = "Created";
        CHttpContext::ErrorCodeMap[202] = "Accepted";
        CHttpContext::ErrorCodeMap[203] = "Non-Authoritative Information";
        CHttpContext::ErrorCodeMap[204] = "No Content";
        CHttpContext::ErrorCodeMap[205] = "Reset Content";
        CHttpContext::ErrorCodeMap[206] = "Partial Content";
        CHttpContext::ErrorCodeMap[207] = "Multi-Status";
        CHttpContext::ErrorCodeMap[208] = "Already Reported";
        CHttpContext::ErrorCodeMap[226] = "IM Used";

        CHttpContext::ErrorCodeMap[300] = "Multiple Choices";
        CHttpContext::ErrorCodeMap[301] = "Moved Permanently";
        CHttpContext::ErrorCodeMap[302] = "Found";
        CHttpContext::ErrorCodeMap[303] = "See Other";
        CHttpContext::ErrorCodeMap[304] = "Not Modified";
        CHttpContext::ErrorCodeMap[305] = "Use Proxy";
        CHttpContext::ErrorCodeMap[306] = "Switch Proxy";
        CHttpContext::ErrorCodeMap[307] = "Temporary Redirect";
        CHttpContext::ErrorCodeMap[308] = "Resume Incomplete";

        CHttpContext::ErrorCodeMap[400] = "Bad Request";
        CHttpContext::ErrorCodeMap[401] = "Unauthorized";
        CHttpContext::ErrorCodeMap[402] = "Payment Required";
        CHttpContext::ErrorCodeMap[403] = "Forbidden";
        CHttpContext::ErrorCodeMap[404] = "Not Found";
        CHttpContext::ErrorCodeMap[405] = "Method Not Allowed";
        CHttpContext::ErrorCodeMap[406] = "Not Acceptable";
        CHttpContext::ErrorCodeMap[407] = "Proxy Authentication Required";
        CHttpContext::ErrorCodeMap[408] = "Request Timeout";
        CHttpContext::ErrorCodeMap[409] = "Conflict";
        CHttpContext::ErrorCodeMap[410] = "Gone";
        CHttpContext::ErrorCodeMap[411] = "Length Required";
        CHttpContext::ErrorCodeMap[412] = "Precondition Failed";
        CHttpContext::ErrorCodeMap[413] = "Request Entity Too Large";
        CHttpContext::ErrorCodeMap[414] = "Request-URI Too Long";
        CHttpContext::ErrorCodeMap[415] = "Unsupported Media Type";
        CHttpContext::ErrorCodeMap[416] = "Requested Range Not Satisfiable";
        CHttpContext::ErrorCodeMap[417] = "Expectation Failed";
        CHttpContext::ErrorCodeMap[418] = "I'm a teapot";
        CHttpContext::ErrorCodeMap[422] = "Unprocessable Entity";
        CHttpContext::ErrorCodeMap[423] = "Locked";
        CHttpContext::ErrorCodeMap[424] = "Failed Dependency";
        CHttpContext::ErrorCodeMap[425] = "Unordered Collection";
        CHttpContext::ErrorCodeMap[426] = "Upgrade Required";
        CHttpContext::ErrorCodeMap[428] = "Precondition Required";
        CHttpContext::ErrorCodeMap[429] = "Too Many Requests";
        CHttpContext::ErrorCodeMap[431] = "Request Header Fields Too Large";
        CHttpContext::ErrorCodeMap[444] = "No Response";
        CHttpContext::ErrorCodeMap[449] = "Retry With";
        CHttpContext::ErrorCodeMap[450] = "Blocked by Windows Parental Controls";
        CHttpContext::ErrorCodeMap[499] = "Client Closed Request";

        CHttpContext::ErrorCodeMap[500] = "Internal Server Error";
        CHttpContext::ErrorCodeMap[501] = "Not Implemented";
        CHttpContext::ErrorCodeMap[502] = "Bad Gateway";
        CHttpContext::ErrorCodeMap[503] = "Service Unavailable";
        CHttpContext::ErrorCodeMap[504] = "Gateway Timeout";
        CHttpContext::ErrorCodeMap[505] = "HTTP Version Not Supported";
        CHttpContext::ErrorCodeMap[506] = "Variant Also Negotiates";
        CHttpContext::ErrorCodeMap[507] = "Insufficient Storage";
        CHttpContext::ErrorCodeMap[508] = "Loop Detected";
        CHttpContext::ErrorCodeMap[509] = "Bandwidth Limit Exceeded";
        CHttpContext::ErrorCodeMap[510] = "Not Extended";
        CHttpContext::ErrorCodeMap[511] = "Network Authentication Required";
        CHttpContext::ErrorCodeMap[598] = "Network read timeout error";
        CHttpContext::ErrorCodeMap[599] = "Network connect timeout error";

        CHttpContext::BuiltinFileContentTypeMap["htm"] = "text/html";
        CHttpContext::BuiltinFileContentTypeMap["html"] = "text/html";
        CHttpContext::BuiltinFileContentTypeMap["js"] = "application/x-javascript";
        CHttpContext::BuiltinFileContentTypeMap["xml"] = "application/xml";
        CHttpContext::BuiltinFileContentTypeMap["xsl"] = "application/xml";
        CHttpContext::BuiltinFileContentTypeMap["xslt"] = "application/xslt+xml";
        CHttpContext::BuiltinFileContentTypeMap["swf"] = "application/x-shockwave-flash";
        CHttpContext::BuiltinFileContentTypeMap["php"] = "text/html";
        CHttpContext::BuiltinFileContentTypeMap["json"] = "application/json; charset=utf-8";
        CHttpContext::BuiltinFileContentTypeMap["rtf"] = "text/rtf";
        CHttpContext::BuiltinFileContentTypeMap["txt"] = "text/plain; charset=utf-8";
        CHttpContext::BuiltinFileContentTypeMap["mhtml"] = "text/html";
        CHttpContext::BuiltinFileContentTypeMap["css"] = "text/css";
        CHttpContext::BuiltinFileContentTypeMap["jpeg"] = "image/jpeg";
        CHttpContext::BuiltinFileContentTypeMap["jpe"] = "image/jpeg";
        CHttpContext::BuiltinFileContentTypeMap["jpg"] = "image/jpeg";
        CHttpContext::BuiltinFileContentTypeMap["bmp"] = "image/bmp";
        CHttpContext::BuiltinFileContentTypeMap["cgm"] = "image/cgm";
        CHttpContext::BuiltinFileContentTypeMap["svg"] = "image/svg+xml";
        CHttpContext::BuiltinFileContentTypeMap["tif"] = "image/tiff";
        CHttpContext::BuiltinFileContentTypeMap["tiff"] = "image/tiff";
        CHttpContext::BuiltinFileContentTypeMap["wbmp"] = "image/vnd.wap.wbmp";
        CHttpContext::BuiltinFileContentTypeMap["djv"] = "image/vnd.djvu";
        CHttpContext::BuiltinFileContentTypeMap["djvu"] = "image/vnd.djvu";
        CHttpContext::BuiltinFileContentTypeMap["ras"] = "image/x-cmu-raster";
        CHttpContext::BuiltinFileContentTypeMap["ico"] = "image/x-icon";
        CHttpContext::BuiltinFileContentTypeMap["pnm"] = "image/x-portable-anymap";
        CHttpContext::BuiltinFileContentTypeMap["pbm"] = "image/x-portable-bitmap";
        CHttpContext::BuiltinFileContentTypeMap["pgm"] = "image/x-portable-graymap";
        CHttpContext::BuiltinFileContentTypeMap["ppm"] = "image/x-portable-pixmap";
        CHttpContext::BuiltinFileContentTypeMap["rgb"] = "image/x-rgb";
        CHttpContext::BuiltinFileContentTypeMap["rbm"] = "image/x-xbitmap";
        CHttpContext::BuiltinFileContentTypeMap["rpm"] = "image/x-xpixmap";
        CHttpContext::BuiltinFileContentTypeMap["rwd"] = "image/x-xwindowdump";
        CHttpContext::BuiltinFileContentTypeMap["vrml"] = "model/vrml";
        CHttpContext::BuiltinFileContentTypeMap["vrl"] = "model/vrml";
        CHttpContext::BuiltinFileContentTypeMap["xap"] = "application/x-silverlight"; //application/x-silverlight-app for cross-domain
        CHttpContext::BuiltinFileContentTypeMap["mathml"] = "application/mathml+xml";
        CHttpContext::BuiltinFileContentTypeMap["doc"] = "application/msword";
        CHttpContext::BuiltinFileContentTypeMap["bin"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["dms"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["lha"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["lzh"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["exe"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["class"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["so"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["dll"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["pdf"] = "application/pdf";
        CHttpContext::BuiltinFileContentTypeMap["ai"] = "application/postscript";
        CHttpContext::BuiltinFileContentTypeMap["eps"] = "application/postscript";
        CHttpContext::BuiltinFileContentTypeMap["ps"] = "application/postscript";
        CHttpContext::BuiltinFileContentTypeMap["rdf"] = "application/rdf+xml";
        CHttpContext::BuiltinFileContentTypeMap["xul"] = "application/vnd.mozilla.xul+xml";
        CHttpContext::BuiltinFileContentTypeMap["xls"] = "application/vnd.ms-excel";
        CHttpContext::BuiltinFileContentTypeMap["ppt"] = "application/vnd.ms-powerpoint";
        CHttpContext::BuiltinFileContentTypeMap["wbxml"] = "application/vnd.wap.wbxml";
        CHttpContext::BuiltinFileContentTypeMap["zip"] = "application/zip";
        CHttpContext::BuiltinFileContentTypeMap["au"] = "audio/basic";
        CHttpContext::BuiltinFileContentTypeMap["snd"] = "audio/basic";
        CHttpContext::BuiltinFileContentTypeMap["mid"] = "audio/midi";
        CHttpContext::BuiltinFileContentTypeMap["midi"] = "audio/midi";
        CHttpContext::BuiltinFileContentTypeMap["kar"] = "audio/midi";
        CHttpContext::BuiltinFileContentTypeMap["rtx"] = "text/richtext";
        CHttpContext::BuiltinFileContentTypeMap["wmls"] = "text/vnd.wap.wmlscript";
        CHttpContext::BuiltinFileContentTypeMap["wml"] = "text/vnd.wap.wml";
        CHttpContext::BuiltinFileContentTypeMap["tsv"] = "text/tab-separated-values";
        CHttpContext::BuiltinFileContentTypeMap["mpe"] = "video/mpeg";
        CHttpContext::BuiltinFileContentTypeMap["mpeg"] = "video/mpeg";
        CHttpContext::BuiltinFileContentTypeMap["mpg"] = "video/mpeg";
        CHttpContext::BuiltinFileContentTypeMap["qt"] = "video/quicktime";
        CHttpContext::BuiltinFileContentTypeMap["mov"] = "video/quicktime";
        CHttpContext::BuiltinFileContentTypeMap["mxu"] = "video/vnd.mpegurl";
        CHttpContext::BuiltinFileContentTypeMap["avi"] = "video/x-msvideo";
        CHttpContext::BuiltinFileContentTypeMap["movie"] = "video/x-sgi-movie";
        CHttpContext::BuiltinFileContentTypeMap["ice"] = "x-conference/x-cooltalk";
        CHttpContext::BuiltinFileContentTypeMap["asc"] = "text/plain";
        CHttpContext::BuiltinFileContentTypeMap["shtml"] = "text/html";
        CHttpContext::BuiltinFileContentTypeMap["ra"] = "audio/x-realaudio";
        CHttpContext::BuiltinFileContentTypeMap["wav"] = "audio/x-wav";
        CHttpContext::BuiltinFileContentTypeMap["xaml"] = "application/xaml+xml";
        CHttpContext::BuiltinFileContentTypeMap["manifest"] = "application/manifest";
        CHttpContext::BuiltinFileContentTypeMap["application"] = "application/x-ms-application";
        CHttpContext::BuiltinFileContentTypeMap["xbap"] = "application/x-ms-xbap";
        CHttpContext::BuiltinFileContentTypeMap["deploy"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["xps"] = "application/vnd.ms-xpsdocument";
    }

    CHttpContext::CHttpContext()
            : m_RequestHeaders(*suRequestHeaders),
            m_ResponseCode(200),
            m_pMultiplaxContext(nullptr),
            m_pChunkedContext(nullptr),
            m_pWebSocketMsg(nullptr),
            m_bWebSocket(false),
            m_HttpRequestType(hrtCustomer),
            m_pt(60000),
            m_transport(SPA::ServerSide::tagTransport::tUnknown),
            m_pWebRequestProcessor(nullptr),
            m_pWebResponseProcessor(nullptr) {

    }

    CHttpContext::~CHttpContext() {
        if (m_file.is_open())
            m_file.close();
        UnlockMultiplaxContext(m_pMultiplaxContext);
        UnlockChunkedContext(m_pChunkedContext);
        UnlockWebSocketMsg(m_pWebSocketMsg);
        delete m_pWebRequestProcessor;
        delete m_pWebResponseProcessor;
    }

    tagHttpRequestType CHttpContext::GetHttpRequestType() const {
        return m_HttpRequestType;
    }

    unsigned int CHttpContext::GetResponseCode() const {
        return m_ResponseCode;
    }

    void CHttpContext::SetResponseCode(unsigned int errCode) {
        m_ResponseCode = errCode;
    }

    bool CHttpContext::StartChunkedResponse(SPA::CUQueue & RecvBuffer) {
        AddResponseHeader(UHTTP::TRANSFER_ENCODING.c_str(), UHTTP::CHUNKED.c_str());
        GetResponeHeaders(RecvBuffer);
        RecvBuffer.Push("\r\n", 2);
        RecvBuffer.SetNull();
        m_HttpResponse.Status = hrsHeadersOnly;
        m_HttpResponse.Chunked = true;
        return true;
    }

    bool CHttpContext::SendChunkedData(const unsigned char *buffer, unsigned int len, SPA::CUQueue & RecvBuffer) {
        if (buffer == nullptr)
            len = 0;
        assert(m_HttpResponse.Chunked); //
        char str[32];
#ifdef WIN32_64
        ::sprintf_s(str, sizeof (str), "%x\r\n", len);
#else
        ::sprintf(str, "%x\r\n", len);
#endif
        RecvBuffer.Push(str);
        RecvBuffer.Push(buffer, len);
        RecvBuffer.Push("\r\n", 2);
        if (len == 0)
            m_HttpResponse.Status = hrsCompleted;
        else
            m_HttpResponse.Status = hrsContent;
        return true;
    }

    tagHttpResponseStatus CHttpContext::PrepareResponse(const unsigned char *buffer, unsigned int len, SPA::CUQueue &qResponse, tagHttpResponseFeedPlan fp) {
        if (buffer == nullptr)
            len = 0;
        if (m_HttpResponse.Status == hrsInitial) {
            switch (fp) {
                case hrfpAll:
                    if (!IsWebSocket()) {
                        char str[32];
#ifdef WIN32_64
                        ::sprintf_s(str, sizeof (str), "%d", len);
#else
                        ::sprintf(str, "%d", len);
#endif
                        m_mapResponse[CONTENT_LENGTH] = str;
                    }
                    m_HttpResponse.Status = hrsCompleted;
                    break;
                default:
                    if (len == 0)
                        m_HttpResponse.Status = hrsHeadersOnly;
                    else
                        m_HttpResponse.Status = hrsContent;
                    break;
            }
            GetResponeHeaders(qResponse);
            qResponse.Push("\r\n", 2);
            //assert(qResponse.GetSize() == ::strlen((const char*) qResponse.GetBuffer()));
        }
        qResponse.Push(buffer, len);
        qResponse.SetNull();
        if (len == 0)
            m_HttpResponse.Status = hrsCompleted;
        return m_HttpResponse.Status;
    }

    void CHttpContext::SetDefaultResponseHeaders() {
        if (AskForClose())
            m_mapResponse[Connection] = SP_CONNECTION_CLOSE.c_str();
        /*       else
                           m_mapResponse[Connection] = SP_CONNECTION_KEEP_ALIVE;*/
        const char *dot = ::strrchr(m_RequestContext.Url.Start, '.');
        if (dot != nullptr) {
            std::string extension(dot + 1, m_RequestContext.Url.Start + m_RequestContext.Url.Length);
            auto it = BuiltinFileContentTypeMap.find(extension);
            if (it != BuiltinFileContentTypeMap.cend()) {
                m_mapResponse[CONTENT_TYPE] = it->second;
            }
        }
    }

    void CHttpContext::SetDate(SPA::CUQueue & mq) {
        using namespace boost::posix_time;
        using namespace boost::gregorian;
        using namespace boost::filesystem;
        char strBuffer[128 + 1];
        ptime Now = second_clock::universal_time();
        date d = Now.date();
        time_duration td = Now.time_of_day();
        unsigned int year = d.year();
        unsigned int month = d.month();
        unsigned int day = d.day();
        unsigned int dayOfWeek = d.day_of_week();
        unsigned int hour = (unsigned int) td.hours();
        unsigned int minute = (unsigned int) td.minutes();
        unsigned int second = (unsigned int) td.seconds();
        const char *strWeekday;
        switch (dayOfWeek) {
            case 0:
                strWeekday = "Sun";
                break;
            case 1:
                strWeekday = "Mon";
                break;
            case 2:
                strWeekday = "Tus";
                break;
            case 3:
                strWeekday = "Wed";
                break;
            case 4:
                strWeekday = "Thu";
                break;
            case 5:
                strWeekday = "Fri";
                break;
            default:
                strWeekday = "Sat";
                break;
        }
        const char *strMonth;
        switch (month) {
            case 1:
                strMonth = "Jan";
                break;
            case 2:
                strMonth = "Feb";
                break;
            case 3:
                strMonth = "Mar";
                break;
            case 4:
                strMonth = "Apr";
                break;
            case 5:
                strMonth = "May";
                break;
            case 6:
                strMonth = "Jun";
                break;
            case 7:
                strMonth = "Jul";
                break;
            case 8:
                strMonth = "Aug";
                break;
            case 9:
                strMonth = "Sep";
                break;
            case 10:
                strMonth = "Oct";
                break;
            case 11:
                strMonth = "Nov";
                break;
            default:
                strMonth = "Dec";
                break;
        }
#ifdef WIN32_64
        ::sprintf_s(strBuffer, sizeof (strBuffer), "Date: %s, %.2d %s %d %.2d:%.2d:%.2d GMT\r\n", strWeekday, day, strMonth, year, hour, minute, second);
#else
        ::sprintf(strBuffer, "Date: %s, %.2d %s %d %.2d:%.2d:%.2d GMT\r\n", strWeekday, day, strMonth, year, hour, minute, second);
#endif
        mq.Push(strBuffer);
    }

    const char* CHttpContext::SeekResponseHeaderValue(const char *header) const {
        const char *str = nullptr;
        std::string h(header);
        boost::trim(h);
        StringMapA::const_iterator it = m_mapResponse.find(h);
        if (it != m_mapResponse.cend())
            str = it->second.c_str();
        return str;
    }

    void CHttpContext::GetResponeHeaders(SPA::CUQueue & mq) {
        const char *str;
        CEcPointer p = ErrorCodeMap.find(m_ResponseCode);
        if (p != ErrorCodeMap.end())
            str = p->second.c_str();
        else {
            assert(false);
            str = "Unknown Error Code";
        }

        {
            char strBuffer[128 + 1];
#ifdef WIN32_64
            ::sprintf_s(strBuffer, sizeof (strBuffer), "HTTP/1.1 %d %s\r\n", m_ResponseCode, str);
#else
            ::sprintf(strBuffer, "HTTP/1.1 %d %s\r\n", m_ResponseCode, str);
#endif
            mq.Push(strBuffer);
        }

        SetDate(mq);
        const std::string AppName = SPA::GetAppName();
        if (AppName.size() > 0) {
            //m_mapResponse["Server"] = AppName;
            mq.Push("Server: ", 8);
            mq.Push(AppName.c_str(), (unsigned int) AppName.size());
            mq.Push("\r\n", 2);
        }
        StringMapA::iterator it;
        StringMapA::iterator end = m_mapResponse.end();
        for (it = m_mapResponse.begin(); it != end; ++it) {
            mq.Push(it->first.c_str(), (unsigned int) it->first.size());
            mq.Push(": ", 2);
            mq.Push(it->second.c_str(), (unsigned int) it->second.size());
            mq.Push("\r\n", 2);
        }
    }

    const std::map<int, std::string>& CHttpContext::GetErrorCodeMap() {
        return ErrorCodeMap;
    }

    tagParseStatus CHttpContext::GetPS() const {
        return m_RequestContext.ParseStatus;
    }

    void CHttpContext::SetPostPS(tagParseStatus ps) {
        m_RequestContext.ParseStatus = ps;
    }

    SPA::ServerSide::tagTransferEncoding CHttpContext::GetTE() const {
        return m_RequestContext.TE;
    }

    SPA::ServerSide::tagContentMultiplax CHttpContext::GetCM() const {
        return m_RequestContext.CM;
    }

    const HttpResponse & CHttpContext::GetResponseProgress() const {
        return m_HttpResponse;
    }

    SPA::UINT64 CHttpContext::GetContentLength() const {
        return m_RequestContext.ContentLen;
    }

    const StringMapA & CHttpContext::GetResponseHeaderMap() const {
        return m_mapResponse;
    }

    const UHttpRequest & CHttpContext::ParseWebRequest() {
        assert(IsSpRequest() || IsWebSocket());
        const UHttpRequest &ur = m_pWebRequestProcessor->Parse();
        delete m_pWebResponseProcessor;
        if (IsWebSocket())
            m_pWebResponseProcessor = new CWebSocketResponseProcessor((CWebSocketRequestProcessor*) m_pWebRequestProcessor);
        else if (m_RequestContext.Method == SPA::ServerSide::tagHttpMethod::hmGet)
            m_pWebResponseProcessor = new CJavaScriptResponseProcessor((CJavaScriptRequestProcessor*) m_pWebRequestProcessor);
        else if (m_RequestContext.Method == SPA::ServerSide::tagHttpMethod::hmPost)
            m_pWebResponseProcessor = new CAjaxResponseProcessor((CAjaxRequestProcessor*) m_pWebRequestProcessor);
        else {
            assert(false);
        }
        return ur;
    }

    void CHttpContext::SetContent(const unsigned char *data, unsigned int len) {
        assert(IsSpRequest() || GetMethod() == SPA::ServerSide::tagHttpMethod::hmPost);
        ((CAjaxRequestProcessor*) m_pWebRequestProcessor)->SetContent(data, len);
    }

    tagHttpResponseStatus CHttpContext::PrepareBatchResponses(const UHttpRequest &ur, SPA::CUQueue &qResponse, tagHttpResponseFeedPlan fp) {
        const SPA::UJsonValue &args = *(ur.Args);
#ifndef NDEBUG
        bool b = args.as_array()[(unsigned int) 0].as_bool();
#endif
        SPA::UJsonObject obj;
        obj[CHttpContext::SP_REQUEST_CI] = ur.CallIndex;
        obj[CHttpContext::SP_RESPONSE_CODE] = ur.ErrCode;
        SPA::UJsonArray docArray;
        const SPA::UJsonValue& objs = args.as_array()[1];
        unsigned int count = (unsigned int) objs.as_array().size();
        for (unsigned int n = 0; n < count; ++n) {
            const SPA::UJsonValue &req = objs.as_array()[n];
            UHttpRequest urKid = ParseSPRequest(req);
            SPA::UJsonValue res;
            ProcessSpRequest(urKid, res);
            docArray.push_back(res);
        }
        obj["rb"] = std::move(docArray);
        SPA::CScopeUQueue jBuffer;
        SPA::UJsonValue jv(std::move(obj));
        jBuffer << jv;
        return PrepareResponse(jBuffer->GetBuffer(), jBuffer->GetSize(), qResponse, fp);
    }

    void CHttpContext::PrepareBatchWSResponseMessage(const UHttpRequest &ur, tagWSOpCode oc, SPA::CUQueue & qResponse) {
        const SPA::UJsonValue &args = *(ur.Args);
#ifndef NDEBUG
        bool b = args.as_array()[(unsigned int) 0].as_bool();
#endif
        const SPA::UJsonValue& objs = args.as_array()[1];
        unsigned int count = (unsigned int) objs.as_array().size();
        for (unsigned int n = 0; n < count; ++n) {
            const SPA::UJsonValue& req = objs.as_array()[n];
            SPA::CScopeUQueue jBuffer;
            UHttpRequest urKid = ParseSPRequest(req);
            ProcessSpRequest(urKid, *jBuffer);
            PrepareWSResponseMessage(jBuffer->GetBuffer(), jBuffer->GetSize(), oc, qResponse);
        }
    }

    void CHttpContext::PrepareWSResponseMessage(const unsigned char *buffer, unsigned int len, tagWSOpCode oc, SPA::CUQueue & qResponse) {
        if (buffer == nullptr)
            len = 0;

        unsigned char Top0 = (CWebSocketMsg::BIT_FIN | (unsigned char) oc);
        unsigned char Top1 = 0;

        if (len <= CWebSocketMsg::MAX_INT8_LEN) {
            Top1 = (unsigned char) len;
            qResponse.Push(&Top0, 1);
            qResponse.Push(&Top1, 1);
        } else if (len <= CWebSocketMsg::MAX_INT16_LEN) {
            Top1 = CWebSocketMsg::UINT16_INDICATOR;
            qResponse.Push(&Top0, 1);
            qResponse.Push(&Top1, 1);
            unsigned short s = (unsigned int) len;
            if (!SPA::IsBigEndian())
                SPA::CUQueue::ChangeEndian((unsigned char*) &s, sizeof (s));
            qResponse.Push((const unsigned char*) &s, sizeof (s));
        } else {
            Top1 = CWebSocketMsg::UINT64_INDICATOR;
            qResponse.Push(&Top0, 1);
            qResponse.Push(&Top1, 1);
            SPA::UINT64 size = len;
            if (!SPA::IsBigEndian())
                SPA::CUQueue::ChangeEndian((unsigned char*) &size, sizeof (size));
            qResponse << size;
        }
        if (len > 0)
            qResponse.Push(buffer, len);
    }

    const unsigned char* CHttpContext::ParseWSMsg(const unsigned char *data, unsigned int len) {
        const unsigned char *record = data;
        const unsigned char *stop = data;
        m_RequestContext.ContentLen = 0;
        if (data == nullptr || len < 2) //Top0 and Top 1
            return record;
        if (!m_bWebSocket)
            throw CUExCode("Not a websocket message", MB_BAD_OPERATION);
        if (m_pWebSocketMsg == nullptr)
            m_pWebSocketMsg = LockWebSocketMsg();
        if (m_pWebSocketMsg->ParseStatus != psInitial)
            return data;
        if (m_pWebSocketMsg->ParseStatus == psInitial) {
            m_pWebSocketMsg->Top0 = data[0];
            if (m_pWebSocketMsg->GetOpCode() == ocMsgContinuation)
                return record;
            m_pWebSocketMsg->Top1 = data[1];
            unsigned int expectedLen = 2; //Top0 and Top 1
            stop += 2; //Top0 and Top 1
            data += 2;
            if (m_pWebSocketMsg->IsMasked())
                expectedLen += sizeof (m_pWebSocketMsg->Mask);
            switch (m_pWebSocketMsg->GetLen()) {
                case CWebSocketMsg::UINT16_INDICATOR:
                    if (len < expectedLen + sizeof (unsigned short))
                        return record;
                    else {
                        unsigned short s;
                        ::memcpy(&s, stop, sizeof (s));
                        //Multi-byte length quantities are expressed in network byte order
                        if (!SPA::IsBigEndian())
                            SPA::CUQueue::ChangeEndian((unsigned char*) &s, sizeof (s));
                        stop += sizeof (unsigned short);
                        expectedLen += sizeof (s);
                        m_RequestContext.ContentLen = s;
                    }
                    break;
                case CWebSocketMsg::UINT64_INDICATOR:
                    if (len < expectedLen + sizeof (SPA::UINT64))
                        return record;
                    else {
                        SPA::UINT64 size;
                        ::memcpy(&size, stop, sizeof (size));
                        //Multi-byte length quantities are expressed in network byte order
                        if (!SPA::IsBigEndian())
                            SPA::CUQueue::ChangeEndian((unsigned char*) &size, sizeof (size));
                        stop += sizeof (size);
                        expectedLen += sizeof (size);
                        m_RequestContext.ContentLen = size;
                    }
                    break;
                default:
                    m_RequestContext.ContentLen = m_pWebSocketMsg->GetLen();
                    break;
            }

            if (m_pWebSocketMsg->IsMasked()) {
                ::memcpy(&m_pWebSocketMsg->Mask, stop, sizeof (m_pWebSocketMsg->Mask));
                stop += sizeof (m_pWebSocketMsg->Mask);
            }
            len -= expectedLen;
            data = stop;
        }

        if (len > (unsigned int) m_RequestContext.ContentLen)
            len = (unsigned int) m_RequestContext.ContentLen;
        if (m_RequestContext.ContentLen + m_pWebSocketMsg->Content.GetSize() + m_pWebSocketMsg->Content.GetHeadPosition() + 16 > m_pWebSocketMsg->Content.GetMaxSize()) {
            m_pWebSocketMsg->Content.ReallocBuffer((unsigned int) m_RequestContext.ContentLen + m_pWebSocketMsg->Content.GetSize() + m_pWebSocketMsg->Content.GetHeadPosition() + 16);
        }
        unsigned char *mask = m_pWebSocketMsg->Mask;
        unsigned int pos = m_pWebSocketMsg->Content.GetSize();
        unsigned int size = m_pWebSocketMsg->Content.GetSize() + len;
        if (size && size < m_RequestContext.ContentLen) {
            m_pWebSocketMsg->Content.SetSize(0);
            return record;
        }

        unsigned char *mydata = (unsigned char *) m_pWebSocketMsg->Content.GetBuffer(pos);
        for (; pos < size; ++pos, ++mydata, ++data) {
            *mydata = *data ^ mask[pos % 4];
        }
        m_pWebSocketMsg->Content.SetSize(size);
        if (size == m_RequestContext.ContentLen)
            m_pWebSocketMsg->ParseStatus = psComplete;

        m_pWebSocketMsg->Content.SetNull();

        if (m_pWebSocketMsg->ParseStatus == psComplete && m_pWebSocketMsg->IsFin() && m_pWebRequestProcessor == nullptr) {
            tagWSOpCode wsOpCode = m_pWebSocketMsg->GetOpCode();
            switch (wsOpCode) {
                case ocTextMsg:
                    m_pWebRequestProcessor = new CWebSocketRequestProcessor(this);
                    break;
                default:
                    break;
            }
        }
        return data;
    }

    const char* CHttpContext::ParseChunked(const char *chunked_data, unsigned int len) {
        const char *sc;
        if (chunked_data == nullptr || len == 0)
            throw CUExCode("No data available for parsing chunked content", MB_BAD_OPERATION);
        if (m_RequestContext.TE != SPA::ServerSide::tagTransferEncoding::teChunked || chunked_data == nullptr || len == 0)
            throw CUExCode("No http chunked found in request", MB_BAD_OPERATION);
        if (m_pChunkedContext == nullptr) {
            m_pChunkedContext = LockChunkedContext();
        }
        SPA::CScopeUQueue su;
        CHeaderValue hv;

        do {
            memset(&hv, 0, sizeof (hv));
            const char *crlf = ::strstr(chunked_data, "\r\n");
            sc = ::strstr(chunked_data, ";");
            if (!crlf)
                return chunked_data;
            unsigned int advanced = (unsigned int) (crlf - chunked_data);
            su->Push(chunked_data, advanced);
            su->SetNull();
            advanced += 2; //CRLF
            len -= advanced;
            unsigned int size = strtoul((const char*) su->GetBuffer(), nullptr, 16);
            if (size + 2 > len)
                break;
            chunked_data += advanced;
            if (sc && sc < crlf) {
                std::string bullshit(sc + 1, crlf);
                boost::trim(bullshit);
                hv.Header.Length = (unsigned int) bullshit.size();
                if (hv.Header.Length)
                    m_pChunkedContext->MHeaders->Push(bullshit.c_str(), hv.Header.Length + 1); //add one extra byte for nullptr
            }

            if (size) {
                m_pChunkedContext->MHeaders->Push(chunked_data, size + 1); //add one extra byte for nullptr
                if (hv.Header.Length == 0)
                    hv.Header.Start = chunked_data;
                hv.Value.Start = chunked_data;
                hv.Value.Length = size;
                m_pChunkedContext->MemoryBuffer << hv;
                advanced += size;
                chunked_data += size;
                len -= size;
            }
            if (chunked_data[0] == '\r' && chunked_data[1] == '\n') {
                advanced += 2;
                chunked_data += 2;
                len -= 2;
                m_pChunkedContext->ParseStatus = psBlock;
            } else {
                SetResponseCode(400);
                return chunked_data;
            }
            if (size == 0 && len == 0)
                m_pChunkedContext->ParseStatus = psComplete;
            su->SetSize(0);
        } while (len > 4); //5 for '0\r\n\r\n'

        unsigned int n;
        char *pos = (char *) m_pChunkedContext->MHeaders->GetBuffer();
        CHeaderValue *pHV = (CHeaderValue *) m_pChunkedContext->GetHeaderValue(len);
        for (n = 0; n < len; ++n, ++pHV) {
            pHV->Header.Start = pos;
            if (pHV->Header.Length) {
                pos += pHV->Header.Length;
                *pos = 0;
                ++pos;
            }
            pHV->Value.Start = pos;
            pos += pHV->Value.Length;
            *pos = 0;
            ++pos;
        }

        return chunked_data;
    }

    const char* CHttpContext::ParseMultipartHeaders(const UHTTP::CHeaderValue *boundary, const char *subdata, unsigned int len) {
        const char *end = strstr(subdata, "\r\n\r\n");
        if (end == nullptr)
            return subdata;
        unsigned int HeaderLen = (unsigned int) (end - subdata) + 4;
#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART
        const char *stop = UHTTP::ParseMultipartHeader(subdata, HeaderLen, boundary->Value.Start, *m_pMultiplaxContext);
        if (m_pMultiplaxContext->ParseStatus != psHeaders) {
            SetResponseCode(400); //bad request
            return subdata;
        } else if (m_RequestContext.ContentLen < HeaderLen) {
            SetResponseCode(400); //bad request
            return subdata;
        }
        if (m_RequestContext.ContentLen != CONTENT_LEN_UNKNOWN) {
            m_RequestContext.ContentLen -= HeaderLen;
        }
        return stop;
#else
        SetResponseCode(501); //Not Implemented
        return subdata;
#endif
    }

    const char* CHttpContext::ParseMultipart(const char *subdata, unsigned int len) {
        unsigned int left;
        bool found = false;
        unsigned int advanced = 0;
        const char *stop = subdata;
        const char *end = nullptr;
        if (subdata == nullptr || len == 0)
            throw CUExCode("No data available for parsing multipart content", MB_BAD_OPERATION);
        if (m_RequestContext.CM == SPA::ServerSide::tagContentMultiplax::cmUnknown || subdata == nullptr || len == 0)
            throw CUExCode("No http multipart found in request", MB_BAD_OPERATION);
        if (m_pMultiplaxContext == nullptr) {
            m_pMultiplaxContext = LockMultiplaxContext();
        }
        const UHTTP::CHeaderValue *boundary = SeekMultipart();
        unsigned int bl = boundary->Value.Length + 4; //-- and CRLF = 4 bytes
        m_pMultiplaxContext->BoundaryLength = bl; //-- 
        SPA::UINT64 cl = m_RequestContext.ContentLen;
        if (cl != CONTENT_LEN_UNKNOWN && (cl + bl) > m_pMultiplaxContext->MHeaders->GetMaxSize())
            m_pMultiplaxContext->MHeaders->ReallocBuffer((unsigned int) cl + bl * 2);
        if (m_pMultiplaxContext->ParseStatus < psHeaders || m_pMultiplaxContext->ParseStatus == psBlock) //4*CRLF = 8
        {
            stop = ParseMultipartHeaders(boundary, subdata, len);
            if (stop == subdata)
                return stop;
            len -= (unsigned int) (stop - subdata);
            subdata = stop;
        } else if (len < (bl * 2 + 2)) {
            return subdata;
        }

        assert(m_pMultiplaxContext->ParseStatus == psHeaders || m_pMultiplaxContext->ParseStatus == psPartialBlock);

        SPA::CScopeUQueue su;
        su->Push("\r\n--", 4);
        su->Push(boundary->Value.Start, boundary->Value.Length);
        unsigned int mlen = su->GetSize();
        CHeaderValue hv;
        memset(&hv, 0, sizeof (hv));
        do {
            const char *dash = (const char *) memchr(subdata, '\r', len);
            while (dash) {
                advanced = (unsigned int) (dash - subdata);
                left = len - advanced;
                if (mlen > left)
                    break;
                found = (::memcmp(dash, su->GetBuffer(), mlen) == 0);
                if (found)
                    break;
                else {
                    dash = (const char *) memchr(dash + 1, '\r', left);
                }
            }
            stop = subdata + advanced;
            hv.Value.Length = advanced;
            left = m_pMultiplaxContext->MHeaders->GetSize();
            m_pMultiplaxContext->MHeaders->Push(subdata, advanced);
            hv.Value.Start = (const char*) m_pMultiplaxContext->MHeaders->GetBuffer(left);
            m_pMultiplaxContext->MemoryBuffer << hv;
            if (cl != CONTENT_LEN_UNKNOWN) {
                len -= advanced;
                m_RequestContext.ContentLen -= advanced;
                cl = m_RequestContext.ContentLen;
            }
            subdata += advanced;
            stop = subdata;
            if (found) {
                subdata += 2; //CRLF
                if (cl != CONTENT_LEN_UNKNOWN) {
                    len -= 2;
                    m_RequestContext.ContentLen -= 2;
                    cl = m_RequestContext.ContentLen;
                }
                m_pMultiplaxContext->ParseStatus = psBlock;
                if ((len == bl + 2) && subdata[bl] == '\r' && subdata[bl + 1] == '\n') {
                    m_pMultiplaxContext->ParseStatus = psComplete;
                    if (cl != CONTENT_LEN_UNKNOWN) {
                        m_RequestContext.ContentLen -= len;
                        if (m_RequestContext.ContentLen != 0)
                            SetResponseCode(400); //bad request
                    }
                    return subdata + len;
                } else
                    stop = ParseMultipartHeaders(boundary, subdata, len);
                if (stop == subdata)
                    return stop;
                len -= (unsigned int) (stop - subdata);
                subdata = stop;
            } else
                m_pMultiplaxContext->ParseStatus = psPartialBlock;
        } while (found);
        return stop;
    }

    bool CHttpContext::DownloadFile(SPA::CUQueue & RecvBuffer) {
        if (m_HttpResponse.Status != hrsDownloadingFile || !m_file.is_open() || m_file.fail())
            return false;
        CHttpDownFileBuffer FileBuffer;
        char *str = (char*) FileBuffer->GetBuffer();
        m_file.read(str, FILE_READ_BUFFER_SIZE);
        size_t read = (size_t) m_file.gcount();
        if (m_HttpResponse.Chunked) {
            char buf[32];
#ifdef WIN32_64
            ::sprintf_s(buf, sizeof (buf), "%X\r\n", (unsigned int) read);
#else
            ::sprintf(buf, "%X\r\n", (unsigned int) read);
#endif
            RecvBuffer.Push(buf);
            if (read) {
                RecvBuffer.Push(FileBuffer->GetBuffer(), (unsigned int) read);
            }
            RecvBuffer.Push("\r\n");
            if (read && read < FILE_READ_BUFFER_SIZE) {
                RecvBuffer.Push("0\r\n\r\n");
            }
        } else {
            if (read) {
                RecvBuffer.Push(FileBuffer->GetBuffer(), (unsigned int) read);
            }
        }
        if (read < FILE_READ_BUFFER_SIZE) {
            m_file.close();
            m_HttpResponse.Status = hrsCompleted;
        }
        return true;
    }

    bool CHttpContext::IsResponseChunked() {
        StringMapA::const_iterator it = m_mapResponse.find(TRANSFER_ENCODING);
        if (it == m_mapResponse.cend())
            return false;
        return iequals(it->second.c_str(), "chunked");
    }

    bool CHttpContext::StartDownloadFile(const char *file, SPA::CUQueue & RecvBuffer) {
        if (m_HttpResponse.Status != hrsInitial)
            return false;
        if (m_file.is_open())
            m_file.close();

        m_file.open(file, std::ios::binary);
        if (!m_file.is_open() || m_file.fail()) {
            SetResponseCode(404);
            PrepareResponse(nullptr, 0, RecvBuffer, hrfpAll);
        } else {
            StringMapA::const_iterator it = m_mapResponse.find(ContentEncoding);
            if (it != m_mapResponse.cend() && iequals(it->second.c_str(), "gzip"))
                m_mapResponse.erase(it); //gzip not supported yet

            m_file.seekg(0, std::ios::end);
            std::streamsize fileSize = m_file.tellg();
            m_file.seekg(0, std::ios::beg);

            CHttpDownFileBuffer FileBuffer;
            char *str = (char*) FileBuffer->GetBuffer();
            m_file.read(str, FILE_READ_BUFFER_SIZE);
            SPA::UINT64 read = m_file.gcount();
            FileBuffer->SetSize((unsigned int) read);
            if (read == fileSize)
                m_file.close();

            bool bChunked = IsResponseChunked();
            if (bChunked && fileSize <= FILE_READ_BUFFER_SIZE) {
                StringMapA::const_iterator it = m_mapResponse.find(TRANSFER_ENCODING);
                m_mapResponse.erase(it);
                bChunked = false;
            }
            m_HttpResponse.Chunked = bChunked;

            if (!bChunked && !IsWebSocket()) {
                char str[32];
#ifdef WIN32_64
                ::sprintf_s(str, sizeof (str), "%I64u", (SPA::UINT64)fileSize);
#else
                ::sprintf(str, "%lu", (SPA::UINT64)fileSize);
#endif
                AddResponseHeader(CONTENT_LENGTH.c_str(), str);
            }
            PrepareResponse(nullptr, 0, RecvBuffer, hrfpHeaderOnly);
            if (bChunked) {
                char str[32];
#ifdef WIN32_64
                ::sprintf_s(str, sizeof (str), "%X\r\n", (unsigned int) read);
#else
                ::sprintf(str, "%X\r\n", (unsigned int) read);
#endif
                RecvBuffer.Push(str);
            }

            RecvBuffer.Push(FileBuffer->GetBuffer(), FileBuffer->GetSize());
            if (bChunked)
                RecvBuffer.Push("\r\n");
            if (m_file.is_open())
                m_HttpResponse.Status = hrsDownloadingFile;
            else
                m_HttpResponse.Status = hrsCompleted;
        }
        return true;
    }

    const char* CHttpContext::GetHost() const {
        const CHeaderValue *p = SeekHeader(Host);
        if (!p)
            return nullptr;
        return p->Value.Start;
    }

    const char* CHttpContext::GetUserAgent() const {
        const CHeaderValue *p = SeekHeader(UserAgent);
        if (!p)
            return nullptr;
        return p->Value.Start;
    }

    const char* CHttpContext::GetReferer() const {
        const CHeaderValue *p = SeekHeader(Referer);
        if (!p)
            return nullptr;
        return p->Value.Start;
    }

    bool CHttpContext::IsCrossDomain() const {
        const CHeaderValue *p = SeekHeader(Referer);
        if (!p)
            return false;
        const char *ref = p->Value.Start;
        p = SeekHeader(Host);
        if (!p)
            return false;
        const char *host = p->Value.Start;
        const char *found = ::strstr(ref, host);
        if (!found)
            return true;
        return false;
    }

    SPA::ServerSide::tagTransport CHttpContext::GetTransport() const {
        return m_transport;
    }

    CWebRequestProcessor * CHttpContext::GetWebRequestProcessor() const {
        return m_pWebRequestProcessor;
    }

    bool CHttpContext::IsBinaryRequest() const {
        if (m_pWebSocketMsg)
            return (m_pWebSocketMsg->ParseStatus >= UHTTP::psComplete);
        UHTTP::tagParseStatus ps = GetPS();
        SPA::ServerSide::tagHttpMethod hm = GetMethod();
        if (hm == SPA::ServerSide::tagHttpMethod::hmPost)
            return (ps >= UHTTP::psBlock);
        return (ps >= UHTTP::psHeaders);
    }

    const char* CHttpContext::ParseHeaders(const char *str, bool peek) {
        const char *ss;
        unsigned int len;
        const char *space = ::strchr(str, ' ');
        if (space == nullptr)
            return str;
        if (!peek)
            ss = ::strstr(str, "\r\n\r\n");
        else
            ss = ::strstr(str, "\r\n");
        if (space >= ss)
            return str;
        m_RequestHeaders.SetSize(0);
        if (!peek)
            len = (unsigned int) (ss - str) + 4;
        else
            len = (unsigned int) (ss - str) + 2;

        if (m_RequestHeaders.GetMaxSize() > 2 * SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE && len < SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE)
            m_RequestHeaders.ReallocBuffer(SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE);

        m_RequestHeaders.Push(str, len);
        m_RequestHeaders.SetNull();
        const char *start = (const char*) m_RequestHeaders.GetBuffer();
        const char *end = ParseHttp(start, len, m_RequestContext);
        if (peek)
            return ss;
        memset(&m_HttpResponse, 0, sizeof (m_HttpResponse));
        m_mapResponse.clear();
        m_ResponseCode = 200;
        if (m_RequestContext.ParseStatus < psHeaders) {
            SetResponseCode(400); //bad request
            return str;
        }

        if (m_RequestContext.Version < 0.9999999999 || m_RequestContext.Version > 1.20000000001) {
            SetResponseCode(505); //HTTP Version Not Supported
            return str;
        }

        SetDefaultResponseHeaders();
        len = (unsigned int) (end - start);
        str += len;
        m_RequestContext.Content = str;
        switch (m_RequestContext.Method) {
            case SPA::ServerSide::tagHttpMethod::hmGet:
                m_RequestContext.ParseStatus = psComplete;
                if (iequals(JS_SP_ADAPTER.c_str(), m_RequestContext.Url.Start)) {
                    m_HttpRequestType = hrtDownloadAdapter;
                } else if (iequals(JS_ULOADER.c_str(), m_RequestContext.Url.Start)) {
                    m_HttpRequestType = hrtDownloadLoader;
                } else if (IsSpRequest()) {
                    m_HttpRequestType = hrtJsRequest;
                    m_transport = SPA::ServerSide::tagTransport::tScript;
                    AddResponseHeader(CONTENT_TYPE.c_str(), "application/x-javascript");
                    delete m_pWebRequestProcessor;
                    m_pWebRequestProcessor = new CJavaScriptRequestProcessor(this);
                }
                break;
            case SPA::ServerSide::tagHttpMethod::hmHead:
            case SPA::ServerSide::tagHttpMethod::hmPut:
            case SPA::ServerSide::tagHttpMethod::hmDelete:
            case SPA::ServerSide::tagHttpMethod::hmTrace:
                m_RequestContext.ParseStatus = psComplete;
                /*
                if (::strlen(str)) //Don't forget HTTP Pipe line 
                                        SetResponseCode(409); //Conflict these methods should not contain any content data
                 */
                break;
            default:
                if (IsSpRequest()) {
                    m_HttpRequestType = hrtJsRequest;
                    m_transport = SPA::ServerSide::tagTransport::tAjax;
                    AddResponseHeader(CONTENT_TYPE.c_str(), "application/json; charset=utf-8");
                    delete m_pWebRequestProcessor;
                    m_pWebRequestProcessor = new CAjaxRequestProcessor(this);
                }
                if (m_RequestContext.ContentLen != CONTENT_LEN_UNKNOWN) {
                    size_t len = ::strlen(str);
                    if (m_RequestContext.ContentLen == len) {
                        m_RequestContext.ParseStatus = psComplete;
                    } else if (m_RequestContext.ContentLen < len) {

                        //Don't forget HTTP Pipe line 
                        //SetResponseCode(409); //Conflict actual length > m_RequestContext.ContentLen

                        m_RequestContext.ParseStatus = psComplete;
                    } else if (len)
                        m_RequestContext.ParseStatus = psPartialContent;
                } else {
                    if (m_RequestContext.Method == SPA::ServerSide::tagHttpMethod::hmPut ||
                            m_RequestContext.Method == SPA::ServerSide::tagHttpMethod::hmOptions ||
                            (m_RequestContext.Method == SPA::ServerSide::tagHttpMethod::hmPost && m_RequestContext.CM == SPA::ServerSide::tagContentMultiplax::cmUnknown && m_RequestContext.TE == SPA::ServerSide::tagTransferEncoding::teUnknown))
                        SetResponseCode(411); // Length Required
                }
                break;
        }

        if (m_RequestContext.Method == SPA::ServerSide::tagHttpMethod::hmGet) {
            do {
                const CHeaderValue *pWebKeyheader = SeekRequestHeader("Sec-WebSocket-Key");
                if (pWebKeyheader == nullptr)
                    break;
                const CHeaderValue *pUpgrade = SeekRequestHeader("Upgrade");
                if (pUpgrade == nullptr)
                    break;
                const CHeaderValue *pProtocol = SeekRequestHeader("Sec-WebSocket-Protocol");
                m_bWebSocket = true;
                SetResponseCode(101);
                m_transport = SPA::ServerSide::tagTransport::tWebSocket;
                m_mapResponse["Upgrade"] = "websocket";
                if (IsKeepAlive())
                    m_mapResponse[Connection] = "Upgrade, keep-alive";
                else
                    m_mapResponse[Connection] = "Upgrade";
                //const CHeaderValue *version = SeekRequestHeader("Sec-WebSocket-Version");

                //http://stackoverflow.com/questions/9444693/ie10-closes-connection-after-handshake
                if (pProtocol)
                    m_mapResponse["Sec-WebSocket-Protocol"] = pProtocol->Value.Start;

                //m_mapResponse["Sec-WebSocket-Version"] = version->Value.Start;
                AddSecWebSocketAcceptHeader(pWebKeyheader->Value.Start);

            } while (false);
        }
        return str;
    }

    tagSpRequest CHttpContext::MapRequest(const char *reqName) {
        if (!reqName)
            return srUnknown;
        if (strcmp(SP_REQUEST_SWITCHTO.c_str(), reqName) == 0)
            return srSwitchTo;
        if (strcmp(SP_REQUEST_DOBATCH.c_str(), reqName) == 0)
            return srDoBatch;
        if (strcmp(SP_REQUEST_ENTER.c_str(), reqName) == 0)
            return srEnter;
        if (strcmp(SP_REQUEST_EXIT.c_str(), reqName) == 0)
            return srExit;
        if (strcmp(SP_REQUEST_SPEAK.c_str(), reqName) == 0)
            return srSpeak;
        if (strcmp(SP_REQUEST_SENDUSERMESSAGE.c_str(), reqName) == 0)
            return srSendUserMessage;
        if (strcmp(SP_REQUEST_PING.c_str(), reqName) == 0)
            return srPing;
        if (strcmp(SP_REQUEST_CLOSE.c_str(), reqName) == 0)
            return srClose;
        return srRequest;
    }

    unsigned int CHttpContext::GetPingTime() const {
        return m_pt;
    }

    void CHttpContext::SetPingTime(unsigned int pt) {
        if (pt < 5000)
            pt = 5000;
        m_pt = pt;
    }

    std::string CHttpContext::GenerateId(const UHttpRequest & UReq) {
        assert(UReq.SpRequest == srSwitchTo);
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

    void CHttpContext::ProcessRequestBatch(const UHttpRequest &UReq, SPA::CUQueue & Response) {
        SPA::UJsonValue &args = *((SPA::UJsonValue *)UReq.Args);
#ifndef NDEBUG
        bool b = args.as_array()[(unsigned int) 0].as_bool();
#endif
        SPA::UJsonValue &objs = args.as_array()[1];
        unsigned int count = (unsigned int) objs.as_array().size();
        for (unsigned int n = 0; n < count; ++n) {
            const SPA::UJsonValue &req = objs.as_array()[n];
            UHttpRequest ur = ParseSPRequest(req);
            ProcessSpRequest(ur, Response);
        }
    }

    void CHttpContext::ProcessSpRequest(const UHttpRequest &UReq, SPA::UJsonValue & res) {
        SPA::UJsonObject obj;
        obj[CHttpContext::SP_REQUEST_CI] = UReq.CallIndex;
        obj[CHttpContext::SP_RESPONSE_CODE] = UReq.ErrCode;
        if (UReq.ErrCode == seOk) {
            switch (UReq.SpRequest) {
                case srSwitchTo:
                {
                    obj[CHttpContext::SP_REQUEST_ID] = GenerateId(UReq);

                    SetPt();
                    obj[HTTP_RESPONSE_PT] = m_pt;
                }
                    break;
                case srPing:
                    if (!IsWebSocket()) {
                        //when a client pings, we turn off socket connection
                        m_mapResponse[Connection] = SP_CONNECTION_CLOSE.c_str();
                    }
                    break;
                    break;
                default:
                    obj[CHttpContext::SP_RESPONSE_RESULT] = "SomeData";
                    break;
            }
        } else {
            switch (UReq.SpRequest) {
                case srSwitchTo:
                    obj[CHttpContext::SP_REQUEST_ID] = "";
                    break;
                case srPing:
                    if (!IsWebSocket()) {
                        //when a client pings, we turn off socket connection
                        m_mapResponse[Connection] = SP_CONNECTION_CLOSE.c_str();
                    }
                    break;
                default:
                    obj[CHttpContext::SP_RESPONSE_RESULT] = "";
                    break;
            }
        }
        res = std::move(obj);
    }

    void CHttpContext::SetPt() {
        m_pt = 15 * 1000;
        const CHeaderValue *p = SeekHeader(UserAgent);
        if (p && p->Value.Start && ::strstr(p->Value.Start, " Chrome/"))
            m_pt = 60 * 1000;
        else if (p && p->Value.Start && ::strstr(p->Value.Start, " Firefox/"))
            m_pt = 50 * 1000;
    }

    void CHttpContext::ProcessSpRequest(const UHttpRequest &UReq, SPA::CUQueue & Response) {
        tagSpError rc = UReq.ErrCode;
        Connection::CConnectionContext::SharedPtr pCC = Connection::CConnectionContext::SeekConnectionContext(UReq.Id);
        SPA::UJsonObject objRes;
        objRes[CHttpContext::SP_REQUEST_CI] = UReq.CallIndex;
        if (UReq.SpRequest != srSwitchTo && pCC == nullptr) {
            rc = seAuthenticationFailed;
        }
        objRes[CHttpContext::SP_RESPONSE_CODE] = rc;
        if (rc == seOk) {
            switch (UReq.SpRequest) {
                case srSwitchTo:
                {
                    if (pCC)
                        Connection::CConnectionContext::RemoveConnectionContext(UReq.Id);
                    std::string id = GenerateId(UReq);
                    objRes[CHttpContext::SP_REQUEST_ID] = id;
                    SetPt();
                    objRes[HTTP_RESPONSE_PT] = m_pt;

                    if (!IsWebSocket()) {
                        Connection::CConnectionContext::SharedPtr cc(new Connection::CConnectionContext);
                        cc->UserId = UReq.GetUserIdW();
                        Connection::CConnectionContext::AddConnectionContext(id, cc);
                    }
                }
                    break;
                case srPing:
                    if (!IsWebSocket()) {
                        //when a client pings, we turn off socket connection
                        m_mapResponse[Connection] = SP_CONNECTION_CLOSE.c_str();
                    }
                    break;
                case srDoBatch:
                    if (!IsWebSocket() && m_RequestContext.Method == SPA::ServerSide::tagHttpMethod::hmGet) {
                        ProcessRequestBatch(UReq, Response);
                    }
                    break;
                case srClose:
                    Connection::CConnectionContext::RemoveConnectionContext(UReq.Id);
                    m_mapResponse[Connection] = SP_CONNECTION_CLOSE.c_str();
                    break;
                default:
                    objRes[CHttpContext::SP_RESPONSE_RESULT] = "SomeData";
                    break;
            }
        } else {
            if (!IsWebSocket()) {
                //when a client pings, we turn off socket connection
                m_mapResponse[Connection] = SP_CONNECTION_CLOSE.c_str();
            }
            switch (UReq.SpRequest) {
                case srSwitchTo:
                    objRes[CHttpContext::SP_REQUEST_ID] = "";
                    break;
                case srPing:
                    break;
                case srClose:
                    Connection::CConnectionContext::RemoveConnectionContext(UReq.Id);
                    m_mapResponse[Connection] = SP_CONNECTION_CLOSE.c_str();
                    break;
                default:
                    objRes[CHttpContext::SP_RESPONSE_RESULT] = "";
                    break;
            }
        }
        if (!IsWebSocket() && m_RequestContext.Method == SPA::ServerSide::tagHttpMethod::hmGet)
            Response.Push(HTTP_JS_CALLBACK_HEAD.c_str(), (unsigned int) HTTP_JS_CALLBACK_HEAD.size());
        SPA::UJsonValue jv(std::move(objRes));
        Response << jv;
        if (!IsWebSocket() && m_RequestContext.Method == SPA::ServerSide::tagHttpMethod::hmGet)
            Response.Push(HTTP_JS_CALLBACK_END.c_str(), (unsigned int) HTTP_JS_CALLBACK_END.size());
    }

    UHttpRequest CHttpContext::ParseSPRequest(const SPA::UJsonValue & req) {
        UHttpRequest UReq;
        UReq.SpRequest = srUnknown;
        if (!req.is_object()) {
            UReq.ErrCode = seUnexpectedRequest;
            return UReq;
        }
        const SPA::UJsonObject& obj = req.as_object();

        size_t size = obj.size();
        if (size != 5) {
            UReq.ErrCode = seBadNumberOfArgs;
            return UReq;
        }

        if (!obj.contains(SP_REQUEST_NAME) ||
                !obj.contains(SP_REQUEST_CI) ||
                !obj.contains(SP_REQUEST_VERSION) ||
                !obj.contains(SP_REQUEST_ARGS) ||
                !obj.contains(SP_REQUEST_ID)
                ) {
            UReq.ErrCode = seBadArgs;
            return UReq;
        }

        const SPA::UJsonValue &reqName = obj.at(SP_REQUEST_NAME);
        if (!reqName.is_string()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }

        UReq.ReqName = reqName.as_string().c_str();
        UReq.SpRequest = MapRequest(UReq.ReqName);

        const SPA::UJsonValue &id = obj.at(SP_REQUEST_ID);
        if (!id.is_string()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }
        UReq.Id = id.as_string().c_str();

        const SPA::UJsonValue &ci = obj.at(SP_REQUEST_CI);
        if (ci.is_int64()) {
            UReq.CallIndex = ci.as_int64();
        } else if (ci.is_uint64()) {
            UReq.CallIndex = (SPA::INT64)ci.as_uint64();
        } else {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }

        const SPA::UJsonValue &v = obj.at(SP_REQUEST_VERSION);
        if (!v.is_double()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }
        UReq.Version = v.as_double();

        const SPA::UJsonValue &args = obj.at(SP_REQUEST_ARGS);
        if (!args.is_array()) {
            UReq.ErrCode = seWrongArgType;
            return UReq;
        }

        if (UReq.IsBatching()) {
            UReq.ErrCode = seBadArgs;
            return UReq;
        }
        UReq.Args = &args;
        UReq.ErrCode = seOk;
        return UReq;
    }

    bool CHttpContext::AddResponseHeader(const char *str, const char *value) {
        if (str == nullptr)
            return false;
        if (!value)
            value = "";
        while (*str && ::isspace(*str)) {
            ++str;
        }
        size_t len = ::strlen(str);
        if (len == 0)
            return false;
        m_mapResponse[str] = value;
        return true;
    }

    bool CHttpContext::AddSecWebSocketAcceptHeader(const char *key) {
        if (key == nullptr)
            return false;
        SPA::CScopeUQueue su;
        su->Push(key);
        if (su->GetSize() == 0)
            return false;
        //http://tools.ietf.org/html/rfc6455
        //http://en.wikipedia.org/wiki/WebSocket
        su->Push("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        su->SetNull();

        unsigned char bytes[32] = {0};
        char str[48] = {0};
        SPA::CSHA1 sha1;
        sha1.Update(su->GetBuffer(), su->GetSize());
        sha1.Final();
        bool ok = sha1.GetHash(bytes);
        unsigned int res = SPA::CBase64::encode(bytes, 20, str);
        m_mapResponse["Sec-WebSocket-Accept"] = str;
        return true;
    }

    const CHeaderValue * CHttpContext::SeekHeader(const std::string & header) const {
        unsigned int n;
        unsigned int size;
        const CHeaderValue *p = m_RequestContext.GetHeaderValue(size);
        for (n = 0; n < size; ++n, ++p) {
            if (iequals(header.c_str(), p->Header.Start))
                return p;
        }
        return nullptr;
    }

    CMultiplaxContext * CHttpContext::GetMultiplaxContext() const {
        return m_pMultiplaxContext;
    }

    CChunkedContext * CHttpContext::GetChunkedContext() const {
        return m_pChunkedContext;
    }

    CWebSocketMsg * CHttpContext::GetWebSocketMsg() const {
        return m_pWebSocketMsg;
    }

    bool CHttpContext::IsGZipAccepted() const {
        const CHeaderValue *p = SeekHeader(AcceptEncoding);
        if (!p)
            return false;
        return boost::ifind_first(p->Value.Start, "gzip");
    }

    bool CHttpContext::AskForClose() const {
        const CHeaderValue *p = SeekHeader(Connection);
        if (!p)
            return false;
        return boost::ifind_first(p->Value.Start, SP_CONNECTION_CLOSE.c_str());
    }

    bool CHttpContext::IsKeepAlive() const {
        const CHeaderValue *p = SeekHeader(Connection);
        if (!p)
            return false;
        return boost::ifind_first(p->Value.Start, SP_CONNECTION_KEEP_ALIVE);
    }

    const char* CHttpContext::GetUJSData() const {
        if (m_RequestContext.Method == SPA::ServerSide::tagHttpMethod::hmGet && m_RequestContext.Params.Start) {
            const char *s = ::strstr(m_RequestContext.Params.Start, UJS_DATA.c_str());
            if (s != nullptr) {
                return (s + UJS_DATA.size());
            }
        }
        return nullptr;
    }

    bool CHttpContext::IsSpRequest() const {
        switch (m_RequestContext.Method) {
            case SPA::ServerSide::tagHttpMethod::hmGet:
                if (m_RequestContext.Params.Start)
                    return (::strstr(m_RequestContext.Params.Start, UJS_DATA.c_str()) != nullptr);
                break;
            case SPA::ServerSide::tagHttpMethod::hmPost:
                return (m_RequestContext.SeekHeaderValue(UHTTP_REQUEST.c_str()) != nullptr);
                break;
            default:
                break;
        }
        return false;
    }

    bool CHttpContext::IsResponseKeepAlive() const {
        StringMapA::const_iterator it = m_mapResponse.find(Connection);
        if (it == m_mapResponse.cend())
            return false;
        return boost::ifind_first(it->second, SP_CONNECTION_KEEP_ALIVE);
    }

    const CHeaderValue * CHttpContext::GetRequestHeaders(unsigned int &count) const {
        return m_RequestContext.GetHeaderValue(count);
    }

    const CHeaderValue * CHttpContext::SeekMultipart() const {
        return m_RequestContext.SeekMultipart();
    }

    const CHeaderValue * CHttpContext::SeekRequestHeader(const char *header) const {
        return m_RequestContext.SeekHeaderValue(header);
    }

    SPA::ServerSide::tagHttpMethod CHttpContext::GetMethod() const {
        return m_RequestContext.Method;
    }

    const CHttpUnit & CHttpContext::GetUrl() const {
        return m_RequestContext.Url;
    }

    bool CHttpContext::IsWebSocket() const {
        return m_bWebSocket;
    }

    const CHttpUnit & CHttpContext::GetParams() const {
        return m_RequestContext.Params;
    }

    CWebResponseProcessor * CHttpContext::GetWebResponseProcessor() const {
        return m_pWebResponseProcessor;
    }

    double CHttpContext::GetVersion() const {
        return m_RequestContext.Version;
    }

    CWebSocketMsg * CHttpContext::LockWebSocketMsg() {
        CWebSocketMsg *p;
        m_cs.lock();
        if (m_vWSM.size()) {
            p = m_vWSM.back();
            m_vWSM.pop_back();
        } else
            p = nullptr;
        m_cs.unlock();
        if (p == nullptr)
            p = new CWebSocketMsg;
        else
            p->Initialize();
        return p;
    }

    void CHttpContext::UnlockWebSocketMsg(CWebSocketMsg * p) {
        if (!p)
            return;
        m_cs.lock();
        if (p->Content.GetMaxSize() > 20 * 1024)
            p->Content.ReallocBuffer(20 * 1024);
        m_vWSM.push_back(p);
        m_cs.unlock();
    }

    CChunkedContext * CHttpContext::LockChunkedContext() {
        CChunkedContext *p;
        m_cs.lock();
        if (m_vCC.size()) {
            p = m_vCC.back();
            m_vCC.pop_back();
        } else
            p = nullptr;
        m_cs.unlock();
        if (p == nullptr)
            p = new CChunkedContext;
        else
            p->Initialize();
        return p;
    }

    void CHttpContext::UnlockChunkedContext(CChunkedContext * p) {
        if (!p)
            return;
        m_cs.lock();
        m_vCC.push_back(p);
        m_cs.unlock();
    }

    CMultiplaxContext * CHttpContext::LockMultiplaxContext() {
        CMultiplaxContext *p;
        m_cs.lock();
        if (m_vMC.size()) {
            p = m_vMC.back();
            m_vMC.pop_back();
        } else
            p = nullptr;
        m_cs.unlock();
        if (p == nullptr)
            p = new CMultiplaxContext;
        else
            p->Initialize();
        return p;
    }

    void CHttpContext::UnlockMultiplaxContext(CMultiplaxContext * p) {
        if (!p)
            return;
        m_cs.lock();
        m_vMC.push_back(p);
        m_cs.unlock();
    }

    CHttpContext * CHttpContext::Lock() {
        CHttpContext *p;
        m_cs.lock();
        if (m_vHC.size()) {
            p = m_vHC.back();
            m_vHC.pop_back();
        } else
            p = nullptr;
        m_cs.unlock();
        if (p) {
            p->m_RequestHeaders.SetSize(0);
            p->m_RequestContext.Initialize();
            ::memset(&p->m_HttpResponse, 0, sizeof (HttpResponse));
            p->m_bWebSocket = false;
            p->m_mapResponse.clear();
            p->m_ResponseCode = 200;
            p->m_HttpRequestType = hrtCustomer;
            p->m_pt = 60000;
            p->m_transport = SPA::ServerSide::tagTransport::tUnknown;
            return p;
        }
        p = new CHttpContext;
        return p;
    }

    void CHttpContext::Unlock(CHttpContext * p) {
        if (!p)
            return;
        if (p->m_file.is_open())
            p->m_file.close();
        delete p->m_pWebRequestProcessor;
        p->m_pWebRequestProcessor = nullptr;
        delete p->m_pWebResponseProcessor;
        p->m_pWebResponseProcessor = nullptr;
        UnlockMultiplaxContext(p->m_pMultiplaxContext);
        p->m_pMultiplaxContext = nullptr;
        UnlockChunkedContext(p->m_pChunkedContext);
        p->m_pChunkedContext = nullptr;
        UnlockWebSocketMsg(p->m_pWebSocketMsg);
        p->m_pWebSocketMsg = nullptr;
        m_cs.lock();
        m_vHC.push_back(p);
        m_cs.unlock();
    }
}